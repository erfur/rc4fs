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

int fs_set_realpath(const char *);
struct fuse_operations *fs_get_ops();

#ifdef FS_CRYPTO_ENABLED
int fs_set_cryptkey(const char *);
#endif


#endif
