/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-11-27     rgw             first version
 */
#include "aft_sdk.h"
#include "sdk_ringbuffer.h"

void sdk_ringbuffer_init(struct sdk_ringbuffer *rb, uint8_t *pool, int16_t size)
{
    rb->buf_ptr = pool;
    rb->bufsz = size;
    rb->get_index = 0;
    rb->put_index = 0;
    rb->is_full = false;
}

void sdk_ringbuffer_reset(struct sdk_ringbuffer *rb)
{
    rb->get_index = 0;
    rb->put_index = 0;
    rb->is_full = false;
}

size_t sdk_ringbuffer_put(struct sdk_ringbuffer *rb, const uint8_t *ptr, uint16_t length)
{
    int ret = 0;
    int sum = 0;
    for (int i = 0; i < length; i++)
    {
        ret = sdk_ringbuffer_putchar(rb, ptr[i]);
        if (ret)
            sum++;
        else
            break;
    }
    return sum;
}

size_t sdk_ringbuffer_put_force(struct sdk_ringbuffer *rb, const uint8_t *ptr, uint16_t length)
{
    for (int i = 0; i < length; i++)
    {
        sdk_ringbuffer_putchar_force(rb, ptr[i]);
    }
    return length;
}

size_t sdk_ringbuffer_putchar(struct sdk_ringbuffer *rb, const uint8_t ch)
{
    if (rb->is_full)
    {
        return 0;
    }

    rb->buf_ptr[rb->put_index] = ch;
    rb->put_index = (rb->put_index + 1) % rb->bufsz;

    if (rb->put_index == rb->get_index)
    {
        rb->is_full = true;
    }

    return 1;
}

size_t sdk_ringbuffer_putchar_force(struct sdk_ringbuffer *rb, const uint8_t ch)
{
    if (rb == NULL)
    {
        return 0;
    }

    // 如果队列已满，更新get_index以覆盖旧的数据
    if (rb->is_full)
    {
        rb->get_index = (rb->get_index + 1) % rb->bufsz;
    }

    rb->buf_ptr[rb->put_index] = ch;
    rb->put_index = (rb->put_index + 1) % rb->bufsz;

    // 如果put_index和get_index相等，那么队列是满的
    if (rb->put_index == rb->get_index)
    {
        rb->is_full = true;
    }

    return 1;
}

size_t sdk_ringbuffer_get(struct sdk_ringbuffer *rb, uint8_t *ptr, uint16_t length)
{
    int ret = 0;
    int sum = 0;
    for (int i = 0; i < length; i++)
    {
        ret = sdk_ringbuffer_getchar(rb, &ptr[i]);
        if (ret)
            sum++;
        else
            break;
    }
    return sum;
}

size_t sdk_ringbuffer_peak(struct sdk_ringbuffer *rb, uint8_t **ptr)
{
    if (rb->get_index == rb->put_index)
        return 0;

    *ptr = &rb->buf_ptr[rb->get_index];

    return 1;
}

size_t sdk_ringbuffer_getchar(struct sdk_ringbuffer *rb, uint8_t *ch)
{
    // check rb is empty
    if (!rb->is_full && rb->get_index == rb->put_index)
    {
        return 0;
    }

    *ch = rb->buf_ptr[rb->get_index];
    rb->get_index = (rb->get_index + 1) % rb->bufsz;

    rb->is_full = false;

    return 1;
}

size_t sdk_ringbuffer_data_len(struct sdk_ringbuffer *rb)
{
    if (rb->is_full)
    {
        return rb->bufsz;
    }
    else if (rb->put_index >= rb->get_index)
    {
        return rb->put_index - rb->get_index;
    }
    else
    {
        return rb->bufsz - (rb->get_index - rb->put_index);
    }
}
