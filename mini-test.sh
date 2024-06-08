#!/bin/bash

echo "[TEST]: mkdir mnt"
mkdir mnt

echo "[TEST-1]: TESTING WITH DECRYPT"
./FUSECpp mnt <<< "1"
echo "1"

echo "[TEST-1]: cd mnt"
cd mnt

echo "[TEST-1]: ls"
ls

echo "[TEST-1]: mkdir folder"
mkdir folder

echo "[TEST-1]: ls"
ls

echo "[TEST-1]: rmdir folder"
rmdir folder

echo "[TEST-1]: ls"
ls

echo "[TEST-1]: touch file"
touch file

echo "[TEST-1]: ls"
ls

echo "[TEST-1]: echo \"hello\" >> file"
echo "hello" >> file

echo "[TEST-1]: cat file"
cat file

echo "[TEST-1]: rm file"
rm file

echo "[TEST-1]: ls"
ls

echo "[TEST-1]: cd .."
cd ..

echo "[TEST-1]: umount mnt"
umount mnt

# TEST WITH NO DECRYPT

echo "[TEST-0]: TESTING WITH NO DECRYPT"
./FUSECpp mnt <<< "0"
echo "0"

echo "[TEST-0]: cd mnt"
cd mnt

echo "[TEST-0]: ls"
ls

echo "[TEST-0]: mkdir folder"
mkdir folder

echo "[TEST-0]: ls"
ls

echo "[TEST-0]: rmdir folder"
rmdir folder

echo "[TEST-0]: ls"
ls

echo "[TEST-0]: touch file"
touch file

echo "[TEST-0]: ls"
ls

echo "[TEST-0]: echo \"hello\" >> file"
echo "hello" >> file

echo "[TEST-0]: cat file"
cat file

echo "[TEST-0]: rm file"
rm file

echo "[TEST-0]: ls"
ls

echo "[TEST-0]: cd .."
cd ..

echo "[TEST-0]: umount mnt"
umount mnt

echo "[TEST-0]: END"

