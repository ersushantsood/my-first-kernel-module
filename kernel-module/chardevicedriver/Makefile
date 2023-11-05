# Single parent makefile can be used but kept separate one just to avoid building multiple modules for testing.

obj-m += chardevicedriverexample.o

PWD := $(CURDIR)

all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean