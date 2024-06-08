#include "Fuse.h"

#include <errno.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include <iostream>
#include <map>
#include <vector>
#include <utility>

static std::string dir_list[256];
static int curr_dir_idx = -1;

static std::string files_list[256];
static int curr_file_idx = -1;

static std::string files_content[256];
static int curr_file_content_idx = -1;

static std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> key_iv_map;

void store_key_iv(const std::string &path, const unsigned char *key, const unsigned char *iv)
{
  key_iv_map[path] = { std::vector<unsigned char>(key, key + 32), std::vector<unsigned char>(iv, iv + 16) };
}

void retrieve_key_iv(const std::string &path, unsigned char *key, unsigned char *iv)
{
  auto it = key_iv_map.find(path);
  if (it != key_iv_map.end()) {
    memcpy(key, it->second.first.data(), 32);
    memcpy(iv, it->second.second.data(), 16);
  }
}

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;
  int len;
  int ciphertext_len;

  // 創建和初始化上下文
  if (!(ctx = EVP_CIPHER_CTX_new()))
    handleErrors();

  // 初始化加密操作
  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  // 提供要加密的數據，這可以被調用多次
  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  // 完成加密操作
  if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
    handleErrors();
  ciphertext_len += len;

  // 清理
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;
  int len;
  int plaintext_len;

  // 創建和初始化上下文
  if (!(ctx = EVP_CIPHER_CTX_new()))
    handleErrors();

  // 初始化解密操作
  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  // 提供要解密的數據，這可以被調用多次
  if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  plaintext_len = len;

  // 完成解密操作
  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    handleErrors();
  plaintext_len += len;

  // 清理
  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}

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

  files_content[file_idx] = std::string(new_content);
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
    return -ENOENT;

  if (files_content[file_idx].empty())
    return 0;

  if (DO_DECRYPT) {
    unsigned char key[32] = {};
    unsigned char iv[16] = {};
    unsigned char ciphertext[1024] = {};
    unsigned char plaintext[1024] = {};

    retrieve_key_iv(path, key, iv);

    size_t ciphertext_len = files_content[file_idx].size();
    memcpy(ciphertext, files_content[file_idx].data(), ciphertext_len);

    int plaintext_len = decrypt(ciphertext, ciphertext_len, key, iv, plaintext);
    if (plaintext_len < offset + size)
      size = plaintext_len - offset;

    plaintext[plaintext_len] = '\0';
    memcpy(buffer, plaintext + offset, size);
  }
  else {
    size_t content_len = files_content[file_idx].size();
    if (content_len < offset + size)
      size = content_len - offset;

    memcpy(buffer, files_content[file_idx].data() + offset, size);
  }

  return size;
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
  int file_idx = get_file_index(path);
  if (file_idx == -1)
    return -ENOENT;

  // 密鑰和 IV
  unsigned char key[32] = {};
  unsigned char iv[16] = {};
  unsigned char ciphertext[1024] = {};

  // 生成密鑰和 IV
  RAND_bytes(key, sizeof(key));
  RAND_bytes(iv, sizeof(iv));

  // 加密
  int ciphertext_len = encrypt((unsigned char *) buffer, size, key, iv, ciphertext);
  write_to_file(path, reinterpret_cast<const char *>(ciphertext));

  store_key_iv(path, key, iv);

  return size;
}

int do_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
  if (get_file_index(path) != -1)
    return -EEXIST;

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