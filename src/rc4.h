#ifndef RC4FS_RC4_H
#define RC4FS_RC4_H

extern unsigned char RC4_STATE[256];

int rc4_set_key(const char *const);
int rc4(char *, int);

#endif
