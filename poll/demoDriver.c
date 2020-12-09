#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>  //用户访问的接口
#include <linux/kfifo.h>
#include <linux/slab.h>
#include <linux/poll.h>

#define DEMO_NAME "kfifo_poll_driver"
#define MAX_DEVICES 8
#define FIFO_SIZE 64

static dev_t dev ;    //字符设备的设备号
static struct cdev *demo_cdev;

struct mydemo_device{
    char name[64];
    struct device* dev;
    wait_queue_head_t read_queue;
    wait_queue_head_t write_queue;
    struct kfifo _fifo; //每个设备维护自己的Kfifo
    struct fasync_struct *fasync;
};

struct _private_data{
    struct mydemo_device *device;
    char name[64];
};

static struct mydemo_device *my_device_p[MAX_DEVICES]; //全局的静态维护整个结构的指针

static int demo_open(struct inode * inode,struct file * file)
{
    unsigned int major = MAJOR(inode->i_rdev);
    unsigned int minor = MINOR(inode->i_rdev);
    struct _private_data *data;
    struct mydemo_device *device_p = my_device_p[minor]; 
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
    if(kfifo_is_empty(&device->_fifo))
    {
        if(file->f_flags & O_NONBLOCK)
            return -EAGAIN;
        printk(KERN_EMERG"%s: pid=%d, going to sleep\n",__func__,current->pid);
        ret = wait_event_interruptible(device->read_queue,!kfifo_is_empty(&device->_fifo));
        if(ret)
            return ret;
    }
    ret = kfifo_to_user(&device->_fifo,buf, buf_count,&readed_); 
    if(ret)
       return -EIO;
    if(!kfifo_is_full(&device->_fifo))
    {
        wake_up_interruptible(&device->write_queue);
        kill_fasync(&device->fasync, SIGIO, POLL_OUT);
        printk(KERN_EMERG"%s kill fasync\n", __func__);
    }
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
    if(kfifo_is_full(&device->_fifo))
    {
        if(file->f_flags & O_NONBLOCK)
            return -EAGAIN;
        printk(KERN_EMERG"%s: pid=%d, going to sleep\n",__func__,current->pid);
        ret = wait_event_interruptible(device->write_queue,!kfifo_is_full(&device->_fifo));
        if(ret)
            return ret;
    }
    ret = kfifo_from_user(&device->_fifo,buf, buf_count,&writed_); //返回没有拷贝成功的字节数
    if(ret)
        return -EIO;
    if(!kfifo_is_empty(&device->_fifo))
    {
        wake_up_interruptible(&device->read_queue);
        kill_fasync(&device->fasync, SIGIO, POLL_IN);
		printk(KERN_EMERG"%s kill fasync\n", __func__);
    }
    printk(KERN_EMERG"%s,pid = %d, writed = %d, pos = %lld \n",__func__,
                       current->pid, writed_,*ppos);
    return writed_;
}
static unsigned int demo_poll(struct file *file, poll_table *wait)
{
	int mask = 0;
	struct _private_data *data = file->private_data;
	struct mydemo_device *device = data->device;

	poll_wait(file, &device->read_queue, wait);
    poll_wait(file, &device->write_queue, wait);

	if (!kfifo_is_empty(&device->_fifo))
		mask |= POLLIN | POLLRDNORM;
	if (!kfifo_is_full(&device->_fifo))
		mask |= POLLOUT | POLLWRNORM;
	return mask;
}

static int demo_fasync(int fd,struct file *file, int on)
{
    struct _private_data *data = file->private_data;
    struct mydemo_device *device = data->device;
    printk("%s send SIGIO\n", __func__);
	
    return fasync_helper(fd, file, on, &device->fasync);
}

static const struct file_operations demo_fops =
{
    .owner = THIS_MODULE,
    .open  = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write,
    .poll = demo_poll,
    .fasync = demo_fasync
};


static int __init demo_init(void)
{
    int ret;
    int i;
    struct mydemo_device * device;
    //分配主设备号
    ret = alloc_chrdev_region(&dev,0 ,MAX_DEVICES,DEMO_NAME); 
    if (ret) 
    {
		printk("failed to allocate char device region");
		return ret;
	}
    demo_cdev = cdev_alloc();
    if (!demo_cdev)
    {
		printk("cdev_alloc failed\n");
		goto unregister_chrdev;
	}
    for ( i = 0; i < MAX_DEVICES; i++)
    {
        device = kmalloc(sizeof(struct mydemo_device),GFP_KERNEL);
        if(!device)
        {
            ret =  -ENOMEM;
            goto free_device;
        }
        sprintf(device->name,"%s%d",DEMO_NAME,i);
        my_device_p[i] = device;
        init_waitqueue_head(&device->read_queue);
		init_waitqueue_head(&device->write_queue);

        ret = kfifo_alloc(&device->_fifo,FIFO_SIZE,GFP_KERNEL);
        if(ret)
        {
            ret = -ENOMEM;
			goto free_kfifo;
        }
        printk("demo_fifo = %p\n",&device->_fifo);
    }
    cdev_init(demo_cdev,&demo_fops);
    ret = cdev_add(demo_cdev,dev,MAX_DEVICES);
    if (ret) {
		printk("cdev_add failed\n");
		goto cdev_fail;
	}
    printk("succeeded register char device: %s\n", DEMO_NAME);
    return 0;

free_kfifo:
	for (i =0; i < MAX_DEVICES; i++)
    {
		if (&my_device_p[i]->_fifo)
			 kfifo_free(&my_device_p[i]->_fifo);
    }
free_device:
	for (i =0; i < MAX_DEVICES; i++)
    {
        if (my_device_p[i])
            kfree(my_device_p[i]);
    }		
cdev_fail:
    cdev_del(demo_cdev);
unregister_chrdev:
    unregister_chrdev_region(dev,MAX_DEVICES); //卸载掉，释放回系统

    return ret;
}

static void  __exit demo_exit(void)
{
    int i;
    printk("remove Device\n");
    if(demo_cdev)
        cdev_del(demo_cdev);
    unregister_chrdev_region(dev,MAX_DEVICES);
    for (i =0; i < MAX_DEVICES; i++)
    {
        if (my_device_p[i])
            kfree(my_device_p[i]);
    }	
}


module_init(demo_init);
module_exit(demo_exit);

//模块可选信息
MODULE_LICENSE("GPL");//许可证声明
MODULE_AUTHOR("junwide");//作者声明
MODULE_DESCRIPTION("This module is a char device");//模块描述
MODULE_VERSION("V1.0");//模块别名
MODULE_ALIAS("Char Modules");//模块别名