#include "rc4plus.h"

unsigned char get_iv_char(const char *const iv, unsigned char index)
{
    if ( index < (128 - RC4PLUS_IV_SIZE) ) {
        return 0;
    } else if ( index < 128 ) {
        return iv[128 - 1 - index];
    } else if ( index < 128 + RC4PLUS_IV_SIZE ) {
        return iv[index - 128];
    } else {
        return 0;
    }
}

int rc4plus_destroy(rc4struct *rcstruct)
{
    if ( rcstruct == NULL ) {
        //NULL ptr error
        return -1;
    }
    memset(rcstruct->state, 0, sizeof(rcstruct->state));
    free(rcstruct);
    return 0;
}

rc4struct *rc4plus_init(const char *const key, const char *const iv)
{
    if ( key == NULL || iv == NULL ) {
        //NULL ptr error
        return NULL;
    }

    size_t key_length = strnlen(key, 256);
    //Check for the zero length keys and keys exceeding 256 byte
    if ( key_length == 256 && key[key_length] != 0 ) {
        printf("Warning, long key! First 256 byte of the key will be taken.\n");
    } else if (  !key_length ) {
        //Key length error
        return NULL;
    }

    //IV length check
    if ( strnlen(iv, RC4PLUS_IV_SIZE) < RC4PLUS_IV_SIZE ) {
        printf("Please use %i byte iv.\n", RC4PLUS_IV_SIZE);
        return NULL;
    }

    rc4struct *ret = (rc4struct *)malloc(sizeof(rc4struct));
    if ( ret == NULL ) {
        //Memory allocation error
        return NULL;
    }
    //Setting the rc4+'s enc-dec function
    ret->enc_dec_func = rc4plus_encdec;

    //Init the state
    for ( size_t i = 256; i-- ; )
        ret->state[i] = i;
    unsigned char j = 0;
    unsigned char temp;
    //Key loading
    for ( size_t i = 0 ; i < 256 ; ++i ) {
        j += ret->state[i] + key[i % key_length];
        //swap part
        temp = ret->state[i];
        ret->state[i] = ret->state[j];
        ret->state[j] = temp;
    }
    //IV loading
    for ( size_t i = 128 ; i-- ; ) {
        j += ret->state[i];
        j ^= key[i % key_length] + get_iv_char(iv, i);
        temp = ret->state[i];
        ret->state[i] = ret->state[j];
        ret->state[j] = temp;
    }
    for ( size_t i = 128 ; i < 256 ; ++i ) {
        j += ret->state[i];
        j ^= key[i % key_length] + get_iv_char(iv, i);
        temp = ret->state[i];
        ret->state[i] = ret->state[j];
        ret->state[j] = temp;
    }
    //Zig-Zag Scrambling
    size_t i = 0;
    for ( size_t y = 0 ; y < 256 ; ++y ) {
        if ( y % 2 == 0 ) {
            i = y/2;
        } else {
            i = 128 - ((y+1)/2);
        }
        j += ret->state[i] + key[i % key_length];
        temp = ret->state[i];
        ret->state[i] = ret->state[j];
        ret->state[j] = temp;
    }
    return ret;
}

int rc4plus_encdec(char *data, size_t size, off_t offset, const unsigned char *const state)
{
    if ( data == NULL || state == NULL ) {
        //NULL ptr error
        return -1;
    }

    //Copying state matrix
    unsigned char tmp_state[256]; 
    memcpy(tmp_state, state, 256);

    //Part of random generation algorithm
    //Algorithm encrypts the given size of the data
    unsigned char i = 0;
    unsigned char j = 0;
    unsigned char t1, t2, t3;
    unsigned char temp;
    //offset part
    for ( size_t x = 0 ; x < offset ; ++x ) {
        i += 1;
        j += tmp_state[i];
        temp = tmp_state[i];
        tmp_state[i] = tmp_state[j];
        tmp_state[j] = temp;
        t1 = tmp_state[i] + tmp_state[j];
        t2 = (tmp_state[((i >> 3) ^ (j << 5)) % 256] + tmp_state[((i << 5) ^ ( j >> 3)) % 256]);
        t2 ^= 0xAA;
        t3 = j + tmp_state[j];
    }
    //enc part
    for ( size_t x = 0 ; x < size ; ++x ) {
        i += 1;
        j += tmp_state[i];
        temp = tmp_state[i];
        tmp_state[i] = tmp_state[j];
        tmp_state[j] = temp;
        t1 = tmp_state[i] + tmp_state[j];
        t2 = (tmp_state[((i >> 3) ^ (j << 5)) % 256] + tmp_state[((i << 5) ^ ( j >> 3)) % 256]);
        t2 ^= 0xAA;
        t3 = j + tmp_state[j];
        data[x] ^= (tmp_state[t1] + tmp_state[t2]) ^ tmp_state[t3];
    }
    return 0;
}
