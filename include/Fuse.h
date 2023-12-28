#ifndef FUSE_H
#define FUSE_H

#include <fuse.h>

#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include <string>

inline std::string dir_list[256];
inline int curr_dir_idx = -1;

inline std::string files_list[256];
inline int curr_file_idx = -1;

inline std::string files_content[256];
inline int curr_file_content_idx = -1;

void add_dir(const char *dir_name);

int is_dir(const char *path);

void add_file(const char *filename);

int is_file(const char *path);

int get_file_index(const char *path);

void write_to_file(const char *path, const char *new_content);

int do_getattr(const char *path, struct stat *st);

int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi);

int do_mkdir(const char *path, mode_t mode);

int do_mknod(const char *path, mode_t mode, dev_t rdev);

int do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info);

int do_create(const char *path, mode_t mode, struct fuse_file_info *info);

int do_utimens(const char *, const struct timespec tv[2]);

int do_rmdir(const char *path);

int do_unlink(const char *path);

#endif