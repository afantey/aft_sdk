/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-01-26     rgw             first version
 */
#ifndef __SDK_CRYPTO_H__
#define __SDK_CRYPTO_H__

int sdk_crypto_encrypt_cbc(const uint8_t *in, int inlen, uint8_t *out, int *outlen, const uint8_t *iv, const uint8_t *key);
int sdk_crypto_decrypt_cbc(const uint8_t *in, int inlen, uint8_t *out, const uint8_t *iv, const uint8_t *key);

#endif //__SDK_CRYPTO_H__
