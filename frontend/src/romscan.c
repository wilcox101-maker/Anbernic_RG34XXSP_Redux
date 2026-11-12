#include "romscan.h"
#include "systems.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define SYS_MAX 128
#define KEY_MAX 32
#define TITLE_MAX 64
#define ROM_NAME_MAX 96
#define PATH_MAX_R 512

typedef struct {
    char name[ROM_NAME_MAX];
    char path[PATH_MAX_R];
} rom_t;

typedef struct {
    char key[KEY_MAX];
    char title[TITLE_MAX];
    int begin;
    int count;
} sys_t;

static sys_t g_sys[SYS_MAX];
static int g_nsys;
/* Grows as needed: Dale's card has well over 10,000 ROMs across 50+ folders, so a fixed
   cap silently dropped whole systems (scan order decided which). */
static rom_t *g_rom;
static int g_nrom;
static int g_cap;

/* Room for one more ROM at index i. Returns 0 on success, -1 if out of memory. */
static int rom_reserve(int i) {
    if (i < g_cap) return 0;
    int cap = g_cap ? g_cap * 2 : 1024;
    rom_t *grown = realloc(g_rom, (size_t)cap * sizeof(rom_t));
    if (!grown) return -1;
    g_rom = grown;
    g_cap = cap;
    return 0;
}

static const struct { const char *key; const char *title; } titles[] = {
    { "GBA", "Game Boy Advance" }, { "GB", "Game Boy" }, { "GBC", "Game Boy Color" },
    { "NES", "NES" }, { "SNES", "Super Nintendo" }, { "MD", "Mega Drive" },
    { "PS1", "PlayStation" }, { "PCSX", "PlayStation" }, { "PSP", "PSP" },
    { "NDS", "Nintendo DS" }, { "FC", "Famicom" }, { "SFC", "Super Famicom" },
    { "SMS", "Master System" }, { "PCE", "PC Engine" }, { "WS", "WonderSwan" },
    { "NGP", "Neo Geo Pocket" }, { "GBA_DGE", "Game Boy Advance" },
    { "GBC_DGE", "Game Boy Color" }, { "MD_DGE", "Mega Drive" },
    { "GG", "Game Gear" }, { "PCECD", "PC Engine CD" }, { "NEOGEO", "Neo Geo" },
    { "FBNEO", "FinalBurn Neo" }, { "CPS1", "CPS-1" }, { "CPS2", "CPS-2" }, { "CPS3", "CPS-3" },
    { "MAME", "MAME (Arcade)" },
    { "A2600", "Atari 2600" }, { "A5200", "Atari 5200" }, { "A7800", "Atari 7800" },
    { "A800", "Atari 800" }, { "LYNX", "Atari Lynx" }, { "ATARIST", "Atari ST" },
    { "AMIGA", "Amiga" }, { "C64", "Commodore 64" }, { "VIC20", "Commodore VIC-20" },
    { "MSX", "MSX" }, { "DOS", "DOS" }, { "SCUMMVM", "ScummVM" }, { "EASYRPG", "EasyRPG" },
    { "ONS", "ONScripter" }, { "FDS", "Famicom Disk System" }, { "GW", "Game & Watch" },
    { "POKE", "Pokemon Mini" }, { "VB", "Virtual Boy" }, { "PICO", "PICO-8" },
    { "SEGA32X", "Sega 32X" }, { "MDCD", "Mega CD" }, { "SATURN", "Saturn" },
    { "N64", "Nintendo 64" }, { "DREAMCAST", "Dreamcast" }, { "NAOMI", "NAOMI" },
    { "ATOMISWAVE", "Atomiswave" }, { "NEOCD", "Neo Geo CD" },
};

/* Folder name on the card -> systems-table key, used only when the folder
   name itself is not already a key. Not verified against Dale's card listing. */
static const struct { const char *folder; const char *key; } aliases[] = {
    /* PS goes to RetroArch pcsx_rearmed: the standalone PCSX launch dropped
       straight back to the menu on the device (2026-11-11). NEOGEO is the
       arcade system with its own table entry, not Neo Geo Pocket. */
    { "PS", "PS1" }, { "PSX", "PS1" }, { "GENESIS", "MD" }, { "MEGADRIVE", "MD" },
    { "NGPC", "NGP" }, { "WSC", "WS" },
};

