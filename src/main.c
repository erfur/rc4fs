#include "rc4.h"
#include "fs.h"

void usage() {
    printf("rc4fs: Encrypted FUSE filesystem\n");
    printf("Usage:\n");
    printf("    $ rc4fs [FUSE and mount options] mountpoint encryptedfolder rc4key\n");
}

int main(int argc, char **argv) {
    if (argc < 4) {
        usage();
        return 0;
    }

    ARGV_RC4_KEY = argv[argc-1];
    ARGV_REAL_PATH = argv[argc-2];

    return fuse_main(argc-2, argv, &fs_ops, 0);
}
