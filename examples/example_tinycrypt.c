/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-01-25     rgw             first version
 */
#include "aft_sdk.h"
#include "sdk_crypto.h"

#define DBG_TAG "ex.cbc"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

uint8_t XinAo_IV[16] = {'C', 'E', 'T', 'C', '2', '0', '2', '3', 'J', 'A', 'E', 'I', 'O', 'T', 'R', '7'};
uint8_t XinAo_Key[16] = {'D', '2', '5', 'A', 'F', 'D', 'F', '0', '0', '7', '8', 'C', '7', '8', 'D', '2'};

//uint8_t in[] = {0x05, 0x18, 0x01, 0x19, 0x15, 0x03, 0x21, 0x4A, 0xD8, 0x12, 0x4D, 0xFB, 0x21, 0x09, 0x40, 0xE7, 0x07, 0x00, 0x00, 0x01, 0x11, 0x00, 0xE6, 0xBC, 0x94, 0xE7, 0xA4, 0xBA, 0xE6, 0x95, 0xB0, 0xE6, 0x8D, 0xAE, 0x54, 0x65, 0x78, 0x74, 0x00, 0x0E, 0x00, 0xE6, 0xB5, 0x8B, 0xE8, 0xAF, 0x95, 0xE7, 0x89, 0x88, 0x56, 0x31, 0x2E, 0x30, 0x00, };
uint8_t in[] = {0x01};
uint8_t out[128] = {0};
uint8_t out2[128] = {0};

int test_cbc_mode()
{
    int outlen = 0;
    int outlen2 = 0;
    int ret = sdk_crypto_encrypt_cbc(in, sizeof(in), out, &outlen, XinAo_IV, XinAo_Key);
    LOG_RAW("ret %d out %d:\n", ret, outlen);
    LOG_HEX(32, out, outlen);
    ret = sdk_crypto_decrypt_cbc(out, outlen, out2, &outlen2, XinAo_IV, XinAo_Key);
    LOG_RAW("ret2 %d out2 %d:\n", ret, outlen2);
    LOG_HEX(32, out2, outlen2);
    return 0;
}