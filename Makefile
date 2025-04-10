EXTRA_CFLAGS=-Wall
obj-m := led23_gpio.o

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules
