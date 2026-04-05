#ifndef PR_MSG_H
#define PR_MSG_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    uint32_t msgid;
    void    *data;
} pr_msg_t;

static inline void *alloc_pr_msg(uint32_t msgid, void *data)
{
    pr_msg_t *ptr = (pr_msg_t *)malloc(sizeof(pr_msg_t));
    assert(ptr != NULL);
    ptr->msgid = msgid;
    ptr->data  = data;
    return ptr;
}

static inline void free_pr_msg(pr_msg_t *msg)
{
    if (msg) {
        free(msg->data);
        free(msg);
    }
}

#endif
