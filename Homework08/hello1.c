/* Example Minimal Character Device Driver */
#include <linux/module.h>
static int debug_enable = 0;
static int debug_enable1 = 0;
/* Added driver parameter */
module_param(debug_enable, int, 0); /* and these 2 lines */
MODULE_PARM_DESC(debug_enable, "Enable module debug mode.");

module_param(debug_enable1, int, 0); /* and these 2 lines */
MODULE_PARM_DESC(debug_enable1, "Enable module debug1 mode.");
static int __init hello_init(void)
{
/* Now print value of new module parameter */
printk("Hello Example Init - debug mode is %s  debug1 mode is %s\n",
debug_enable ? "enabled" : "disabled", debug_enable1 ? "enabled" : "disabled" );
return 0;
}
static void __exit hello_exit(void)
{
printk("Hello Example Exit\n");
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_AUTHOR("Chris Hallinan");
MODULE_DESCRIPTION("Hello World Example");
MODULE_LICENSE("GPL");

