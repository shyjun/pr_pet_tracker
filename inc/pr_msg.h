#ifndef PR_MSG_H
#define PR_MSG_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* ---- message type ownership (MSB bits) ---- */
typedef enum {
    MSG_TYPE_DAQ    = 1u << 31,
    MSG_TYPE_CLOUD  = 1u << 30,
    MSG_TYPE_UPLOAD = 1u << 29,
} msg_type_t;

#define MSG_TYPE_MASK (MSG_TYPE_DAQ | MSG_TYPE_CLOUD | MSG_TYPE_UPLOAD)

#define msg_is_type(msg, type) (((msg)->msgid & MSG_TYPE_MASK) == (type))

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
