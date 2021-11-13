
██████╗░░█████╗░░░██╗██╗███████╗░██████╗
██╔══██╗██╔══██╗░██╔╝██║██╔════╝██╔════╝
██████╔╝██║░░╚═╝██╔╝░██║█████╗░░╚█████╗░
██╔══██╗██║░░██╗███████║██╔══╝░░░╚═══██╗
██║░░██║╚█████╔╝╚════██║██║░░░░░██████╔╝
╚═╝░░╚═╝░╚════╝░░░░░░╚═╝╚═╝░░░░░╚═════╝░

--[ 0 - RC4FS ]

This is a FUSE filesystem implementation that uses rc4 to encrypt file contents.
There will be no plain data written to disk (aside from the file name of
course).


--[ 1 - Implementation ]

A FUSE filesystem is a user-mounted (no root required) virtual filesystem. File
functions such as open, read, write, stat are implemented as custom functions in
userspace. This allows creating a filesystem from virtually any input source.

These functions are required for basic functionality:
    - open
    - release
    - read
    - write
    - readdir
    - getattr
    - mknod
    - unlink

A basic tutorial can be found in [1]. A passthrough implementation can be found
in libfuse documentation [2].


--[ 1.1 - Encryption Layer ]

rc4fs implements rc4+ encryption scheme to keep the data on the disk encrypted.
Since rc4 is a stream cipher and disk operations require random access, a
direct approach fails on both performance and security. To compensate these
shortcomings, at each sector (default 4KB) rc4fs resets the encryption stream
and uses a random IV. These enchancements result in both reasonable performance
and security.

The rc4+ key is used for all the files in a folder. A new IV key is generated
from /dev/urandom for each new file and this key is placed at the end of the
file. A random IV for each sector is then derived from this initial key.


--[ 2 - How to Build ]

rc4fs can be build using Meson/Ninja:

    $ meson build
    $ cd build
    $ ninja


--[ 3 - Usage ]

To mount an encrypted folder, run the program with the following args:

    $ rc4fs [FUSE and mount options] mountpoint encryptedfolder rc4key


--[ A - References ]

[1] https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/html/index.html
[2] http://libfuse.github.io/doxygen/example_2passthrough__fh_8c.html
