/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-09-18     rgw             first version
 */
#include "aft_sdk.h"
#include "sdk_board.h"
#include "sdk_dlist.h"
#include "event_scheduler.h"

#define CRITICAL_REGION_ENTER sdk_os_enter_critical
#define CRITICAL_REGION_EXIT  sdk_os_exit_critical


uint32_t event_sched_init(struct event_node *event_head)
{
    sdk_dlist_init(&event_head->event_list);

    return SDK_OK;
}

#define INIT_EVENT_NODE(event, data, sz, handler) \
    do                                            \
    {                                             \
        sdk_dlist_init(&((event)->event_list));   \
        (event)->event_handler = (handler);       \
        (event)->p_event_data = (data);           \
        (event)->event_data_size = (sz);          \
    } while (0)

uint32_t event_node_put(struct event_node *event_head, struct event_node *event_node, void *p_event_data,
                        uint16_t event_data_size,
                        event_handler_t handler)
{
    INIT_EVENT_NODE(event_node, p_event_data, event_data_size, handler);
    sdk_dlist_insert_after(&event_head->event_list, &event_node->event_list);

    return 0;
}

void event_sched_execute(struct event_node *event_head)
{
    struct sdk_dlist *list_pos, *list_tmp;

    if(!sdk_dlist_is_empty(&event_head->event_list))
    {
        sdk_dlist_for_each_safe(list_pos, list_tmp, &event_head->event_list)
        {
            sdk_dlist_remove(list_pos);
            struct event_node *event_node = sdk_dlist_entry(list_pos, struct event_node, event_list);
            event_node->event_handler(event_node->p_event_data, event_node->event_data_size);
        }
    }
}
