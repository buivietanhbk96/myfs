# myfs
my first filesystem in linux

after compile, run command to test
1. make
2. sudo su
3. dd bs=4096 count=100 if=/dev/zero of=image
4. mkdir mount
5. ./mkfs-myfs image
6. dmesg -C

# Creating a loop back device and formatting it with our shiny, brand new myfs


7. mount -o loop -t myfs image /home/vanh/Desktop/C_Training/kernel_module/myfs/mount/
8. dmesg
9. umount mount/
10. rmmod myfilesystem.ko
