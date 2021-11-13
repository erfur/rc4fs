#ifndef RC4FS_FS_H
#define RC4FS_FS_H

#define FUSE_USE_VERSION 31

int fs_set_realpath(const char *);
struct fuse_operations *fs_get_ops();

#ifdef FS_CRYPTO_ENABLED
int fs_set_cryptkey(const char *);
#endif


#endif
