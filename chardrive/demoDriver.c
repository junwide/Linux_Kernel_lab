#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEMO_NAME "demodriver"
static dev_t dev ;    //字符设备的设备号
static unsigned int count =1 ; //在一个主设备号下，次设备数量统计
static struct cdev *demo_cdev;

static int demo_open(struct inode * inode,struct file * file)
{
    //我们将驱动做成一个设备节点，所以可以用inode结构来访问，驱动信息
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);
    printk(KERN_EMERG"%s: Major Num: %d , Minor Num: %d",__func__,major,minor);
    return 0;
}
static int demo_release(struct inode * inode,struct file * file)
{
    return 0;
}

static ssize_t demo_read(struct file * file, char __user *buf, size_t lbuf,\
                            loff_t *ppos)
{
    printk(KERN_EMERG"%s enter",__func__);
    return 0;
}
static ssize_t demo_write(struct file * file, const char __user *buf, size_t count,\
                            loff_t *f_pos)
{
    printk(KERN_EMERG"%s enter",__func__);
    return 0;
}
static const struct file_operations demo_fops =
{
    .owner = THIS_MODULE,
    .open  = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write
};

static int __init demo_init(void)
{
    int ret;
    //系统自动分配一个主设备号，使用这个接口避免冲突，这里需要4个参数cdev，baseminor ，count ，name
    ret = alloc_chrdev_region(&dev, 0 ,count,DEMO_NAME);
    if(ret) 
    {
        printk(KERN_EMERG"Faild in alloc char device region");
        return ret;
    }
    demo_cdev = cdev_alloc(); //申请设备数据结构
    if (!demo_cdev)
    {
        printk(KERN_EMERG"cdev_alloc faild");
        goto unregister_chrdev;
    }
    cdev_init(demo_cdev,&demo_fops); //结构体和操作函数的指针建立联系
    ret =  cdev_add(demo_cdev,dev,count); //将设备添加到系统中，传入主设备号和次设备数目
    if(ret)
    {
        printk(KERN_EMERG"cdev_add faild");
        goto cdev_fail;
    }
    printk(KERN_EMERG"success register char device: %s\n",DEMO_NAME);
    printk(KERN_EMERG"Major Num: %d , Minor Num: %d",MAJOR(dev),MINOR(dev));

    return 0;

    cdev_fail:
        cdev_del(demo_cdev);
    unregister_chrdev:
        unregister_chrdev_region(dev,count); //卸载掉，释放回系统

    return ret;
}

static void  __exit demo_exit(void)
{
    printk("remove Device\n");
    if(demo_cdev)
        cdev_del(demo_cdev);
    unregister_chrdev_region(dev,count);
}


module_init(demo_init);
module_exit(demo_exit);

//模块可选信息
MODULE_LICENSE("GPL");//许可证声明
MODULE_AUTHOR("junwide");//作者声明
MODULE_DESCRIPTION("This module is a char device");//模块描述
MODULE_VERSION("V1.0");//模块别名
MODULE_ALIAS("Char Modules");//模块别名