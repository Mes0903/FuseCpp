# intsall tool
sudo apt-get update
sudo apt-get install gcc fuse libfuse-dev make cmake

# Compiling & Mounting The Filesystem
gcc lsysfs.c -o lsysfs `pkg-config fuse --cflags --libs`
./lsysfs -f [mount point]

# test
- this homework implement the following command 
cd, ls, mkdir, touch, rmdir, rm, echo, cat