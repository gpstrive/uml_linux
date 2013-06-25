obj-m:=dev_mem.o
obj-m+=kprobe.o
all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules 
clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
