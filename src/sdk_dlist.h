/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-08-13     rgw             first version
 */

#ifndef __SDK_DLIST_H
#define __SDK_DLIST_H

struct sdk_dlist
{
    struct sdk_dlist *prev;
    struct sdk_dlist *next;
    void *data;  // 指针，用于保存您的实际数据
};

#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))
#endif

#define sdk_dlist_entry(ptr, type, member)       container_of(ptr, type, member)
#define sdk_dlist_for_each(pos, head)  for(pos = (head)->next; pos != (head); pos = (pos)->next)
#define sdk_dlist_for_each_safe(pos, n, head) for(pos = (head)->next, n = (pos)->next; pos != (head); pos = (n), n = (pos)->next)

void sdk_dlist_init(struct sdk_dlist *node);
void sdk_dlist_insert(struct sdk_dlist *new, struct sdk_dlist *prev, struct sdk_dlist *next);
void sdk_dlist_insert_after(struct sdk_dlist *head, struct sdk_dlist *new);
void sdk_dlist_insert_before(struct sdk_dlist *head, struct sdk_dlist *new);
void sdk_dlist_remove(struct sdk_dlist *node);
int sdk_dlist_is_empty(struct sdk_dlist *head);

#endif /* __SDK_DLIST_H */
