#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "ioctl_macro.h"


static void read_from_device(const int fd, const size_t len) {
    char buf[100];
    const ssize_t read_len = read(fd, buf, len);
    if (read_len < 0) {
        printf("read_from_device: Failed: %ld\n", read_len);
        return;
    }
    buf[read_len] = 0;
    printf("read_from_device: \"%s\"\n", buf);
}

static void write_to_device(const int fd, const char str[], const size_t len) {
    const ssize_t wrote_len = write(fd, str, len);
    if (wrote_len < 0) {
        printf("write_to_device: Failed: %ld\n", wrote_len);
        return;
    }
    printf("Written \"%s\" to dev\n", str);
}

int main() {
    const char driver_path[] = "/dev/pz4";

    const int reader_fd = open(driver_path, O_RDONLY);
    if (reader_fd < 0) {
        printf("Failed to open %s: %d\n", driver_path, reader_fd);
        return reader_fd;
    }

    const int writer_fd = open(driver_path, O_WRONLY);
    if (writer_fd < 0) {
        printf("Failed to open %s: %d\n", driver_path, writer_fd);
        return writer_fd;
    }

    write_to_device(writer_fd, "PING PONG", 9);
    read_from_device(reader_fd, 9);
    bool buf_is_empty = false;
    if (ioctl(reader_fd, DRIVER_BUF_IS_EMPTY, &buf_is_empty) != 0) {
        printf("DRIVER_BUF_IS_EMPTY\n");
        return -1;
    } else {
        printf("Buffer isn't empty: %s\n", buf_is_empty ? "true" : "false");
    }

    if (ioctl(reader_fd, DRIVER_CLEAR_BUF) != 0) {
        printf("Failed to ioctl DRIVER_CLEAR_BUF\n");
        return -1;
    }

    if (ioctl(reader_fd, DRIVER_BUF_IS_EMPTY, &buf_is_empty) != 0) {
        printf("Failed to ioctl DRIVER_BUF_IS_EMPTY\n");
        return -1;
    } else {
        printf("Buffer is empty after DRIVER_CLEAR_BUF: %s\n", buf_is_empty ? "true" : "false");
    }
}
