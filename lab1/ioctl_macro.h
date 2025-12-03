#pragma once

#include <linux/ioctl.h>

#define MAGIC_NUM 0x11
#define DRIVER_GET_HISTOGRAM_LEN _IOR(MAGIC_NUM, 0, size_t)
#define DRIVER_GET_HISTOGRAM_BUF _IOR(MAGIC_NUM, 1, size_t *)
