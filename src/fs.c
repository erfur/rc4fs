#include "fs.h"

struct fuse_operations fs_ops {
    .getxattr = fs_getxattr,
    .readdir = fs_readdir,
    .open = fs_open,
    .read = fs_read,
    .write = fs_write
};

char *_fs_realpath(char *path) {
    int fname_offset;

    if ((fname_offset = strrchr(path, '/')) == NULL) {
        return NULL;
    }

    fname_offset += 1;

    return asprintf("%s/%s", ARGV_REAL_PATH, fname_offset);
}

inline void fs_log(char *fcn, char *msg) {
    fprintf(stderr, "[%s]: %s\n", fcn, msg);
}

int fs_getattr(const char *path, struct stat *stbuf,
               struct fuse_file_info *fi) {
    int ret = 0;
    char *real_path;

    if ((real_path = _fs_realpath(path))) {
        ret = stat(real_path, stbuf); 
        free(real_path);
    } else {
        ret = -ENOENT;
    }

    return ret;
}

int fs_readdir(const char *path, void *buffer, 
               fuse_fill_dir_t filler, off_t offset,
               struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;

    dp = (DIR *)fi->fh;

    do {
        if ((de = readdir(dp))) {
            filler(buffer, de->dname, NULL, 0, 0);
        }
    } while (de);

    return 0;
}

int fs_open(const char *path, struct fuse_file_info *fi) {
    char *real_path;
    int ret = 0;

    if ((real_path = _fs_realpath(path))) {
        fi->fh = open(path, fi->flags);
        free(real_path);
    } else {
        ret = -ENOENT;
    }

    return ret;
}
