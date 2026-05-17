obj-m :=add.o

KERNELDIR = /lib/modules/$(shell uname -r)/build
#KERNELDIR = /home/cdac/DeviceDrivers/linux/
 	
#current working directory
PWD := $(shell pwd)

default : #build the kernel module
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 

install : #install the kernel module
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean: #clean the build files
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean 