static void copy_trunc(char *dst, size_t n, const char *src) {
    size_t i = 0;
    if (n == 0) return;
    while (i + 1 < n && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

/* Returns 0 if a + "/" + b fits. No snprintf, so -Wformat-truncation stays quiet. */
static int join2(char *dst, size_t n, const char *a, const char *b) {
    size_t la = strlen(a), lb = strlen(b);
    if (la + 1 + lb + 1 > n) return -1;
    memcpy(dst, a, la);
    dst[la] = '/';
    memcpy(dst + la + 1, b, lb + 1);
    return 0;
}

static int icmp(const char *a, const char *b) {
    while (*a && *b) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d) return d;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static const char *title_for(const char *key) {
    for (size_t i = 0; i < sizeof titles / sizeof titles[0]; i++)
        if (strcmp(titles[i].key, key) == 0) return titles[i].title;
    return key;
}

static const char *key_for_folder(const char *folder) {
    if (systems_lookup(folder)) return folder;
    for (size_t i = 0; i < sizeof aliases / sizeof aliases[0]; i++)
        if (icmp(aliases[i].folder, folder) == 0 && systems_lookup(aliases[i].key))
            return aliases[i].key;
    return NULL;
}

static int skipped_ext(const char *name) {
    const char *dot = strrchr(name, '.');
    if (!dot || dot == name) return 0;
    static const char *skip[] = {
        "png", "jpg", "jpeg", "gif", "bmp", "txt", "nfo", "xml", "sav", "srm",
        "state", "rtc", "cfg", "ini", "dat", "cht", "ips", "bps", "db", "svg",
        "log", "sh", "json", "md", "sbi", "m3u8"
    };
    for (size_t i = 0; i < sizeof skip / sizeof skip[0]; i++)
        if (icmp(dot + 1, skip[i]) == 0) return 1;
    return 0;
}

static void display_name(char *dst, size_t n, const char *file) {
    copy_trunc(dst, n, file);
    char *dot = strrchr(dst, '.');
    if (dot && dot != dst) *dot = '\0';
}

static int is_dir(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int cmp_rom(const void *a, const void *b) {
    return icmp(((const rom_t *)a)->name, ((const rom_t *)b)->name);
}

static int cmp_sys(const void *a, const void *b) {
    return icmp(((const sys_t *)a)->title, ((const sys_t *)b)->title);
}

/* Read one system directory. Returns how many ROMs were appended. */
static int scan_dir(const char *dirpath, int begin) {
    DIR *d = opendir(dirpath);
    if (!d) return 0;
    int n = 0;
    struct dirent *de;
    while ((de = readdir(d)) != NULL) {
        if (de->d_name[0] == '.') continue;
        if (skipped_ext(de->d_name)) continue;
        if (rom_reserve(begin + n) != 0) break;
        char path[PATH_MAX_R];
        if (join2(path, sizeof path, dirpath, de->d_name) != 0) continue;
        struct stat st;
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) continue;
        rom_t *r = &g_rom[begin + n];
        display_name(r->name, sizeof r->name, de->d_name);
        copy_trunc(r->path, sizeof r->path, path);
        n++;
    }
    closedir(d);
    if (n > 1) qsort(g_rom + begin, (size_t)n, sizeof(rom_t), cmp_rom);
    return n;
}

void romscan_load(const char *root) {
    g_nsys = 0;
    g_nrom = 0;
    if (!root || !root[0]) return;
    DIR *d = opendir(root);          /* read-only; never created here */
    if (!d) return;
    struct dirent *de;
    while ((de = readdir(d)) != NULL && g_nsys < SYS_MAX) {
        if (de->d_name[0] == '.') continue;
        const char *key = key_for_folder(de->d_name);
        if (!key) continue;
        char path[PATH_MAX_R];
        if (join2(path, sizeof path, root, de->d_name) != 0) continue;
        if (!is_dir(path)) continue;
        int n = scan_dir(path, g_nrom);
        if (n <= 0) continue;
        sys_t *s = &g_sys[g_nsys++];
        copy_trunc(s->key, sizeof s->key, key);
        copy_trunc(s->title, sizeof s->title, title_for(key));
        s->begin = g_nrom;
        s->count = n;
        g_nrom += n;
    }
    closedir(d);
    if (g_nsys > 1) qsort(g_sys, (size_t)g_nsys, sizeof(sys_t), cmp_sys);
}

#ifndef REDUX_PLATFORM_STOCK
/* Zero-byte names, not games. Lives in /tmp so nothing ROM-like is committed.
   The five names are the F0 ROM mock-up, in alphabetical order. */
static void ensure_fixture(char *out, size_t n) {
    const char *root = "/tmp/redux-rom-fixture";
    copy_trunc(out, n, root);
    mkdir(root, 0755);
    char dir[PATH_MAX_R];
    if (join2(dir, sizeof dir, root, "GBA") != 0) return;
    mkdir(dir, 0755);
    static const char *names[] = {
        "Advance Wars.gba", "Fire Emblem.gba", "Golden Sun.gba",
        "Metroid Fusion.gba", "Pokemon Emerald.gba"
    };
    for (size_t i = 0; i < sizeof names / sizeof names[0]; i++) {
        char path[PATH_MAX_R];
        if (join2(path, sizeof path, dir, names[i]) != 0) continue;
        FILE *f = fopen(path, "a");   /* create if missing; do not truncate a real file */
        if (f) fclose(f);
    }
}
#endif

void romscan_init(void) {
    const char *env = getenv("REDUX_ROMS");
    if (env && env[0]) { romscan_load(env); return; }
#ifdef REDUX_PLATFORM_STOCK
    romscan_load("/mnt/mmc/Roms");
#else
    char root[PATH_MAX_R];
    ensure_fixture(root, sizeof root);
    romscan_load(root);
#endif
}

int romscan_systems(void) { return g_nsys; }
const char *romscan_key(int sys) {
    return (sys >= 0 && sys < g_nsys) ? g_sys[sys].key : "";
}
const char *romscan_title(int sys) {
    return (sys >= 0 && sys < g_nsys) ? g_sys[sys].title : "";
}
int romscan_index_of(const char *key) {
    for (int i = 0; i < g_nsys; i++)
        if (strcmp(g_sys[i].key, key) == 0) return i;
    return 0;
}
int romscan_roms(int sys) {
    return (sys >= 0 && sys < g_nsys) ? g_sys[sys].count : 0;
}
const char *romscan_rom_name(int sys, int rom) {
    if (sys < 0 || sys >= g_nsys || rom < 0 || rom >= g_sys[sys].count) return "";
    return g_rom[g_sys[sys].begin + rom].name;
}
const char *romscan_rom_path(int sys, int rom) {
    if (sys < 0 || sys >= g_nsys || rom < 0 || rom >= g_sys[sys].count) return "";
    return g_rom[g_sys[sys].begin + rom].path;
}
