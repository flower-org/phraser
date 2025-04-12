#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pbkdf2-sha256.h"

#define HASH_SIZE 32
#define BLOCK_SIZE 32 // SHA-256 produces a 32-byte hash

typedef struct {
    unsigned char V[HASH_SIZE]; // Internal state
    unsigned char K[HASH_SIZE]; // Key
    size_t reseed_counter;
} Hash_DRBG;

void hash_update(unsigned char *hash, const unsigned char *data, size_t data_len) {
	sha2_context ctx;
  sha2_starts(&ctx, 0);
  sha2_update(&ctx, hash, HASH_SIZE);
  sha2_update(&ctx, data, data_len);
  sha2_finish(&ctx, hash);
}

void hash_drbg_init(Hash_DRBG *drbg, const unsigned char *seed, size_t seed_len) {
    // Initialize K and V
    memset(drbg->K, 0, HASH_SIZE);
    memset(drbg->V, 1, HASH_SIZE);
    drbg->reseed_counter = 1;

    // Update K and V with the seed
    hash_update(drbg->K, seed, seed_len);
    hash_update(drbg->V, drbg->K, HASH_SIZE);
}

void hash_drbg_init(Hash_DRBG *drbg, uint32_t seed) {
  // Convert uint32_t seed to byte array
  unsigned char seed_bytes[sizeof(uint32_t)];
  memcpy(seed_bytes, &seed, sizeof(uint32_t));

  hash_drbg_init(drbg, seed_bytes, sizeof(seed_bytes));
}

void hash_drbg_reseed(Hash_DRBG *drbg, const unsigned char *additional_input, size_t input_len) {
    // Update K and V with additional input
    hash_update(drbg->K, additional_input, input_len);
    hash_update(drbg->V, drbg->K, HASH_SIZE);
    drbg->reseed_counter++;
}

void hash_drbg_generate(Hash_DRBG *drbg, unsigned char *output, size_t output_len) {
    size_t generated = 0;

    while (generated < output_len) {
        // Generate new V
        hash_update(drbg->V, NULL, 0);
        size_t to_copy = (output_len - generated < BLOCK_SIZE) ? output_len - generated : BLOCK_SIZE;
        memcpy(output + generated, drbg->V, to_copy);
        generated += to_copy;
    }
}

uint32_t hash_drbg_generate_uint32(Hash_DRBG *drbg) {
  uint8_t output[4]; // Buffer for 4 bytes (32 bits)
  hash_drbg_generate(drbg, output, sizeof(output));
  return (output[0] << 24) | (output[1] << 16) | (output[2] << 8) | output[3];
}

// ------------------------------------------

Hash_DRBG common_drbg;

void drbg_randomSeed(uint32_t seed) {
  hash_drbg_init(&common_drbg, seed);
}

void drbg_generate(unsigned char *output, size_t output_len) {
  hash_drbg_generate(&common_drbg, output, output_len);
}

uint32_t drbg_rand() {
  return hash_drbg_generate_uint32(&common_drbg);
}

uint32_t drbg_random(uint32_t howbig) {
  if (howbig == 0) {
      return 0 ;
  }

  return drbg_rand() % howbig;
}

uint32_t drbg_random(uint32_t howsmall, uint32_t howbig) {
  if (howsmall >= howbig) {
      return howsmall;
  }

  uint32_t diff = howbig - howsmall;

  return drbg_random(diff) + howsmall;
}



/*
int main() {
    Hash_DRBG drbg;
    unsigned char seed[] = "my_secret_seed";
    unsigned char output[64]; // Buffer for random output

    // Initialize the DRBG
    hash_drbg_init(&drbg, seed, strlen((char *)seed));

    // Generate random bytes
    hash_drbg_generate(&drbg, output, sizeof(output));

    // Print the generated random bytes
    printf("Generated random bytes:\n");
    for (size_t i = 0; i < sizeof(output); i++) {
        printf("%02x", output[i]);
    }
    printf("\n");

    return 0;
}
*/