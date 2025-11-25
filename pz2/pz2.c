#include <linux/module.h>
#include <linux/kernel.h>           
#include <linux/init.h>


static int __init init(void);
static void __exit cleanup(void);

static int __init init(void){
   pr_info("INIT\n");
   return 0;
}
 
static void __exit cleanup(void){
   pr_info("CLEANUP\n");
}
 
module_init(init);
module_exit(cleanup);

MODULE_DESCRIPTION("MAI drivers training");
MODULE_LICENSE("GPL");
