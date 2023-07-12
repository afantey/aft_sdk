/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-06     rgw          first version
 */

#include "aft_sdk.h"
#include "sdk_can.h"
#include "board.h"
#include "SEGGER_RTT.h"

#define DBG_TAG "ex.can"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

void sdk_hw_console_output(const char *str)
{
    SEGGER_RTT_WriteString(0, str);
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    SEGGER_RTT_PutChar(0, (uint8_t)ch);
    return ch;
}

void example_can_init(void)
{
    sdk_can_open(&can_0);
}

void example_can_read(void)
{
    int32_t rl = 0;
    sdk_can_msg_t msg = {0};
    rl = sdk_can_read(&can_0, &msg);
    if (rl == 1)
    {
        printf("can_id 0x%08x, ide %d, rtr %d, dlc %d\n", msg.canid, msg.ide, msg.rtr, msg.dlc);
        LOG_HEX(8, msg.data, msg.dlc);
    }
}

void example_can_send(void)
{
    uint8_t test[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    sdk_can_msg_t msg = { 0 };
    msg.canid = 0x321;
    msg.dlc = 8;
    msg.ide = SDK_CAN_IDE_EXTENDED;
    msg.rtr = SDK_CAN_RTR_DATA;
    memcpy(msg.data, test, 8);
    sdk_can_write(&can_0, &msg);
}
extern volatile uint32_t systicks;
int main(void)
{
    int cnt = 0;
    board_init();
    example_can_init();
    while(1)
    {
        cnt = systicks;
        if(cnt % 10 == 0)
        {
            example_can_send();
        }
        example_can_read();
    }
}
