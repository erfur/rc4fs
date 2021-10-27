#ifndef RC4FS_RC4PLUS_H
#define RC4FS_RC4PLUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RC4PLUS_IV_SIZE 16

typedef struct _rc4struct {
    unsigned char state[256];
    int (*enc_dec_func)(char *, size_t, off_t, const unsigned char *const);
} rc4struct;

rc4struct *rc4plus_init(const char *const, const char *const);
int rc4plus_destroy(rc4struct *);
int rc4plus_encdec(char *, size_t, off_t, const unsigned char *const);

#endif
