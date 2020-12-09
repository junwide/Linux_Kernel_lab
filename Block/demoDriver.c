#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>  //用户访问的接口
#include <linux/kfifo.h>
#include <linux/slab.h>

#define DEMO_NAME "kfifo_bl_driver"
DEFINE_KFIFO(demo_fifo, char, 64);

struct mydemo_device{
    const char * name;
    struct device* dev;
    struct miscdevice *miscdev;
    wait_queue_head_t read_queue;
    wait_queue_head_t write_queue;
};

struct _private_data{
    struct mydemo_device *device;
};

static struct mydemo_device *my_device_p; //全局的静态维护整个结构的指针

static int demo_open(struct inode * inode,struct file * file)
{
    struct _private_data *data;
    struct mydemo_device *device_p = my_device_p; //具体分析
    //我们将驱动做成一个设备节点，所以可以用inode结构来访问，驱动信息
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);
    printk(KERN_EMERG"%s: Major Num: %d , Minor Num: %d",__func__,major,minor);
    data = kmalloc(sizeof(struct _private_data),GFP_KERNEL);
    if(!data)
        return -ENOMEM;
    data->device = device_p;
    file->private_data = data;

    return 0;
}

static int demo_release(struct inode * inode,struct file * file)
{
    struct _private_data *data = file->private_data;
    kfree(data);

    return 0;
}

//loff_t 表示当前读写位置的offset  
static ssize_t demo_read(struct file * file, char __user *buf, size_t buf_count,\
                            loff_t *ppos)
{   
    int ret;
    int readed_;
    struct _private_data *data = file->private_data;
    struct mydemo_device *device = data->device;
    if(kfifo_is_empty(&demo_fifo))
    {
        if(file->f_flags & O_NONBLOCK)
            return -EAGAIN;
        printk(KERN_EMERG"%s: pid=%d, going to sleep\n",__func__,current->pid);
        ret = wait_event_interruptible(device->read_queue,!kfifo_is_empty(&demo_fifo));
        if(ret)
            return ret;
    }
    ret = kfifo_to_user(&demo_fifo,buf, buf_count,&readed_); 
    if(ret)
       return -EIO;
    if(!kfifo_is_full(&demo_fifo))
        wake_up_interruptible(&device->write_queue);
    printk(KERN_EMERG"%s,pid = %d, readed = %d, pos = %lld \n",__func__,
                       current->pid, readed_,*ppos);
    return readed_;
}
static ssize_t demo_write(struct file * file, const char __user *buf, size_t buf_count,\
                            loff_t *ppos)
{
    int ret;
    int writed_ ;
    struct _private_data *data = file->private_data;
    struct mydemo_device *device = data->device;
    if(kfifo_is_full(&demo_fifo))
    {
        if(file->f_flags & O_NONBLOCK)
            return -EAGAIN;
        printk(KERN_EMERG"%s: pid=%d, going to sleep\n",__func__,current->pid);
        ret = wait_event_interruptible(device->write_queue,!kfifo_is_full(&demo_fifo));
        if(ret)
            return ret;
    }
    ret = kfifo_from_user(&demo_fifo,buf, buf_count,&writed_); //返回没有拷贝成功的字节数
    if(ret)
        return -EIO;
    if(!kfifo_is_empty(&demo_fifo))
        wake_up_interruptible(&device->read_queue);
    printk(KERN_EMERG"%s,pid = %d, writed = %d, pos = %lld \n",__func__,
                       current->pid, writed_,*ppos);
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
    struct mydemo_device * device = kmalloc(sizeof(struct mydemo_device),GFP_KERNEL);
    if(!device)
        return -ENOMEM;
    ret = misc_register(&mydemo_device_misc_d);
    if(ret) 
    {
        printk(KERN_EMERG"Faild in register misc device");
        goto free_device;
    }
    device->dev = mydemo_device_misc_d.this_device;
    device->miscdev = &mydemo_device_misc_d;

    init_waitqueue_head(&device->read_queue);
    init_waitqueue_head(&device->write_queue);

    my_device_p = device;  //device 这个名的消失，但是内存还在
    printk(KERN_EMERG"success register misc device: %s\n",DEMO_NAME);
    return 0;

free_device:
	kfree(device);
	return ret;
}

static void  __exit demo_exit(void)
{
    struct mydemo_device *p = my_device_p;
    printk("remove Device\n");
    misc_deregister(p->miscdev);
    kfree(p);
}


module_init(demo_init);
module_exit(demo_exit);

//模块可选信息
MODULE_LICENSE("GPL");//许可证声明
MODULE_AUTHOR("junwide");//作者声明
MODULE_DESCRIPTION("This module is a char device");//模块描述
MODULE_VERSION("V1.0");//模块别名
MODULE_ALIAS("Char Modules");//模块别名