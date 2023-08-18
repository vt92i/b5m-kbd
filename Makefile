obj-m += b5m-kbd.o

all:
	@$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) modules

clean:
	@$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) clean

install:
	mkdir -p /lib/modules/$(shell uname -r)/extra
	cp b5m-kbd.ko /lib/modules/$(shell uname -r)/extra
	depmod -a
	echo b5m-kbd > /etc/modules-load.d/b5m-kbd.conf
	modprobe -v b5m-kbd

uninstall:
	-modprobe -rv b5m-kbd 
	rm -f /lib/modules/$(shell uname -r)/extra/b5m-kbd.ko
	depmod -a
	rm -f /etc/modules-load.d/b5m-kbd.conf
