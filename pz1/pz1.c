#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  char buf[100];
  int fd = open("/dev/zero", O_RDONLY);
  read(fd, buf, 100);
  close(fd);

  return 0;
}
