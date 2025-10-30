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

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file> <host> <port>\n", argv[0]);
        exit(1);
    }

    const char *filepath = argv[1];
    const char *host = argv[2];
    int port = atoi(argv[3]);

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

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        close(fd);
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // ✅ CORRECT CHECK: must return 1
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid host IP: %s\n", host);
        close(fd);
        close(sock);
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(fd);
        close(sock);
        exit(1);
    }

    printf("📤 Sending %ld bytes from '%s' to %s:%d\n", (long)sb.st_size, filepath, host, port);

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

    // Signal EOF to receiver
    shutdown(sock, SHUT_WR);
    usleep(100000); // 100ms grace

    printf("✅ Sent %ld bytes\n", (long)offset);

    close(sock);
    close(fd);
    return 0;
}