#第一次执行KERNELRELEASE是空的,所以执行else里面的
ifneq ($(KERNELRELEASE),)
name =_kfifo_bl_driver
$(name)-objs := demoDriver.o
obj-m :=$(name).o

else
name =_kfifo_bl_driver
KBUILD_EXTRA_SYMBOLS += /home/junwide/Kernel_Module/hello/Module.symvers
export KBUILD_EXTRA_SYMBOLS
KDIR:= /lib/modules/$(shell uname -r)/build 
INSTALL_MOD_PATH = /lib/modules/$(shell uname -r)/
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
install:
	cp $(name).ko $(INSTALL_MOD_PATH)$(name).ko
	depmod -a
	modprobe $(name)
boot:
	echo $(name) >> /etc/modules-load.d/modules.conf 
rboot:
	sed -i "/${name}/d" /etc/modules-load.d/modules.conf
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.order *.mod
	rm -f $(INSTALL_MOD_PATH)$(name).ko
	depmod -a
endif
