#include "rc4.h"
#include "fs.h"
#include <unistd.h>

void usage() {
    printf("rc4fs: Encrypted FUSE filesystem\n");
    printf("Usage:\n");
    printf("    $ rc4fs [FUSE and mount options] mountpoint encryptedfolder rc4key\n");
    printf("\nFUSE help:\n");
    fuse_lib_help(NULL);
}

int main(int argc, char **argv) {
    char buf[1024];
    int ret;

    if (argc == 1) {
        usage();
        return 0;
    }

    if ( rc4_set_key(argv[argc-1]) != 0 ) {
        printf("Invalid key length\n");
        return 1;
    }

    if (fs_set_realpath(argv[argc-2]) != 0) {
        fprintf(stderr, "[-] Error while setting realpath.\n");
        return -1;
    }

    return fuse_main(argc-2, argv, fs_get_ops(), 0);
}
