#include <unistd.h>

int main(void) {
    char buf[4096];
    ssize_t n;

    while ((n = read(0, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] >= 'a' && buf[i] <= 'z') {
                buf[i] = (char)(buf[i] - 'a' + 'A');
            }
        }

        ssize_t written = 0;
        while (written < n) {
            const ssize_t w = write(1, buf + written, n - written);
            if (w < 0) return 1;
            written += w;
        }
    }
    return 0;
}