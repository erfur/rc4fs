#include "rc4plus.h"

unsigned char RC4PLUS_STATE[256];
unsigned char RC4PLUS_IV[RC4PLUS_IV_SIZE];

unsigned char get_iv_char(unsigned char index)
{
    if ( index < (128 - RC4PLUS_IV_SIZE) ) {
        return 0;
    } else if ( index < 128 ) {
        return RC4PLUS_IV[128 - 1 - index];
    } else if ( index < 128 + RC4PLUS_IV_SIZE ) {
        return RC4PLUS_IV[index - 128];
    } else {
        return 0;
    }
}

int rc4plus_set_key(const char *const key, const char *const iv)
{
    if ( key == NULL || iv == NULL ) {
        //NULL ptr error
        return -1;
    }

    size_t key_length = strnlen(key, 256);
    //Check for the zero length keys and keys exceeding 256 byte
    if ( key_length == 256 && key[key_length] != 0 ) {
        printf("Warning, long key! First 256 byte of the key will be taken.\n");
    } else if (  !key_length ) {
        //Key length error
        return -1;
    }

    //IV length check
    if ( strnlen(iv, RC4PLUS_IV_SIZE) < RC4PLUS_IV_SIZE ) {
        printf("Please use %i byte iv.\n", RC4PLUS_IV_SIZE);
        return -1;
    }
    //Copy the iv
    memcpy(RC4PLUS_IV, iv, RC4PLUS_IV_SIZE);

    //Init RC4PLUS_STATE
    for ( size_t i = 256; i-- ; )
        RC4PLUS_STATE[i] = i;
    unsigned char j = 0;
    unsigned char temp;
    //Key loading
    for ( size_t i = 0 ; i < 256 ; ++i ) {
        j += RC4PLUS_STATE[i] + key[i % key_length];
        //swap part
        temp = RC4PLUS_STATE[i];
        RC4PLUS_STATE[i] = RC4PLUS_STATE[j];
        RC4PLUS_STATE[j] = temp;
    }
    //IV loading
    for ( size_t i = 128 ; i-- ; ) {
        j += RC4PLUS_STATE[i];
        j ^= key[i % key_length] + get_iv_char(i);
        temp = RC4PLUS_STATE[i];
        RC4PLUS_STATE[i] = RC4PLUS_STATE[j];
        RC4PLUS_STATE[j] = temp;
    }
    for ( size_t i = 128 ; i < 256 ; ++i ) {
        j += RC4PLUS_STATE[i];
        j ^= key[i % key_length] + get_iv_char(i);
        temp = RC4PLUS_STATE[i];
        RC4PLUS_STATE[i] = RC4PLUS_STATE[j];
        RC4PLUS_STATE[j] = temp;
    }
    //Zig-Zag Scrambling
    size_t i = 0;
    for ( size_t y = 0 ; y < 256 ; ++y ) {
        if ( y % 2 == 0 ) {
            i = y/2;
        } else {
            i = 128 - ((y+1)/2);
        }
        j += RC4PLUS_STATE[i] + key[i % key_length];
        temp = RC4PLUS_STATE[i];
        RC4PLUS_STATE[i] = RC4PLUS_STATE[j];
        RC4PLUS_STATE[j] = temp;
    }
    return 0;
}

int rc4plus(char *data, size_t size)
{
    if ( data == NULL ) {
        //NULL ptr error
        return -1;
    }

    //Getting state matrix
    unsigned char state[256]; 
    memcpy(state, RC4PLUS_STATE, 256);

    //Part of random generation algorithm
    //Algorithm encrypts the given size of the data
    unsigned char i = 0;
    unsigned char j = 0;
    unsigned char t1, t2, t3;
    unsigned char temp;
    for ( size_t x = 0 ; x < size ; ++x ) {
        i += 1;
        j += state[i];
        temp = state[i];
        state[i] = state[j];
        state[j] = temp;
        t1 = state[i] + state[j];
        t2 = (state[((i >> 3) ^ (j << 5)) % 256] + state[((i << 5) ^ ( j >> 3)) % 256]);
        t2 ^= 0xAA;
        t3 = j + state[j];
        data[x] ^= (state[t1] + state[t2]) ^ state[t3];
    }

    return 0;
}
