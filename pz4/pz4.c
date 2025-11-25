#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/wait.h>
#include "ioctl_macro.h"

#define DRIVER_NAME "pz4_device"
#define DEVICE_NAME "pz4_dev"
#define CLASS_NAME  "pz4_class"
#define BUF_LEN     20

DECLARE_WAIT_QUEUE_HEAD(wq);

static char buf[BUF_LEN];
static size_t buf_len = 0;
static dev_t dev_no;
static struct cdev dev;
static struct class* dev_class;
static struct device* device;

static int device_open(struct inode* inode, struct file* file);
static int device_release(struct inode* inode, struct file* file);
static ssize_t device_read(struct file* flip, char __user* user_buf, size_t count, loff_t* offset);
static ssize_t device_write(struct file* flip, const char __user* user_buf, size_t count, loff_t* offset);
static long device_ioctl(struct file* flip, unsigned int cmd, unsigned long arg);

const struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
    .owner = THIS_MODULE,
};

static int device_open(struct inode* inode, struct file* file) {
    printk(KERN_INFO DRIVER_NAME ": device_open: trying to open the device\n");
    return 0;
}

static int device_release(struct inode* inode, struct file* file) {
    printk(KERN_INFO DRIVER_NAME ": device_release: trying to release the device\n");
    return 0;
}

static ssize_t device_read(struct file* flip, char __user* user_buf, size_t count, loff_t* offset) {
    printk(KERN_INFO DRIVER_NAME ": device_read: count=%zu, offset=%lld, buffer_len=%zu\n", count, *offset, buf_len);

    if (wait_event_interruptible(wq, buf_len != 0)) {
        printk(KERN_INFO DRIVER_NAME ": device_read: interrupted");
        return -ERESTARTSYS;
    }

    if (*offset >= buf_len) {
        printk(KERN_INFO DRIVER_NAME ": device_read: End Of File");
        return 0;
    }

    const size_t readable_bytes_len = min(count, buf_len - *offset);

    if (copy_to_user(user_buf, buf + *offset, readable_bytes_len)) {
        printk(KERN_ERR DRIVER_NAME ": device_read: copy_to_user failed");
        return -EFAULT;
    }

    *offset += readable_bytes_len;

    printk(KERN_INFO DRIVER_NAME ": device_read: successfully read %lu bytes", readable_bytes_len);

    return readable_bytes_len;
}

static ssize_t device_write(struct file* flip, const char __user* user_buf, size_t count, loff_t* offset) {
    printk(KERN_INFO DRIVER_NAME ": device_write: count=%zu, offset=%lld, buffer_len=%zu\n", count, *offset, buf_len);

    if (buf_len + count > BUF_LEN) {
        printk(KERN_ERR DRIVER_NAME ": device_write: buffer is full");
        return -ENOSPC;
    }

    if (copy_from_user(buf + *offset, user_buf, count)) {
        printk(KERN_ERR DRIVER_NAME ": device_write: copy_from_user failed");
        return -EFAULT;
    }

    buf_len += count;
    wake_up_interruptible(&wq);
    *offset = buf_len;

    printk(KERN_INFO DRIVER_NAME ": device_write: successfully wrote %lu bytes", count);

    return count;
}

static long device_ioctl(struct file* flip, unsigned int cmd, unsigned long arg) {
    printk(KERN_INFO DRIVER_NAME ": device_ioctl: cmd=0x%x\n", cmd);

    switch (cmd) {
        case DRIVER_CLEAR_BUF:
            printk(KERN_INFO DRIVER_NAME ": device_ioctl: DRIVER_CLEAR_BUF");

            buf_len = 0;
            break;
        case DRIVER_BUF_IS_EMPTY:
            printk(KERN_INFO DRIVER_NAME ": device_ioctl: DRIVER_BUF_IS_EMPTY");

            const bool buf_is_empty = buf_len == 0 ? true : false;
            if (copy_to_user((bool*)arg, &buf_is_empty, sizeof(bool))) {
                printk(KERN_ERR DRIVER_NAME ": device_ioctl: copy_to_user failed");
                return -EFAULT;
            }

            break;
        default: printk(KERN_ERR DRIVER_NAME ": device_ioctl: Unknown command: %d", cmd); break;
    }

    return 0;
}

static int __init init(void) {
    printk(KERN_INFO DRIVER_NAME ": trying to initialize the driver\n");

    int res = 0;

    res = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
    if (res < 0) {
        printk(KERN_ERR DRIVER_NAME ": failed to allocate device numbers with error %d\n", res);
        return res;
    }

    cdev_init(&dev, &fops);
    dev.owner = THIS_MODULE;

    res = cdev_add(&dev, dev_no, 1);
    if (res < 0) {
        printk(KERN_ERR DRIVER_NAME ": failed to add character device with error %d\n", res);
        unregister_chrdev_region(dev_no, 1);
        return res;
    }

    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        printk(KERN_ERR DRIVER_NAME ": failed to create device class\n");
        res = PTR_ERR(dev_class);
        cdev_del(&dev);
        unregister_chrdev_region(dev_no, 1);
        return res;
    }

    device = device_create(dev_class, NULL, dev_no, NULL, DEVICE_NAME);
    if (IS_ERR(device)) {
        printk(KERN_ERR DRIVER_NAME ": Failed to create device node\n");
        res = PTR_ERR(device);
        class_destroy(dev_class);
        cdev_del(&dev);
        unregister_chrdev_region(dev_no, 1);
        return res;
    }

    printk(KERN_INFO DRIVER_NAME ": successfully initialized the driver\n");
    return 0;
}

static void __exit cleanup(void) {
    printk(KERN_INFO DRIVER_NAME ": cleaning up driver\n");

    device_destroy(dev_class, dev_no);
    class_destroy(dev_class);
    cdev_del(&dev);
    unregister_chrdev_region(dev_no, 1);

    printk(KERN_INFO DRIVER_NAME ": driver unloaded successfully\n");
}

module_init(init);
module_exit(cleanup);

MODULE_DESCRIPTION("MAI drivers training");
MODULE_LICENSE("GPL");
