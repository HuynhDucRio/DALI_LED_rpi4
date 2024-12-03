KERN_SRC:= /home/duc/Desktop/linux
PWD   := $(shell pwd)
obj-m := my_DALI_drv.o
all:
	make clean
	make -C $(KERN_SRC) \
	ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- \
	M=$(PWD) modules
clean:
	make -C $(KERN_SRC) \
	ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- \
	M=$(PWD) clean
scp:
	scp *.ko pi@raspberrypi.local:/home/pi/DALI_int
ssh:
	ssh -X pi@raspberrypi.local
info:
	modinfo my_DALI_drv.ko
