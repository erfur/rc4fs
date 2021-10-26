#include "rc4.h"

unsigned char RC4_STATE[256];

int rc4_set_key(const char *const key)
{
    if ( key == NULL ) {
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

    //Init RC4_STATE
    for ( size_t i = 256; i-- ; )
        RC4_STATE[i] = i;

    size_t j = 0;
    unsigned char temp;
    for ( size_t i = 0 ; i < 256 ; ++i ) {
        j = (j + RC4_STATE[i] + key[i % key_length]) % 256;
        //swap part
        temp = RC4_STATE[i];
        RC4_STATE[i] = RC4_STATE[j];
        RC4_STATE[j] = temp;
    }

    return 0;
}

int rc4(char *data, size_t size)
{
    if ( data == NULL ) {
        //NULL ptr error
        return -1;
    }

    //Getting state matrix
    unsigned char state[256]; 
    memcpy(state, RC4_STATE, 256);

    //Part of random generation algorithm
    //Algorithm encrypts the given size of the data
    size_t i = 0;
    size_t j = 0;
    char temp;
    for ( size_t x = 0 ; x < size ; ++x ) {
        i = ( i + 1 ) % 256;
        j = ( j + state[i] ) % 256;
        temp = state[i];
        state[i] = state[j];
        state[j] = temp;
        data[x] ^= state[(state[i] + state[j]) % 256];
    }

    return 0;
}
