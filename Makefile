obj-m := myfilesystem.o
myfilesystem-objs := myfs.o dir.o file.o inode.o super.o

all: ko mkfs-myfs

ko:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

mkfs-myfs_SOURCES:
	mkfs-myfs.c myfs.h

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm mkfs-myfs