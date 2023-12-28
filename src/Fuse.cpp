#include "Fuse.h"

#include <errno.h>
#include <string.h>
#include <errno.h>

#include <iostream>

void add_dir(const char *dir_name)
{
  curr_dir_idx++;
  dir_list[curr_dir_idx] = dir_name;
}

int is_dir(const char *path)
{
  path++;    // Eliminating "/" in the path

  for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++)
    if (path == dir_list[curr_idx])
      return 1;

  return 0;
}

void add_file(const char *filename)
{
  curr_file_idx++;
  files_list[curr_file_idx] = filename;

  curr_file_content_idx++;
  files_content[curr_file_content_idx] = "";
}

int is_file(const char *path)
{
  path++;    // Eliminating "/" in the path

  for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
    if (path == files_list[curr_idx])
      return 1;

  return 0;
}

int get_file_index(const char *path)
{
  path++;    // Eliminating "/" in the path

  for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
    if (path == files_list[curr_idx])
      return curr_idx;

  return -1;
}

void write_to_file(const char *path, const char *new_content)
{
  int file_idx = get_file_index(path);

  if (file_idx == -1)    // No such file
    return;

  files_content[file_idx] = new_content;
}

int do_getattr(const char *path, struct stat *st)
{
  st->st_uid = getuid();    // The owner of the file/directory is the user who mounted the filesystem
  st->st_gid = getgid();    // The group of the file/directory is the same as the group of the user who mounted the filesystem
  st->st_atime = time(NULL);    // The last "a"ccess of the file/directory is right now
  st->st_mtime = time(NULL);    // The last "m"odification of the file/directory is right now

  if (strcmp(path, "/") == 0 || is_dir(path)) {
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink = 2;    // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
  }
  else if (is_file(path)) {
    st->st_mode = S_IFREG | 0644;
    st->st_nlink = 1;
    st->st_size = 1024;
  }
  else {
    return -ENOENT;
  }

  return 0;
}

int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  filler(buffer, ".", NULL, 0);    // Current Directory
  filler(buffer, "..", NULL, 0);    // Parent Directory

  if (strcmp(path, "/") == 0)    // If the user is trying to show the files/directories of the root directory show the following
  {
    for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++)
      filler(buffer, dir_list[curr_idx].data(), NULL, 0);

    for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
      filler(buffer, files_list[curr_idx].data(), NULL, 0);
  }

  return 0;
}

int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  int file_idx = get_file_index(path);

  if (file_idx == -1)
    return -1;

  char *content = files_content[file_idx].data();

  memcpy(buffer, content + offset, size);

  return strlen(content) - offset;
}

int do_mkdir(const char *path, mode_t mode)
{
  path++;
  add_dir(path);

  return 0;
}

int do_mknod(const char *path, mode_t mode, dev_t rdev)
{
  path++;
  add_file(path);

  return 0;
}

int do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info)
{
  write_to_file(path, buffer);

  return size;
}

int do_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
  if (get_file_index(path) != -1)
    return -1;
  path++;
  add_file(path);
  return 0;
}

int do_utimens(const char *path, const struct timespec tv[2])
{
  return 0;
}

int do_rmdir(const char *path)
{
  path++;
  int touch_target = 0;
  int target_dir_idx = curr_dir_idx;

  for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++) {
    if (path == dir_list[curr_idx]) {
      touch_target = 1;
      target_dir_idx = curr_idx;
      break;
    }
  }

  for (int curr_idx = target_dir_idx; curr_idx < curr_dir_idx; curr_idx++) {
    dir_list[curr_idx] = dir_list[curr_idx + 1];
  }

  curr_dir_idx -= touch_target;

  return 0;
}

int do_unlink(const char *path)
{
  path++;

  int touch_target = 0;
  int target_file_idx = curr_file_idx;
  for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++) {
    if (path == files_list[curr_idx]) {
      touch_target = 1;
      target_file_idx = curr_idx;
      break;
    }
  }

  for (int curr_idx = target_file_idx; curr_idx <= curr_file_idx; curr_idx++) {
    files_list[curr_idx] = files_list[curr_idx + 1];
    files_content[curr_idx] = files_content[curr_idx + 1];
  }

  curr_file_idx -= touch_target;
  curr_file_content_idx -= touch_target;

  return 0;
}