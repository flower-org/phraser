#pragma once

#include <Thumby.h>

void drbg_randomSeed(uint32_t seed);
void drbg_generate(unsigned char *output, size_t output_len);
uint32_t drbg_rand();
uint32_t drbg_random(uint32_t howbig);
uint32_t drbg_random(uint32_t howsmall, uint32_t howbig);
