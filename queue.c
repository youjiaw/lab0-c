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
    if (head) {
        INIT_LIST_HEAD(head);
        return head;
    }
    return NULL;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *node = head->next;
    element_t *element = list_entry(node, element_t, list);
    if (sp) {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(node);
    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *node = head->prev;
    element_t *element = list_entry(node, element_t, list);
    if (sp) {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(node);
    return element;
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
    struct list_head *left = head->next, *right = head->prev;
    while (!(left == right) && !(right->next == left)) {
        left = left->next;
        right = right->prev;
    }
    list_del(left);
    q_release_element(list_entry(left, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    element_t *current, *next;
    bool flag = false;
    list_for_each_entry_safe (current, next, head, list) {
        if (current->list.next == head)
            break;

        if (strcmp(current->value, next->value) == 0) {
            list_del(&current->list);
            q_release_element(current);
            flag = true;
        } else if (flag) {
            list_del(&current->list);
            q_release_element(current);
            flag = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *current;
    list_for_each (current, head) {
        if (current->next == head)
            break;

        list_move(current->next, current->prev);
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *first = head->next;
    while (first->next != head)
        list_move(first->next, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head))
        return;
    struct list_head *current, *safe, *ptr = head;
    int cnt = 0;
    list_for_each_safe (current, safe, head) {
        if (++cnt == k) {
            LIST_HEAD(tmp);
            list_cut_position(&tmp, ptr->next, current);
            q_reverse(&tmp);
            list_splice(&tmp, ptr);
            ptr = safe->prev;
            cnt = 0;
        }
    }
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

// Merge two lists in ascending/descending order
static void list_merge(struct list_head *head1,
                       struct list_head *head2,
                       bool descend)
{
    struct list_head merged;
    INIT_LIST_HEAD(&merged);

    while (!list_empty(head1) && !list_empty(head2)) {
        int compare_result =
            strcmp(list_entry(head1->next, element_t, list)->value,
                   list_entry(head2->next, element_t, list)->value);

        if ((descend && compare_result > 0) ||
            (!descend && compare_result < 0)) {
            struct list_head *tmp = head1->next;
            list_move_tail(tmp, &merged);
        } else {
            struct list_head *tmp = head2->next;
            list_move_tail(tmp, &merged);
        }
    }

    // Add remaining elements
    if (!list_empty(head1))
        list_splice_tail_init(head1, &merged);
    if (!list_empty(head2))
        list_splice_tail_init(head2, &merged);

    INIT_LIST_HEAD(head1);
    list_splice(&merged, head1);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *fast = head->next, *slow = head;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    struct list_head second_half;
    list_cut_position(&second_half, head, slow);
    q_sort(head, descend);
    q_sort(&second_half, descend);
    list_merge(head, &second_half, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    struct list_head *current = head->prev;
    while (current->prev != head) {
        struct list_head *prev = current->prev;
        if (strcmp(list_entry(current, element_t, list)->value,
                   list_entry(prev, element_t, list)->value) <= 0) {
            list_del_init(prev);
            q_release_element(list_entry(prev, element_t, list));
        } else
            current = current->prev;
    }
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    struct list_head *current = head->prev;
    while (current->prev != head) {
        struct list_head *prev = current->prev;
        if (strcmp(list_entry(current, element_t, list)->value,
                   list_entry(prev, element_t, list)->value) >= 0) {
            list_del_init(prev);
            q_release_element(list_entry(prev, element_t, list));
        } else
            current = current->prev;
    }
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
