#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    element_t *cur, *next;
    list_for_each_entry_safe (cur, next, head, list) {
        q_release_element(cur);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *element = malloc(sizeof(element_t));
    size_t len = strlen(s);
    element->value = malloc(len + 1);
    strncpy(element->value, s, len + 1);
    list_add(&element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *element = malloc(sizeof(element_t));
    size_t len = strlen(s);
    element->value = malloc(len + 1);
    strncpy(element->value, s, len + 1);
    list_add_tail(&element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *eleptr = list_first_entry(head, element_t, list);
    if (sp)
        strncpy(sp, eleptr->value, bufsize);
    list_del_init(&eleptr->list);
    return eleptr;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *eleptr = list_last_entry(head, element_t, list);
    if (sp)
        strncpy(sp, eleptr->value, bufsize);
    list_del_init(&eleptr->list);
    return eleptr;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    struct list_head *slow;
    struct list_head *fast = head->next;
    list_for_each (slow, head) {
        if (fast->next == head || fast->next->next == head) {
            list_del_init(slow);
            q_release_element(list_entry(slow, element_t, list));
            return true;
        }
        fast = fast->next->next;
    }
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    q_sort(head, false);
    element_t *cur, *next;
    list_for_each_entry_safe (cur, next, head, list) {
        size_t len = strlen(cur->value) + 1;
        char *target = malloc(len);
        strncpy(target, cur->value, len);
        if (&next->list != head && !strcmp(target, next->value)) {
            while (&cur->list != head && !strcmp(target, cur->value)) {
                list_del_init(&cur->list);
                q_release_element(cur);
                cur = next;
                next = list_entry(next->list.next, element_t, list);
            }
        }
        free(target);
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    struct list_head *first, *second;
    list_for_each_safe (first, second, head) {
        if (first == head || second == head)
            return;
        list_del(first);
        first->prev = second;
        first->next = second->next;
        second->next = first;
        first->next->prev = first;
        second = first->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur, *next;
    list_for_each_safe (cur, next, head) {
        list_del(cur);
        list_add(cur, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *dummy_head = head, *cur, *next = head->next;
    size_t iteration = q_size(head) / k;
    for (int iter = 0; iter < iteration; iter++) {
        for (int i = 0; i < k; i++) {
            cur = next;
            next = cur->next;
            list_del(cur);
            list_add(cur, dummy_head);
        }
        dummy_head = next->prev;
    }
}

struct list_head *mergeTwoLists(struct list_head *L1, struct list_head *L2)
{
    struct list_head *head = NULL, **ptr = &head, **node;

    for (node = NULL; L1 && L2; *node = (*node)->next) {
        char *str1 = list_entry(L1, element_t, list)->value;
        char *str2 = list_entry(L2, element_t, list)->value;
        node = strcmp(str1, str2) < 0 ? &L1 : &L2;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((uintptr_t) L1 | (uintptr_t) L2);
    return head;
}

struct list_head *merge_sort(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head *slow = head;
    for (struct list_head *fast = head->next; fast && fast->next;
         fast = fast->next->next)
        slow = slow->next;
    struct list_head *mid = slow->next;
    slow->next = NULL;
    struct list_head *left = merge_sort(head), *right = merge_sort(mid);
    return mergeTwoLists(left, right);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    head->prev->next = NULL;
    head->next = merge_sort(head->next);
    struct list_head *node;
    for (node = head; node->next != NULL; node = node->next) {
        node->next->prev = node;
    }
    node->next = head;
    head->prev = node;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    char *max = list_entry(head->prev, element_t, list)->value;
    for (struct list_head *node = head->prev->prev, *prev = node->prev;
         node != head; node = prev, prev = prev->prev) {
        char *target = list_entry(node, element_t, list)->value;
        if (strcmp(max, target) > 0)
            max = target;
        else {
            list_del_init(node);
            q_release_element(list_entry(node, element_t, list));
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    char *max = list_entry(head->prev, element_t, list)->value;
    for (struct list_head *node = head->prev->prev, *prev = node->prev;
         node != head; node = prev, prev = prev->prev) {
        char *target = list_entry(node, element_t, list)->value;
        if (strcmp(max, target) < 0)
            max = target;
        else {
            list_del_init(node);
            q_release_element(list_entry(node, element_t, list));
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    queue_contex_t *cur_contex = NULL;
    struct list_head *merged = NULL;
    list_for_each_entry (cur_contex, head, chain) {
        struct list_head *cur_q = cur_contex->q;
        cur_q->prev->next = NULL;
        merged = mergeTwoLists(merged, cur_q->next);
    }
    struct list_head *main_q = list_first_entry(head, queue_contex_t, chain)->q;
    main_q->next = merged;
    struct list_head *node;
    for (node = main_q; node->next != NULL; node = node->next) {
        node->next->prev = node;
    }
    node->next = main_q;
    main_q->prev = node;
    for (struct list_head *cur = head->next->next; cur != head; cur = cur->next)
        INIT_LIST_HEAD(list_entry(cur, queue_contex_t, chain)->q);
    return q_size(main_q);
}
