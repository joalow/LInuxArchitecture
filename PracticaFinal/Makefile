obj-m =  pfinal.o # pfinal.c no tiene que existir
pfinal-objs = list.o control.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
