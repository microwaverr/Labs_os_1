#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <string.h>

#define BUF_SIZE 4096

static void die(const char *msg) {
    write(2, msg, strlen(msg));
    write(2, "\n", 1);
    _exit(1);
}

int main(void) {
    int pipe1[2];
    int pipe_mid[2];
    int pipe2[2];

    if (pipe(pipe1) < 0) die("pipe1");
    if (pipe(pipe_mid) < 0) die("pipe_mid");
    if (pipe(pipe2) < 0) die("pipe2");


    const pid_t pid1 = fork();
    if (pid1 < 0) die("fork1");
    if (pid1 == 0) {
        if (dup2(pipe1[0], 0) < 0) die("dup2 c1 stdin");
        if (dup2(pipe_mid[1], 1) < 0) die("dup2 c1 stdout");

        close(pipe1[0]);    close(pipe1[1]);
        close(pipe_mid[0]); close(pipe_mid[1]);
        close(pipe2[0]);    close(pipe2[1]);

        char *args[] = {"./child1", NULL};
        execve(args[0], args, NULL);
        die("exec child1");
    }

    const pid_t pid2 = fork();
    if (pid2 < 0) die("fork2");

    if (pid2 == 0) {
        if (dup2(pipe_mid[0], 0) < 0) die("dup2 c2 stdin");
        if (dup2(pipe2[1], 1) < 0) die("dup2 c2 stdout");

        close(pipe1[0]);    close(pipe1[1]);
        close(pipe_mid[0]); close(pipe_mid[1]);
        close(pipe2[0]);    close(pipe2[1]);

        char *args[] = {"./child2", NULL};
        execve(args[0], args, NULL);
        die("exec child2");
    }

    close(pipe1[0]);
    close(pipe_mid[0]);
    close(pipe_mid[1]);
    close(pipe2[1]);

    int stdin_open = 1;
    int pipe2_open = 1;
    char buf[BUF_SIZE];

    while (stdin_open || pipe2_open) {
        fd_set readfds;
        FD_ZERO(&readfds);

        int maxfd = -1;

        if (stdin_open) {
            FD_SET(0, &readfds);
            if (maxfd < 0) maxfd = 0;
        }

        if (pipe2_open) {
            FD_SET(pipe2[0], &readfds);
            if (maxfd < pipe2[0]) maxfd = pipe2[0];
        }

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (stdin_open && FD_ISSET(0, &readfds)) {
            ssize_t n = read(0, buf, BUF_SIZE);
            if (n < 0) die("read stdin");
            if (n == 0) {
                stdin_open = 0;
                close(pipe1[1]);
            } else {
                char *p = buf;
                ssize_t left = n;
                while (left > 0) {
                    ssize_t w = write(pipe1[1], p, left);
                    if (w < 0) die("write pipe1");
                    left -= w;
                    p += w;
                }
            }
        }

        if (pipe2_open && FD_ISSET(pipe2[0], &readfds)) {
            ssize_t n = read(pipe2[0], buf, BUF_SIZE);
            if (n < 0) die("read pipe2");
            if (n == 0) {
                pipe2_open = 0;
                close(pipe2[0]);
            } else {
                char *p = buf;
                ssize_t left = n;
                while (left > 0) {
                    ssize_t w = write(1, p, left);
                    if (w < 0) die("write stdout");
                    left -= w;
                    p += w;
                }
            }
        }
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}