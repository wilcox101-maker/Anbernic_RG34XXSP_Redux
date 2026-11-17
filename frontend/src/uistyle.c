#define _POSIX_C_SOURCE 200809L
#include "uistyle.h"
#include "grid.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef REDUX_PLATFORM_STOCK
#define UISTYLE_PATH "/mnt/data/redux/uistyle"
#else
#define UISTYLE_PATH "/tmp/redux.uistyle"
#endif

/* The 16 standard VGA text-mode colours, in their usual order. */
const uis_color_t uis_colors[UIS_COLORS] = {
    { "Black",           0,   0,   0 }, { "Blue",            0,   0, 170 },
    { "Green",           0, 170,   0 }, { "Cyan",            0, 170, 170 },
    { "Red",           170,   0,   0 }, { "Magenta",       170,   0, 170 },
    { "Brown",         170,  85,   0 }, { "Light Gray",    170, 170, 170 },
    { "Dark Gray",      85,  85,  85 }, { "Light Blue",     85,  85, 255 },
    { "Light Green",    85, 255,  85 }, { "Light Cyan",     85, 255, 255 },
    { "Light Red",     255,  85,  85 }, { "Light Magenta", 255,  85, 255 },
    { "Yellow",        255, 255,  85 }, { "White",         255, 255, 255 },
};

static const char *const keys[UIS_COUNT] = { "header", "footer", "window", "text", "hftext", "wincolor" };
static const int defaults[UIS_COUNT] = { 0, 0, 0, UIS_WHITE, UIS_WHITE, UIS_BLACK };
static const int maxes[UIS_COUNT] = { 1, 1, 2, UIS_COLORS - 1, UIS_COLORS - 1, UIS_COLORS - 1 };

static int val[UIS_COUNT] = { 0, 0, 0, UIS_WHITE, UIS_WHITE, UIS_BLACK };
static const char *g_path;

void uistyle_set_path(const char *p) { g_path = p; }
static const char *path(void) { return g_path ? g_path : UISTYLE_PATH; }

int uistyle_get(int item) { return item >= 0 && item < UIS_COUNT ? val[item] : 0; }

static int hidden(int item, int v) {
    if (item == UIS_TEXT)     return v == val[UIS_WINCOLOR];
    if (item == UIS_HFTEXT)   return v == UIS_BLACK;          /* header and footer sit on black */
    if (item == UIS_WINCOLOR) return v == val[UIS_TEXT];
    return 0;
}

static void save(void) {
#ifdef REDUX_PLATFORM_STOCK
    if (mkdir("/mnt/data/redux", 0755) != 0 && errno != EEXIST) return;
#endif
    char tmp[512];
    snprintf(tmp, sizeof tmp, "%s.new", path());
    FILE *f = fopen(tmp, "w");
    if (!f) return;
    for (int i = 0; i < UIS_COUNT; i++) fprintf(f, "%s %d\n", keys[i], val[i]);
    if (fclose(f) != 0) { remove(tmp); return; }
    rename(tmp, path());
}

int uistyle_step(int item, int dir) {
    if (item < 0 || item >= UIS_COUNT || dir == 0) return uistyle_get(item);
    dir = dir > 0 ? 1 : -1;
    int v = val[item];
    if (item <= UIS_WINDOW) {
        v += dir;
        if (v < 0) v = 0;
        if (v > maxes[item]) v = maxes[item];
    } else {
        for (int n = 0; n < UIS_COLORS; n++) {
            v = (v + dir + UIS_COLORS) % UIS_COLORS;
            if (!hidden(item, v)) break;
        }
    }
    val[item] = v;
    save();
    uistyle_apply();
    return v;
}

const char *uistyle_value_name(int item) {
    static const char *const sizes[3] = { "Default", "Large", "Largest" };
    if (item < 0 || item >= UIS_COUNT) return "";
    if (item <= UIS_WINDOW) return sizes[val[item]];
    return uis_colors[val[item]].name;
}

void uistyle_load(void) {
    memcpy(val, defaults, sizeof val);
    FILE *f = fopen(path(), "r");
    if (f) {
        char k[16]; int v;
        while (fscanf(f, "%15s %d", k, &v) == 2)
            for (int i = 0; i < UIS_COUNT; i++)
                if (strcmp(k, keys[i]) == 0 && v >= 0 && v <= maxes[i]) val[i] = v;
        fclose(f);
    }
    /* A hand-edited file must not hide text either. */
    if (val[UIS_HFTEXT] == UIS_BLACK) val[UIS_HFTEXT] = defaults[UIS_HFTEXT];
    if (val[UIS_TEXT] == val[UIS_WINCOLOR]) { val[UIS_TEXT] = UIS_WHITE; val[UIS_WINCOLOR] = UIS_BLACK; }
    uistyle_apply();
}

void uistyle_reset_fonts(void) {
    val[UIS_HEADER] = val[UIS_FOOTER] = val[UIS_WINDOW] = 0;
    save();
    uistyle_apply();
}

void uistyle_reset(void) {
    remove(path());
    memcpy(val, defaults, sizeof val);
    uistyle_apply();
}

void uistyle_apply(void) {
    const uis_color_t *t = &uis_colors[val[UIS_TEXT]], *h = &uis_colors[val[UIS_HFTEXT]],
                      *w = &uis_colors[val[UIS_WINCOLOR]];
    grid_set_sizes(val[UIS_HEADER], val[UIS_FOOTER], val[UIS_WINDOW]);
    grid_set_colors(t->r, t->g, t->b, h->r, h->g, h->b, w->r, w->g, w->b);
}
