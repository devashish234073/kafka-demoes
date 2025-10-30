#define _GNU_SOURCE
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/* Fetch monotonic time from receiver's HTTP endpoint. Returns seconds as double.
 * If the HTTP call or parsing fails, returns a negative value.
 */
double get_remote_time(const char *url) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "curl -s --max-time 5 '%s'", url);
    FILE *f = popen(cmd, "r");
    if (!f) return -1.0;
    char buf[256];
    size_t n = fread(buf, 1, sizeof(buf)-1, f);
    pclose(f);
    if (n == 0) return -1.0;
    buf[n] = '\0';
    double t = -1.0;
    // Expecting JSON like: {"time":0.123456789}
    if (sscanf(buf, "{\"time\":%lf", &t) == 1) return t;
    // fallback: try to find any number in the response
    char *p = buf;
    while (*p && !( (*p>='0'&&*p<='9') || *p=='.' )) p++;
    if (!*p) return -1.0;
    if (sscanf(p, "%lf", &t) == 1) return t;
    return -1.0;
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <file> <host> <port> [count]\n", argv[0]);
        exit(1);
    }

    const char *filepath = argv[1];
    const char *host = argv[2];
    int port = atoi(argv[3]);
    int count = 20; // default
    if (argc == 5) count = atoi(argv[4]);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open file");
        exit(1);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        close(fd);
        exit(1);
    }

    if (sb.st_size == 0) {
        fprintf(stderr, "File is empty\n");
        close(fd);
        exit(1);
    }

    /*
     * We'll keep the file descriptor open and call sendfile() `count` times.
     * To avoid wall-clock jumps we measure using CLOCK_MONOTONIC and only
     * measure the actual sendfile loop (i.e. after connect, before shutdown).
     */

    /* Set up address structure once before the loop */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid host IP: %s\n", host);
        close(fd);
        exit(1);
    }

    /* We'll request a monotonic timestamp from receiver before and after the loop */
    const char *time_url = "http://127.0.0.1:8001/time";
    double start_t = get_remote_time(time_url);

    int successful = 0;
    for (int run = 0; run < count; ++run) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            perror("socket");
            continue;
        }

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("connect");
            close(sock);
            continue;
        }

        off_t offset = 0;
        ssize_t sent;
        while (offset < sb.st_size) {
            sent = sendfile(sock, fd, &offset, sb.st_size - offset);
            if (sent == -1) {
                if (errno == EINTR || errno == EAGAIN) continue;
                perror("sendfile");
                break;
            }
        }

        /* Signal EOF to receiver and close socket */
        shutdown(sock, SHUT_WR);
        usleep(1000); // 100ms grace
        close(sock);

        /* rewind file offset for next run */
        if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
            perror("lseek");
        }

        successful++;
    }

    double end_t = get_remote_time(time_url);
    double elapsed = -1.0;
    if (start_t >= 0 && end_t >= 0) {
        elapsed = end_t - start_t;
    }
    if (successful > 0) {
        printf("Total elapsed: %.9f seconds for %d runs\n", elapsed, successful);
        printf("Average time: %.9f seconds\n", elapsed / successful);
    } else {
        fprintf(stderr, "No successful runs\n");
    }

    close(fd);
    return 0;
}