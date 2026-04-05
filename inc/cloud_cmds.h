#ifndef CLOUD_CMDS_H
#define CLOUD_CMDS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pr_msg.h"

/* ---- API ---- */
void cloud_cmds_init(void);
int  cloud_cmds_send(pr_msg_t *msg);

/* ---- thread ---- */
void cloud_cmds_thread(void *arg);

#endif /* CLOUD_CMDS_H */
