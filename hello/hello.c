#include <linux/module.h>
#include <linux/init.h>

static int __init my_test_init(void)
{
	printk(KERN_EMERG "hello world\n");
	return 0;
}

static void __exit my_test_exit(void)
{
	printk("nice try\n");
}

int add_sum(int a,int b)
{
    return a+b;
}
EXPORT_SYMBOL(add_sum);

module_init(my_test_init);
module_exit(my_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("junwide");
MODULE_DESCRIPTION("my test kernel module");
MODULE_ALIAS("mytest");

