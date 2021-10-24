
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
    - read
    - write
    - readdir
    - getattr

A basic tutorial can be found in [1].


--[ 2 - Usage ]

To mount an encrypted folder, run the program with the following args:

    $ rc4fs encrypted_folder mountpoint


--[ 3 - References ]

[1] https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/html/index.html
