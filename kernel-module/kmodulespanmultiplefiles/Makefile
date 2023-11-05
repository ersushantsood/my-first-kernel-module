obj-m += startstopmodule.o
startstopmodule-objs := startkmodule.o stopkmodule.o

PWD := $(CURDIR)

all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean