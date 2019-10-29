# Simple implementation of the Linux filesystem
MyFS is simple implementation of FileSystem with basic on-disk layout.
The main goal of this FS is educational puropse.


## How to start?
# to create kernel modules
make

# to create initial filesystem on device i.e. for device /dev/sdb use mkfs
./mkfs_myfs /dev/sdb

# you can see disk layout in the hex if you will, to do so:
dd if=/dev/sdb bs=1k count=20 | hexdump

# insert kernel module
# (Please mind the fact that it is learning module,
# so good idea is to run it on VM or save all data before!)
insmod ./myfilesystem.ko

# Then mount filesystem (again given device /dev/sdb, and mount point /mnt):
mount -t myfs /dev/sdb /mnt/myfs

# verfy if the fs is mounted properly
mount | grep myfs

ls -al /mnt
...

# create file in dummyfs
touch /mnt/test

# do simple 'hello word'
echo hello_from_myfs > /mnt/test
cat /mnt/myfs/test
...
```
