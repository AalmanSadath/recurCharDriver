obj-m += recur.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

sendToRecur: sendToRecur.c
	gcc -o sendToRecur sendToRecur.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f sendToRecur

