/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-08-13     rgw             first version
 */
#include "sdk_dlist.h"

/**
 * @brief initialize a list
 * 
 * @param[in] node the list to be initialized
 */
void sdk_dlist_init(struct sdk_dlist *node)
{
    node->next = node->prev = node;
}

/**
 * @brief insert a node
 * 
 * @param[in] new 
 * @param[in] prev 
 * @param[in] next 
 */
void sdk_dlist_insert(struct sdk_dlist *new, struct sdk_dlist *prev, struct sdk_dlist *next)
{
    // 处理new的前面和后面
    prev->next = new;
    next->prev = new;
    new->prev = prev;
    new->next = next;
}

/**
 * @brief insert a node after head
 * 
 * @param[in] head 
 * @param[in] new 
 */
void sdk_dlist_insert_after(struct sdk_dlist *head, struct sdk_dlist *new)
{
    sdk_dlist_insert(new, head, head->next);
}

/**
 * @brief insert a node before head
 * 
 * @param[in] head 
 * @param[in] new 
 */
void sdk_dlist_insert_before(struct sdk_dlist *head, struct sdk_dlist *new)
{
    sdk_dlist_insert(new, head->prev, head);
}

/**
 * @brief remove a node
 * 
 * @param[in] node 
 */
void sdk_dlist_remove(struct sdk_dlist *node)
{
    struct sdk_dlist *prev = node->prev;
    struct sdk_dlist *next = node->next;

    prev->next = next;
    next->prev = prev;

    node->next = node->prev = node;
}

/**
 * @brief check whether the list is empty
 * 
 * @param[in] head  the head of the list
 * @return int 1: empty, 0: not empty
 */
int sdk_dlist_is_empty(struct sdk_dlist *head)
{
    return head->next == head ? 1 : 0;
}

/*
void print_list(struct sdk_dlist *head)
{
    struct sdk_dlist *pos = NULL;
    list_for_each(pos, head)
    {
        printf("%d ", *(int*)(pos->data));
    }
}

int main()
{
    struct sdk_dlist mylist;
    sdk_dlist_init(&mylist);

    int data[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++)
    {
        struct sdk_dlist *new = (struct sdk_dlist *)malloc(sizeof(struct sdk_dlist));
        new->data = &data[i];
        sdk_dlist_insert_after(&mylist, new);
    }

    print_list(&mylist);
    return 0;
}

Output: 5 4 3 2 1
*/


