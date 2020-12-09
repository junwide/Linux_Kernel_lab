#include <linux/module.h>
#include <linux/init.h>

//模块参数
static char *name = "yuwei xiao";
static int age = 22;
//S_IRUGO是参数权限，也可以用数字
module_param(age,int,S_IRUGO);
module_param(name,charp,S_IRUGO);

//使用外部文件函数
extern int add(int a,int b);
//声明 外部内核符号 函数
extern int add_sum(int a,int b);

static int __init cite_init(void)
{
     //多文件编译

    printk(KERN_EMERG"Test hi");
    int vle=add(3,3);
    printk(KERN_EMERG"add value:%d\n",vle);
    //模块参数

     printk(KERN_EMERG" name : %s\n",name);
     printk(KERN_EMERG" age : %d\n",age);

    //使用其他模块的函数(内核符号导出)
    int adds=add_sum(4,4);
    printk(KERN_EMERG" add_sum : %d\n",adds);
    return 0;
}

static void __exit cite_exit(void)
{
    printk(KERN_EMERG"param exit!");
}

module_init(cite_init);
module_exit(cite_exit);

//模块可选信息
MODULE_LICENSE("GPL");//许可证声明
MODULE_AUTHOR("junwide");//作者声明
MODULE_DESCRIPTION("This module is a param example.");//模块描述
MODULE_VERSION("V1.0");//模块别名
MODULE_ALIAS("a simple module");//模块别名
