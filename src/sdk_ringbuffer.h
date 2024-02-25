/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-11-27     rgw             first version
 */

#ifndef __SDK_RINGBUFFER_H__
#define __SDK_RINGBUFFER_H__

struct sdk_ringbuffer {
    uint8_t *buf_ptr;
    uint32_t bufsz;
    uint16_t get_index;
    uint16_t put_index;
    bool is_full;
};

void sdk_ringbuffer_init(struct sdk_ringbuffer *rb, uint8_t *pool, int16_t size);
void sdk_ringbuffer_reset(struct sdk_ringbuffer *rb);
size_t sdk_ringbuffer_put(struct sdk_ringbuffer *rb, const uint8_t *ptr, uint16_t length);
size_t sdk_ringbuffer_put_force(struct sdk_ringbuffer *rb, const uint8_t *ptr, uint16_t length);
size_t sdk_ringbuffer_putchar(struct sdk_ringbuffer *rb, const uint8_t ch);
size_t sdk_ringbuffer_putchar_force(struct sdk_ringbuffer *rb, const uint8_t ch);
size_t sdk_ringbuffer_get(struct sdk_ringbuffer *rb, uint8_t *ptr, uint16_t length);
size_t sdk_ringbuffer_peak(struct sdk_ringbuffer *rb, uint8_t **ptr);
size_t sdk_ringbuffer_getchar(struct sdk_ringbuffer *rb, uint8_t *ch);
size_t sdk_ringbuffer_data_len(struct sdk_ringbuffer *rb);


#endif /* __SDK_RINGBUFFER_H__ */
