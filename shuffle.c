#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "shuffle.h"

static void swap(struct list_head *a, struct list_head *b)
{
    struct list_head *a_prev = a->prev;
    struct list_head *b_prev = b->prev;
    if (a->prev != b)
        list_move(b, a_prev);
    list_move(a, b_prev);
}

void q_shuffle(struct list_head *head)
{
    if (!head || list_is_singular(head))
        return;

    int len = q_size(head);
    struct list_head *new = head;
    while (len > 0) {
        struct list_head *old = head->next;
        int steps = rand() % len;
        while (steps--)
            old = old->next;
        struct list_head *tmp = new->prev;
        swap(tmp, old);
        new = new->prev;
        len--;
    }
}