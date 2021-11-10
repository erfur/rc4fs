#include "fs.h"
#include "rc4plus.h"
#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <errno.h>

char ARGV_REAL_PATH[PATH_MAX];

void *fs_init(struct fuse_conn_info *, struct fuse_config *);
int fs_getattr(const char *, struct stat *, struct fuse_file_info *);
int fs_mknod(const char *, mode_t, dev_t);
int fs_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
int fs_open(const char *, struct fuse_file_info *);
int fs_release(const char *, struct fuse_file_info *);
int fs_read (const char *, char *, size_t, off_t, struct fuse_file_info *);
int fs_write(const char *, char *, size_t, off_t, struct fuse_file_info *);
//int fs_unlink(const char *);
//int fs_setattr(hhhhhh)

#ifdef FS_CRYPTO_ENABLED
int fs_crypt(int, char *, size_t, off_t);
uint64_t fetch_random_number();

#define FS_CRYPT_SECTOR_SIZE 4096
char crypt_key[257] = {0};

typedef struct crypt_ctx_s {
    int in_use;
    uint64_t key;
    /* rc4struct *rc4_ctx; */
} crypt_ctx;

crypt_ctx crypt_ctxs[1024];
#endif

struct fuse_operations fs_ops = {
    .init = fs_init,
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .open = fs_open,
    .read = fs_read,
    .write = fs_write,
    .release = fs_release,
    .mknod = fs_mknod,
//    .unlink = fs_unlink,
//    .setattr = fs_setattr
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

    // snprintf returns the number of bytes written
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

void *fs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    (void) conn;
    cfg->use_ino = 1;
    //cfg->nullpath_ok = 1;
    /* Pick up changes from lower filesystem right away. This is
       also necessary for better hardlink support. When the kernel
       calls the unlink() handler, it does not know the inode of
       the to-be-removed entry and can therefore not invalidate
       the cache of the associated inode - resulting in an
       incorrect st_nlink value being reported for any remaining
       hardlinks to this inode. */
    cfg->entry_timeout = 0;
    cfg->attr_timeout = 0;
    cfg->negative_timeout = 0;

#ifdef FS_CRYPTO_ENABLED
    memset(crypt_ctxs, 0, sizeof crypt_ctxs);
#endif

    return NULL;
}

int fs_getattr(const char *path, struct stat *stbuf,
        struct fuse_file_info *fi) {
    int ret = 0;
    char *real_path;

    fs_log("getattr", "called for %s", path);

    if (fi) {
       ret = fstat(fi->fh, stbuf);
    } else if ((real_path = _fs_realpath(path))) {
        fs_log("getattr", "real path: %s", real_path);
        ret = stat(real_path, stbuf);
        free(real_path);
    } else {
        ret = -ENOENT;
    }

    if (ret == -1)
        return -errno;

    return 0;
}

