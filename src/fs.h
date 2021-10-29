#ifndef RC4FS_FS_H
#define RC4FS_FS_H

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void *fs_init(struct fuse_conn_info *, struct fuse_config *);
int fs_set_realpath(const char *);
int fs_getattr(const char *, struct stat *, struct fuse_file_info *);
int fs_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
int fs_open(const char *, struct fuse_file_info *);
int fs_read (const char *, char *, size_t, off_t, struct fuse_file_info *);
int fs_write(const char *, char *, size_t, off_t, struct fuse_file_info *);

#ifdef FS_CRYPTO_ENABLED
int fs_set_cryptkey(const char *);
int fs_crypt(int, char *, size_t, off_t);
#endif

struct fuse_operations *fs_get_ops();

#endif
