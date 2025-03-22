#ifndef _PBKDF_SHA256_H_
#define _PBKDF_SHA256_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void PKCS5_PBKDF2_SHA256_HMAC(unsigned char *password, size_t plen,
    unsigned char *salt, size_t slen,
    const unsigned long iteration_count, const unsigned long key_length,
    unsigned char *output);


#endif // _PBKDF_SHA256_H_
