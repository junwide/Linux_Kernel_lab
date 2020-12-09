#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>  //用户访问的接口
#include <linux/slab.h>

#define DEMO_NAME "demodriver"
#define MAX_BUFFER_SIZE 64
static struct device * mydemo_device;
static char * _buffer; //虚拟FIFO缓冲区

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
    int max_free = MAX_BUFFER_SIZE - *ppos; //剩余空间计算
    int remain_ = max_free > buf_count ? buf_count: max_free ;   // 剩下未读到的
    int readed_ = 0;
    if(remain_ == 0)
        dev_warn(mydemo_device, "No space for read");
    ret = copy_to_user(buf,_buffer + *ppos,remain_); //返回没有拷贝成功的字节数
    if(ret == remain_)
       return -EFAULT;
    readed_ = remain_  -  ret;
    *ppos  += readed_;
    printk(KERN_EMERG"%s readed = %d, pos = %lld \n",__func__,readed_,*ppos);
    return readed_;
}
static ssize_t demo_write(struct file * file, const char __user *buf, size_t buf_count,\
                            loff_t *ppos)
{
    int ret;
    int max_free = MAX_BUFFER_SIZE - *ppos; //剩余空间计算
    int remain_ = max_free > buf_count ? buf_count: max_free ;   // 
    int writed_ = 0;
    if(remain_ == 0)
        dev_warn(mydemo_device, "No space for write");
    ret = copy_from_user(_buffer+*ppos, buf, remain_); //返回没有拷贝成功的字节数
    if(ret == remain_)
       return -EFAULT;
    writed_ = remain_  -  ret;
    *ppos  += writed_;
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
    _buffer = kmalloc(MAX_BUFFER_SIZE,GFP_KERNEL);
    if(!_buffer)
        return -ENOMEM;
    ret = misc_register(&mydemo_device_misc_d);
    if(ret) 
    {
        printk(KERN_EMERG"Faild in register misc device");
        kfree(_buffer);
        return ret;
    }
    mydemo_device = mydemo_device_misc_d.this_device;
    printk(KERN_EMERG"success register misc device: %s\n",DEMO_NAME);
    return 0;
}

static void  __exit demo_exit(void)
{
    printk("remove Device\n");
    kfree(_buffer);
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