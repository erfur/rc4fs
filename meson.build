project('rc4fs', 'c')
fuse = dependency('fuse3')
executable('rc4fs', 'src/main.c', 'src/fs.c', 'src/rc4.c', dependencies: fuse)
