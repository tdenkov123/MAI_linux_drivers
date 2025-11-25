#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>

static int __init init(void);
static void __exit cleanup(void);

static int device_open(struct inode *inode, struct file *file) {
  pr_info("OPENING\n");
  return 0;
}

static int device_release(struct inode *inode, struct file *file) {
  pr_info("RELEASING\n");
  return 0;
}

const struct file_operations fops = {
    .open = device_open,
    .release = device_release,
};

unsigned int Major;

static int __init init(void) {
  pr_info("Initializing\n");

  Major = register_chrdev(0, "my_driver", &fops);
  if (Major < 0) {
    pr_alert("Registering char device failed with %d\n", Major);
    return Major;
  }

  pr_info("Succesful driver init\n");

  return 0;
}

static void __exit cleanup(void) {
  pr_info("CLEANUP\n");
  unregister_chrdev(Major, "my_driver");
}


module_init(init);
module_exit(cleanup);

MODULE_DESCRIPTION("MAI drivers training");
MODULE_LICENSE("GPL");
