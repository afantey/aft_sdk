/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-01-26     rgw             first version
 */
#include "aft_sdk.h"
#include <tinycrypt/cbc_mode.h>

#define DBG_TAG "bsp.rtc"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

int sdk_crypto_encrypt_cbc(const uint8_t *in, int inlen, uint8_t *out, int *outlen, const uint8_t *iv, const uint8_t *key)
{
    uint8_t encrypt_buff[512+TC_AES_BLOCK_SIZE] = {0};
    struct tc_aes_key_sched_struct key_sched;
    int fill = 16 - (inlen) % 16;
    uint8_t fill_buff[512] = {0};
    int ret;
    int enc_len;

    if(in == NULL||out == NULL || iv == NULL || key == NULL || outlen == NULL)
        return 0;
    if(inlen > sizeof(fill_buff))
    {
        LOG_E("input len is too long, %d\n", inlen);
        return 0;
    }
    
    memcpy(fill_buff, in, inlen);
    // for (int i = 0; i < fill; i++) {
    //     fill_buff[inlen + i] = fill;
    // }
    enc_len = inlen+fill;
    (void)tc_aes128_set_encrypt_key(&key_sched, key);
    ret = tc_cbc_mode_encrypt(encrypt_buff, enc_len + TC_AES_BLOCK_SIZE, fill_buff, enc_len, iv, &key_sched);
    if (ret == 1) {
        memcpy(out, encrypt_buff + TC_AES_BLOCK_SIZE, enc_len);
        *outlen = enc_len;
    }

    return ret;
}

int sdk_crypto_decrypt_cbc(const uint8_t *in, int inlen, uint8_t *out, int *outlen, const uint8_t *iv, const uint8_t *key)
{
    uint8_t encrypt_data[512+TC_AES_BLOCK_SIZE] = {0};
    struct tc_aes_key_sched_struct key_sched;
    int ret;

    if(in == NULL||out == NULL || iv == NULL || key == NULL || outlen == NULL)
        return 0;
    if(inlen > sizeof(encrypt_data)-TC_AES_BLOCK_SIZE)
    {
        LOG_E("input len is too long, %d\n", inlen);
        return 0;
    }

    (void)tc_aes128_set_decrypt_key(&key_sched, key);

    // TinyCrypt CBC decryption assumes that the iv and the ciphertext are contiguous
    memcpy(encrypt_data, iv, TC_AES_BLOCK_SIZE);
    memcpy(encrypt_data + TC_AES_BLOCK_SIZE, in, inlen);

    ret = tc_cbc_mode_decrypt(out, inlen,
                                encrypt_data+TC_AES_BLOCK_SIZE, inlen, encrypt_data, &key_sched);
    if (ret == 1) {
        *outlen = inlen;
    }

    return ret;
}
