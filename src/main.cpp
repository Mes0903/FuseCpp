/**
 * Less Simple, Yet Stupid Filesystem.
 *
 * Mohammed Q. Hussain - http://www.maastaar.net
 *
 * This is an example of using FUSE to build a simple filesystem. It is a part of a tutorial in MQH Blog with the title "Writing Less Simple, Yet Stupid Filesystem Using FUSE in C": http://maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/
 *
 * License: GNU GPL
 */

#define FUSE_USE_VERSION 30

#include "Fuse.h"

#include <fuse.h>

#include <iostream>

static struct fuse_operations operations;

static void set_operations()
{
  operations.getattr = do_getattr;
  operations.readdir = do_readdir;
  operations.read = do_read;
  operations.mkdir = do_mkdir;
  operations.mknod = do_mknod;
  operations.write = do_write;
  operations.create = do_create;
  operations.utimens = do_utimens;
  operations.rmdir = do_rmdir;
  operations.unlink = do_unlink;
}

int main(int argc, char *argv[])
{
  set_operations();

  std::cout << "Do you want to decrypt the files? (1/0): ";
  std::cin >> DO_DECRYPT;

  return fuse_main(argc, argv, &operations, NULL);
}