int fs_mknod(const char *path, mode_t mode, dev_t rdev) {
    int ret;
    char *real_path;

    if (real_path = _fs_realpath(path)) {
        ret = mknod(real_path, mode, rdev);
    } else {
        return -ENOENT;
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
    int ret = 0, fd;
    ssize_t fsize;
    uint64_t key = 0;
    rc4struct *crypt_ctx;

    fs_log("open", "called for %s", path);
    if ((real_path = _fs_realpath(path))) {

#ifdef FS_CRYPTO_ENABLED
        if (access(real_path, F_OK) != 0) {
            fs_log("open", "generating a new iv key for the file");

            key = fetch_random_number();
            if (key == 0) {
                fs_log("open", "random number generator failed!");
                return -1;
            }
        } else {

            fs_log("open", "fetching the iv key from the file");

            if ((fd = open(real_path, O_RDWR)) == -1) {
                fs_log("open", "cannot open file to extract iv key");
                return -1;
            }

            if ((fsize = lseek(fd, 0, SEEK_END)) == 0) {
                fs_log("open", "empty file, generating a new iv key");

                key = fetch_random_number();
                if (key == 0) {
                    fs_log("open", "random number generator failed!");
                    return -1;
                }

            } else if (fsize < sizeof key) {

                fs_log("open", "file is too short to hold a key, invalid file.");
                return -1;

            } else {

                lseek(fd, fsize - sizeof key, SEEK_SET);
                read(fd, &key, sizeof key);
                ftruncate(fd, fsize - sizeof key);
                close(fd);
            }

            fs_log("open", "key for fd %d: %016llx", fd, key);
        }
#endif

        fd = open(real_path, fi->flags);
        fi->fh = fd;

#ifdef FS_CRYPTO_ENABLED
        fs_crypt_init_fd(fd, key);
#endif

        free(real_path);

    } else {
        ret = -ENOENT;
    }

    return ret;
}

int fs_release(const char *path, struct fuse_file_info *fi) {
    (void) path;
    int fd = fi->fh, ffd;
    char *real_path;

    close(fd);

#ifdef FS_CRYPTO_ENABLED
    if (real_path = _fs_realpath(path)) {

        if ((ffd = open(real_path, O_WRONLY)) == -1) {
            fs_log("release", "could not open the file to save the iv key");
            return -1;
        }

        lseek(ffd, 0, SEEK_END);

        fs_log("release", "iv key for fd %d: %016llx", fd, crypt_ctxs[fd].key);
        write(ffd, &(crypt_ctxs[fd].key), sizeof crypt_ctxs[fd].key);

        crypt_ctxs[fd].in_use = 0;
        /* rc4plus_destroy(crypt_ctxs[fd].rc4_ctx); */

        close(ffd);
    }
#endif

    return 0;
}

int fs_read(const char *path, char *buf, size_t size,
        off_t offset, struct fuse_file_info *fi) {
    int ret=0, fd;
    char *real_path;

    if ((real_path = _fs_realpath(path))) {

        fd = fi->fh;

        if (lseek(fd, offset, SEEK_SET) != offset) {
            return 0;
        }

        ret = read(fd, buf, size);

#ifdef FS_CRYPTO_ENABLED
        fs_crypt(fd, buf, ret, offset);
#endif

        free(real_path);
    } else {
        ret = 0;
    }

    return ret;
}

int fs_write(const char *path, char *buf, size_t size,
        off_t offset, struct fuse_file_info *fi) {
    int ret=0, fd;
    char *real_path;

    fs_log("write", "called for %s", path);
    fs_log("write", "fd=%d", fi->fh);

    if ((real_path = _fs_realpath(path))) {

        fd = fi->fh;

#ifdef FS_CRYPTO_ENABLED
        fs_crypt(fd, buf, size, offset);
#endif

        if (lseek(fd, offset, SEEK_SET) != offset) {
            return 0;
        }

        ret = write(fd, buf, size);

        free(real_path);
    } else {
        ret = 0;
    }

    return ret;
}

#ifdef FS_CRYPTO_ENABLED

uint64_t fetch_random_number() {
    int fd;
    uint64_t result;

    if ((fd = open("/dev/urandom", O_RDONLY)) == -1) {
        return 0;
    }

    if (read(fd, &result, sizeof result) < sizeof result) {
        return 0;
    }

    close(fd);
    return result;
}

int fs_set_cryptkey(const char *key) {
    if (strlen(key) == 0)
        return -1;
    strncpy(crypt_key, key, 256);
    return 0;
}

void fs_crypt_init_fd(int fd, uint64_t key) {
    /* rc4struct *rc4_ctx; */

    if (crypt_ctxs[fd].in_use) {
        fs_log("open", "existing crypt context!");
        /* rc4plus_destroy(crypt_ctxs[fd].rc4_ctx); */
    }

    /* rc4_ctx = rc4plus_init(crypt_key, &key); */

    /* if (rc4_ctx == NULL) { */
    /*     fs_log("open", "failed to create crypt context."); */
    /*     return -1; */
    /* } */

    /* crypt_ctxs[fd].rc4_ctx = rc4_ctx; */
    fs_log("init_fd", "iv key for fd %d: %016llx", fd, key);
    crypt_ctxs[fd].key = key;
    crypt_ctxs[fd].in_use = 1;
}

uint64_t fs_crypt_nth_rand(uint64_t s, off_t n) {
    return ((0xabba5dede0bacada ^ s) * n);
}

int fs_crypt(int fd, char *buf, size_t len, off_t off) {
    size_t sz;
    off_t soff;
    int ret;
    rc4struct *rc4_ctx;
    uint64_t iv;
    off_t n;

    if (crypt_ctxs[fd].in_use == 0) {
        fs_log("crypt", "no crypto context!");
        return -1;
    }

    while(len) {
        soff = off % FS_CRYPT_SECTOR_SIZE;
        sz = FS_CRYPT_SECTOR_SIZE - soff;

        if (len < sz)
            sz = len;

        fs_log("crypt", "sz=%u", sz);
        fs_log("crypt", "off=%u", off);

        n = off / FS_CRYPT_SECTOR_SIZE;
        iv = fs_crypt_nth_rand(crypt_ctxs[fd].key, n);

        fs_log("crypt", "iv=%016llx", iv);
        fs_log("crypt", "n=%u", n);

        rc4_ctx = rc4plus_init(crypt_key, &iv);

        ret = rc4_ctx->enc_dec_func(buf, sz, soff, rc4_ctx->state);

        if (ret != 0) {
            fs_log("crypt", "error in enc/dec.");
        }

        rc4plus_destroy(rc4_ctx);

        off += sz;
        buf += sz;
        len -= sz;
    }

    return 0;
}

#endif
