#pragma once

#include <linux/ioctl.h>

#define MAGIC_NUM 0x11

#define DRIVER_CLEAR_BUF _IO(MAGIC_NUM, 0)
#define DRIVER_BUF_IS_EMPTY _IOR(MAGIC_NUM, 1, int)
#define MYDRIVER_IOC_MAXNR 1
