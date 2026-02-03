/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-09-18     rgw             first version
 */

#ifndef __EVENT_SCHEDULER_H__
#define __EVENT_SCHEDULER_H__

typedef void (*event_handler_t)(void *p_event_data, uint16_t event_data_size);

struct event_node
{
    // struct sdk_dlist event_list;
    event_handler_t event_handler;
    void *p_event_data;
    uint16_t event_data_size;
};

uint32_t event_sched_init(struct event_node *event_head);
uint32_t event_node_put(struct event_node *event_head, struct event_node *event_node, void *p_event_data,
                        uint16_t event_data_size,
                        event_handler_t handler);
void event_sched_execute(struct event_node *event_head);

#endif
