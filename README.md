# myfs
my first filesystem in linux

after compile, run command to test
1. make
# Creating a loop back device and formatting it with our shiny, brand new myfs
2. sudo su
3. dd bs=4096 count=100 if=/dev/zero of=image
4. mkdir mount
5. ./mkfs-myfs image
# Test module filesystem
6. dmesg -C
7. insmod myfilesystem.ko
8. dmesg
9. mount -o loop -t myfs image /home/vanh/Desktop/C_Training/kernel_module/myfs/mount/
10. dmesg
11. umount mount/
12. rmmod myfilesystem.ko
