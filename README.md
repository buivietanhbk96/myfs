# myfs
my first filesystem in linux

after compile, run command to test
1. dd bs=1M count=100 if=/dev/zero of=image
2. mkdir mount
3. sudo mount -o loop -t myfs image /home/vanh/Desktop/C_Training/kernel_module/myfs/mount/
