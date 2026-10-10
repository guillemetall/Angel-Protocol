#include "crypto.h"
#include "esp_random.h"
#include "mbedtls/gcm.h"
#include <string.h>
static uint8_t s_key[AES_KEY_LEN];
static bool s_key_set = false;
void crypto_init(void) {
    for (int i = 0; i < AES_KEY_LEN; i++) s_key[i] = (uint8_t)(i * 7 + 3);
    s_key_set = true;
}
bool crypto_set_key(const uint8_t *key) { memcpy(s_key, key, AES_KEY_LEN); s_key_set = true; return true; }
void crypto_get_key(uint8_t *out) { memcpy(out, s_key, AES_KEY_LEN); }
int crypto_encrypt(uint8_t *data, uint16_t in_len, uint8_t *nonce) {
    if (!s_key_set) return -1;
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);
    mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, s_key, AES_KEY_LEN * 8);
    esp_fill_random(nonce, AES_NONCE_LEN);
    uint8_t tag[AES_TAG_LEN];
    int ret = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, in_len,
        nonce, AES_NONCE_LEN, NULL, 0, data, data, AES_TAG_LEN, tag);
    mbedtls_gcm_free(&ctx);
    if (ret != 0) return -1;
    memcpy(data + in_len, tag, AES_TAG_LEN);
    return in_len + AES_TAG_LEN;
}
int crypto_decrypt(uint8_t *data, uint16_t in_len, const uint8_t *nonce) {
    if (!s_key_set || in_len < AES_TAG_LEN) return -1;
    uint16_t ct = in_len - AES_TAG_LEN;
    uint8_t tag[AES_TAG_LEN];
    memcpy(tag, data + ct, AES_TAG_LEN);
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);
    mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, s_key, AES_KEY_LEN * 8);
    int ret = mbedtls_gcm_auth_decrypt(&ctx, ct, nonce, AES_NONCE_LEN, NULL, 0, tag, AES_TAG_LEN, data, data);
    mbedtls_gcm_free(&ctx);
    return (ret == 0) ? ct : -1;
}
