#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>  //用户访问的接口
#include <linux/kfifo.h>

#define DEMO_NAME "kfifodriver"
static struct device * mydemo_device;
DEFINE_KFIFO(demo_fifo, char, 64);

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
//loff_t 表示当前读写位置的offset  
static ssize_t demo_read(struct file * file, char __user *buf, size_t buf_count,\
                            loff_t *ppos)
{    
    int ret;
    int readed_;
    if(kfifo_is_empty(&demo_fifo))
    {
        if(file->f_flags & O_NONBLOCK)
            return -EAGAIN;
    }
    ret = kfifo_to_user(&demo_fifo,buf, buf_count,&readed_); 
    if(ret)
       return -EIO;
    printk(KERN_EMERG"%s readed = %d, pos = %lld \n",__func__,readed_,*ppos);
    return readed_;
}
static ssize_t demo_write(struct file * file, const char __user *buf, size_t buf_count,\
                            loff_t *ppos)
{
    int ret;
    int writed_ ;
    if(kfifo_is_full(&demo_fifo))
    {
        if(file->f_flags & O_NONBLOCK)
            return -EAGAIN;
    }
    ret = kfifo_from_user(&demo_fifo,buf, buf_count,&writed_); //返回没有拷贝成功的字节数
    if(ret)
        return -EIO;
    printk(KERN_EMERG"%s writed = %d, pos = %lld \n",__func__,writed_,*ppos);
    return writed_;
}
static const struct file_operations demo_fops =
{
    .owner = THIS_MODULE,
    .open  = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write
};

static struct miscdevice mydemo_device_misc_d = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEMO_NAME,
    .fops = &demo_fops
};
static int __init demo_init(void)
{
    int ret;
    ret = misc_register(&mydemo_device_misc_d);
    if(ret) 
    {
        printk(KERN_EMERG"Faild in register misc device");
        return ret;
    }
    mydemo_device = mydemo_device_misc_d.this_device;
    printk(KERN_EMERG"success register misc device: %s\n",DEMO_NAME);
    return 0;
}

static void  __exit demo_exit(void)
{
    printk("remove Device\n");
    misc_deregister(&mydemo_device_misc_d);
}


module_init(demo_init);
module_exit(demo_exit);

//模块可选信息
MODULE_LICENSE("GPL");//许可证声明
MODULE_AUTHOR("junwide");//作者声明
MODULE_DESCRIPTION("This module is a char device");//模块描述
MODULE_VERSION("V1.0");//模块别名
MODULE_ALIAS("Char Modules");//模块别名