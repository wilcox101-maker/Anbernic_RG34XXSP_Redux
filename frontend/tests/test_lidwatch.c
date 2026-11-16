/* redux-lidwatch (F4 step 4) against a file of input events and a local UDP socket. No device.
   Exit 0 = pass. */
#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <linux/input.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include "lidwatch.h"

static int fails;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }

static void ev(FILE *f, int type, int code, int value) {
    struct input_event e;
    memset(&e, 0, sizeof e);
    e.type = (unsigned short)type; e.code = (unsigned short)code; e.value = value;
    fwrite(&e, sizeof e, 1, f);
}

int main(void) {
    /* Install: the embedded program lands at the path, executable, byte for byte. */
    lidwatch_set_path("test_lidwatch.bin");
    expect(lidwatch_install() == 0, "install writes the program");
    struct stat st;
    expect(stat("test_lidwatch.bin", &st) == 0 && (st.st_mode & 0111) && (size_t)st.st_size == redux_lidwatch_bin_len,
           "installed file is executable and complete");

    /* Events: power press, lid repeat, lid close, lid open, open tap. Only the close counts. */
    FILE *f = fopen("test_lidwatch.events", "wb");
    ev(f, EV_KEY, 116, 1); ev(f, EV_KEY, 116, 0);
    ev(f, EV_KEY, 110, 2);
    ev(f, EV_KEY, 110, 1);
    ev(f, EV_SYN, 0, 0);
    ev(f, EV_KEY, 110, 0); ev(f, EV_KEY, 111, 1); ev(f, EV_KEY, 111, 0);
    fclose(f);

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in a;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK); a.sin_port = 0;
    bind(s, (struct sockaddr *)&a, sizeof a);
    socklen_t al = sizeof a;
    getsockname(s, (struct sockaddr *)&a, &al);
    char port[16];
    snprintf(port, sizeof port, "%d", ntohs(a.sin_port));
    struct timeval tv = { 2, 0 };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);

    pid_t pid = fork();
    if (pid == 0) {
        execl("./test_lidwatch.bin", "redux-lidwatch", "test_lidwatch.events", port, (char *)NULL);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    expect(WIFEXITED(status) && WEXITSTATUS(status) == 0, "watcher exits 0 at end of input");

    char buf[64];
    ssize_t n = recv(s, buf, sizeof buf - 1, 0);
    if (n > 0) buf[n] = 0;
    expect(n == 10 && strcmp(buf, "SAVE_FILES") == 0, "lid close sends SAVE_FILES");
    tv.tv_sec = 0; tv.tv_usec = 200000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    expect(recv(s, buf, sizeof buf, 0) < 0, "power, repeat, open and the open tap send nothing");
    close(s);

    remove("test_lidwatch.events");
    remove("test_lidwatch.bin");
    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
