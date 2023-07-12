/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#include "board.h"
#include "SEGGER_RTT.h"
#include "pt.h"

#define DBG_TAG "ex.pt"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

static struct pt pt1, pt2;
static struct pt pt3, pt4;

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

static int protothread1_flag, protothread2_flag;
static int protothread3_flag, protothread4_flag;
void example_pt34_run(void);
static int
protothread1(struct pt *pt)
{
  /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
  PT_BEGIN(pt);

  /* We loop forever here. */
  while(1) {
    /* Wait until the other protothread has set its flag. */
    PT_WAIT_UNTIL(pt, protothread2_flag != 0);
    printf("Protothread 1 running\n");
    protothread3(&pt3);
    protothread4(&pt4);
    protothread4(&pt4);
    /* We then reset the other protothread's flag, and set our own
       flag so that the other protothread can run. */
    protothread2_flag = 0;
    protothread1_flag = 1;

    /* And we loop. */
  }

  /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
  PT_END(pt);
}

/**
 * The second protothread function. This is almost the same as the
 * first one.
 */
static int
protothread2(struct pt *pt)
{
  PT_BEGIN(pt);

  while(1) {
    /* Let the other protothread run. */
    protothread2_flag = 1;

    /* Wait until the other protothread has set its flag. */
    PT_WAIT_UNTIL(pt, protothread1_flag != 0);
    printf("Protothread 2 running\n");
    
    /* We then reset the other protothread's flag. */
    protothread1_flag = 0;

    /* And we loop. */
  }
  PT_END(pt);
}

static int
protothread3(struct pt *pt)
{
  /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
  PT_BEGIN(pt);

  /* We loop forever here. */
  while(1) {
    /* Wait until the other protothread has set its flag. */
    PT_WAIT_UNTIL(pt, protothread4_flag != 0);
    printf("Protothread 3 running\n");

    /* We then reset the other protothread's flag, and set our own
       flag so that the other protothread can run. */
    protothread4_flag = 0;
    protothread3_flag = 1;

    /* And we loop. */
  }

  /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
  PT_END(pt);
}

/**
 * The second protothread function. This is almost the same as the
 * first one.
 */
static int
protothread4(struct pt *pt)
{
  PT_BEGIN(pt);

  while(1) {
    /* Let the other protothread run. */
    protothread4_flag = 1;

    /* Wait until the other protothread has set its flag. */
    PT_WAIT_UNTIL(pt, protothread3_flag != 0);
    printf("Protothread 4 running\n");
    
    /* We then reset the other protothread's flag. */
    protothread3_flag = 0;

    /* And we loop. */
  }
  PT_END(pt);
}

void example_pt_init(void)
{
    PT_INIT(&pt1);
    PT_INIT(&pt2);
}
void example_pt34_init(void)
{
    PT_INIT(&pt3);
    PT_INIT(&pt4);
}

void example_pt_run(void)
{
    protothread1(&pt1);
    protothread2(&pt2);
}

void example_pt34_run(void)
{
    protothread3(&pt3);
    protothread4(&pt4);
}



int main(void)
{
    board_init();
    example_pt_init();
    example_pt34_init();
    while(1)
    {
        example_pt_run();

    }
}