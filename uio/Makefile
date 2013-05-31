obj-m := simple.o
all:
	make -Wall -C /lib/modules/`uname -r`/build M=`pwd` modules
