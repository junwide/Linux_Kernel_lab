#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>  //用户访问的接口
#include <linux/kfifo.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include "OLED.h"

#define DEMO_NAME "oled_driver"
//static struct device * mydemo_device;
DEFINE_KFIFO(demo_fifo, char, 64);


static int bus = 5;  //i2c-5
static int addr = 0x3C;

module_param(bus, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(addr, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

MODULE_PARM_DESC(bus, " I2C Bus number, default 5");
MODULE_PARM_DESC(addr, " I2C device address, default 0x3C");


static struct i2c_board_info board_info = {
    I2C_BOARD_INFO("oled_0.96",0x3C),
};
static const struct i2c_device_id oled_id[] = {
    {"oled_0.96",0},
    {}
};

static struct i2c_driver oled_driver = {
    	.driver = {
		.name = "oled_0.96",
		.owner = THIS_MODULE,
	},
	.probe = OLED_Probe,
	.remove = OLED_Remove,
	.id_table = oled_id,
};

static struct i2c_client *client;


static int demo_open(struct inode * inode,struct file * file)
{
    //for app
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

    // infomation show 
    // OLED configure
    return 0;
}
static ssize_t demo_write(struct file * file, const char __user *buf, size_t buf_count,\
                            loff_t *ppos)
{
	char *str = vmalloc(buf_count);
	if (copy_from_user(str, buf, buf_count))
	{
		vfree(str);
		return -EFAULT;
	}
	*(str + buf_count) = '\0';
    OLED_setFont(font8x8);
    divide_Cmd(str);
	//OLED_putString(0,57,str,1);
    OLED_refresh_gram();
	vfree(str);
	return buf_count;
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
    static struct i2c_adapter * adapter;
    int ret;
    adapter =  i2c_get_adapter(bus);    //获取总线适配器
    if(!adapter)
        return -EINVAL;
    client = i2c_new_device(adapter, &board_info);   //总线中加入设备
	if (!client) {
		i2c_put_adapter(adapter);
		return -EINVAL;
	}
    i2c_put_adapter(adapter);              // 把加了设备的再放回去
    ret = i2c_add_driver(&oled_driver);
	if(ret){
		printk(KERN_ERR "I2C error\n");
		return ret;
	}
    ret = misc_register(&mydemo_device_misc_d);
    if(ret) 
    {
        printk(KERN_EMERG"Faild in register misc device");
        return ret;
    }
  //  mydemo_device = mydemo_device_misc_d.this_device;
    printk(KERN_EMERG"success register oled device: %s\n",DEMO_NAME);
    return 0;
}

static void  __exit demo_exit(void)
{
    printk("remove Device\n");
    misc_deregister(&mydemo_device_misc_d);
    i2c_unregister_device(client);
    i2c_del_driver(&oled_driver);
}


module_init(demo_init);
module_exit(demo_exit);

//模块可选信息
MODULE_LICENSE("GPL");//许可证声明
MODULE_AUTHOR("junwide");//作者声明
MODULE_DESCRIPTION("This module is a OLED device");//模块描述
MODULE_VERSION("V1.0");//模块别名
MODULE_ALIAS("OLED Modules");//模块别名