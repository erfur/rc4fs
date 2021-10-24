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

    ARGV_RC4_KEY = argv[argc-1];

    if (getcwd(buf, 1024)) {
	asprintf(&ARGV_REAL_PATH, "%s/%s", buf, argv[argc-2]);
    } else {
	return -1;
    }

    ret = fuse_main(argc-2, argv, &fs_ops, 0);
    
    free(ARGV_REAL_PATH);
    return ret;
}
