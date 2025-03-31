#ifndef _PBKDF_SHA256_H_
#define _PBKDF_SHA256_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	unsigned long total[2];	/*!< number of bytes processed  */
	unsigned long state[8];	/*!< intermediate digest state  */
	unsigned char buffer[64];	/*!< data block being processed */

	unsigned char ipad[64];	/*!< HMAC: inner padding        */
	unsigned char opad[64];	/*!< HMAC: outer padding        */
	int is224;		/*!< 0 => SHA-256, else SHA-224 */
} sha2_context;


void PKCS5_PBKDF2_SHA256_HMAC(unsigned char *password, size_t plen,
    unsigned char *salt, size_t slen,
    const unsigned long iteration_count, const unsigned long key_length,
    unsigned char *output);

void sha2_starts(sha2_context *ctx, int is224);
void sha2_update(sha2_context *ctx, const unsigned char *input, size_t ilen);
void sha2_finish(sha2_context *ctx, unsigned char output[32]);


#endif // _PBKDF_SHA256_H_
