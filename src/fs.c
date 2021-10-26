#include "fs.h"
#include <linux/limits.h>

char ARGV_REAL_PATH[PATH_MAX];

struct fuse_operations fs_ops = {
    .getattr = fs_getattr,
    //    .getxattr = fs_getxattr,
    .readdir = fs_readdir,
    .open = fs_open,
    //    .read = fs_read,
    //    .write = fs_write
};

struct fuse_operations *fs_get_ops() {
    return &fs_ops;
}

int fs_set_realpath(const char *path) {
    char buf[PATH_MAX];
    int ret;

    if (getcwd(buf, PATH_MAX)) {
        ret = snprintf(ARGV_REAL_PATH, PATH_MAX, "%s/%s", buf, path);
    }
    
    // snprintf returns no of bytes written
    if (ret)
        return 0;
    return -1;
}

char *_fs_realpath(char *path) {
    char *fname, *ptr, *out;
    int ret;

    fs_log("fs_realpath", "called for %s", path);

    ret = asprintf(&out, "%s%s", ARGV_REAL_PATH, path);

    if (ret) {
        fs_log("fs_realpath", "result: %s", out);
        return out;
    } else {
        return NULL;
    }
}

void fs_log(const char *fcn, const char *format, ...) {
    va_list ap;
    va_start(ap, format);

    fprintf(stderr, "[%s]: ", fcn);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
}

int fs_getattr(const char *path, struct stat *stbuf,
        struct fuse_file_info *fi) {
    int ret = 0;
    char *real_path;

    fs_log("getattr", "called for %s", path);

    if ((real_path = _fs_realpath(path))) {
        fs_log("getattr", "real path: %s", real_path);
        ret = stat(real_path, stbuf); 
        free(real_path);
    } else {
        ret = -ENOENT;
    }

    return ret;
}

// int fs_getxattr(const char *path, const char *name,
//                 char *value, size_t size) {
//     int ret = 0;
//     char *real_path;
// 
//     fs_log("getxattr", "called");
// 
//     if ((real_path = _fs_realpath(path))) {
//         ret = stat(real_path, name, value, size); 
//         free(real_path);
//     } else {
//         ret = -ENOENT;
//     }
// 
//     return ret;
// }

int fs_readdir(const char *path, void *buffer, 
        fuse_fill_dir_t filler, off_t offset,
        struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    char *real_path;

    dp = (DIR *)fi->fh;

    if (!dp) {
        if ((real_path = _fs_realpath(path))) {
            fs_log("getattr", "real path: %s", real_path);
            dp = opendir(real_path);
            free(real_path);
        } else {
            return -ENOENT;
        }
    }
    if (!dp) {
        return -1;
    }

    fs_log("readdir", "called for %s", path);
    fs_log("readdir", "handle: %d", dp);

    do {
        if (de = readdir(dp)) {
            filler(buffer, de->d_name, NULL, 0, 0);
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
