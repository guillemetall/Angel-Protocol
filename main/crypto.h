#ifndef CRYPTO_H
#define CRYPTO_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
void crypto_init(void);
bool crypto_set_key(const uint8_t *key);
void crypto_get_key(uint8_t *out);
int crypto_encrypt(uint8_t *data, uint16_t in_len, uint8_t *nonce);
int crypto_decrypt(uint8_t *data, uint16_t in_len, const uint8_t *nonce);
#endif
