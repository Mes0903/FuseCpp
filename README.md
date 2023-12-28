# intsall tool

```
sudo apt-get update
sudo apt-get install gcc fuse libfuse-dev make cmake
```

# Compiling

```
mkdir mnt
mkdir build && cd build
cmake ..
cmake --build .
```

# Testing

demo video：

```
cd ..
./FUSECpp -f mnt
```

then open an new terminal

## cd

```
cd mnt
```

## ls

```
ls
```

## mkdir

```
mkdir test_folder
```

## rmdir

```
rmdir test_folder
```

## touch

```
touch test
```

## rm

```
rm test
```

## echo

```
echo "Test String" >> test
```

## cat

```
cat test
```