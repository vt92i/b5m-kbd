obj-m += b5m-kdb.o

all:
	@$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) modules

clean:
	@$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) clean
