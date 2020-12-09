#第一次执行KERNELRELEASE是空的,所以执行else里面的
ifneq ($(KERNELRELEASE),)
name = _hello
$(name)-objs :=hello.o
obj-m :=$(name).o
#else块
else
name = _hello
KDIR:= /lib/modules/$(shell uname -r)/build 
INSTALL_MOD_PATH = /lib/modules/$(shell uname -r)/
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules   #编译
install:
	cp $(name).ko $(INSTALL_MOD_PATH)$(name).ko  #将modules复制到指定目录
	depmod -a         # 并更新依赖环境
	modprobe $(name)   # 加载Modules
boot:
	echo $(name) >> /etc/modules-load.d/modules.conf  #加入开启自启
rboot:
	sed -i "/${name}/d" /etc/modules-load.d/modules.conf  # 删除开机自启
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.order *.mod
	rm -f $(INSTALL_MOD_PATH)$(name).ko
	depmod -a
endif