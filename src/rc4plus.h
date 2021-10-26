#ifndef RC4FS_RC4PLUS_H
#define RC4FS_RC4PLUS_H

#include <string.h>
#include <stdio.h>

#define RC4PLUS_IV_SIZE 16

extern unsigned char RC4PLUS_STATE[256];
extern unsigned char RC4PLUS_IV[RC4PLUS_IV_SIZE];

int rc4plus_set_key(const char *const, const char *const);
int rc4plus(char *, size_t);

#endif
