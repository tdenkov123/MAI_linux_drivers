#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include "ioctl_macro.h"

static void read_from_dev(const int fd) {
	int buf;
	const ssize_t read_len = read(fd, &buf, sizeof(buf));
	if (read_len < 0) {
		printf("read_from_dev: Failed: %ld\n", read_len);
		return;
	}
	printf("read_from_dev: %d\n", buf);
}

static void write_to_dev(const int fd, const int buf) {
	const ssize_t wrote_len = write(fd, &buf, sizeof(buf));
	if (wrote_len < 0) {
		printf("write_to_dev: Failed: %ld\n", wrote_len);
		return;
	}
	printf("write_to_dev: %d\n", buf);
}

int main() {
	const char driver_path[] = "/dev/lab1";

	const int reader_fd = open(driver_path, O_RDONLY);
	if (reader_fd < 0) {
		printf("Failed to open %s: %d\n", driver_path, reader_fd);
		return reader_fd;
	}

	const int writer_fd = open(driver_path, O_WRONLY);
	if (reader_fd < 0) {
		printf("Failed to open %s: %d\n", driver_path, reader_fd);
		return reader_fd;
	}

	for (size_t i = 0; i < 1000; i++) {
		write_to_dev(writer_fd, i);
		read_from_dev(reader_fd);
	}

	size_t histogram_len;
	int ioctl_err;
	if (ioctl_err = ioctl(reader_fd, DRIVER_GET_HISTOGRAM_LEN,
			      &histogram_len)) {
		printf("Failed to ioctl DRIVER_GET_HISTOGRAM_LEN: %d\n",
		       ioctl_err);
		return -1;
	}
	printf("Histogram buffer length is %ld\n", histogram_len);

	size_t *histogram_buf = (size_t*) malloc(histogram_len * sizeof(size_t));
	if (ioctl_err =
		    ioctl(reader_fd, DRIVER_GET_HISTOGRAM_BUF, histogram_buf)) {
		printf("Failed to ioctl DRIVER_GET_HISTOGRAM_BUF: %d\n",
		       ioctl_err);
		return -1;
	}

	for (size_t bin_idx = 0; bin_idx < histogram_len; bin_idx++)
		printf("%ld:\t%ld\n", bin_idx, histogram_buf[bin_idx]);
}
