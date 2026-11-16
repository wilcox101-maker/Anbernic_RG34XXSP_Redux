/* redux-lidwatch: runs beside a RetroArch game (F4 step 4, notes/2026-11-16-claude-decision-f4-save-flush.md, A).
   Stock already suspends on lid close in a game (Dale, 2026-11-16 13:53); this only makes sure the
   game's in-game saves reach the card first. On lid close (event0 KEY_INSERT down) it sends
   RetroArch "SAVE_FILES" over its network command port, then sync(). It never suspends, never
   writes a file itself and never grabs the device (stock keeps every event).
   Usage: redux-lidwatch [input device] [udp port]. Exits at end of input (tests) or when killed.
   Built for the device, embedded in dmenu.bin and written to /tmp before a RetroArch launch. */
#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/input.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LID_CODE 110   /* KEY_INSERT on axp2202-pek (#185 row 28) */

static void save_files(int port) {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) { perror("[lidwatch] socket"); return; }
    struct sockaddr_in a;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)port);
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    static const char cmd[] = "SAVE_FILES";
    if (sendto(s, cmd, sizeof cmd - 1, 0, (struct sockaddr *)&a, sizeof a) < 0)
        perror("[lidwatch] sendto");
    close(s);
}

int main(int argc, char **argv) {
    const char *dev = argc > 1 ? argv[1] : "/dev/input/event0";
    int port = argc > 2 ? atoi(argv[2]) : 55355;
    if (port <= 0 || port > 65535) port = 55355;
    int fd = open(dev, O_RDONLY);
    if (fd < 0) { perror("[lidwatch] open"); return 1; }
    struct input_event ev;
    while (read(fd, &ev, sizeof ev) == (ssize_t)sizeof ev) {
        if (ev.type != EV_KEY || ev.code != LID_CODE || ev.value != 1) continue;   /* close only */
        save_files(port);
        /* RetroArch handles the command on its next frame; give it that frame, then flush. */
        usleep(200 * 1000);
        sync();
        fprintf(stderr, "[lidwatch] lid closed: SAVE_FILES sent, sync done\n");
    }
    close(fd);
    return 0;
}
