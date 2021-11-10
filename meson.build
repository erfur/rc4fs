project('rc4fs', 'c')
fuse = dependency('fuse3')
add_project_arguments('-DFS_CRYPTO_ENABLED=1', language : 'c')
executable('rc4fs',
    'src/main.c',
    'src/fs.c',
    'src/rc4plus.c', 
    dependencies: fuse)
