#ifndef CLOUD_CMDS_H
#define CLOUD_CMDS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ---- command structure ---- */
typedef struct {
    int   cmd_id;
    void *data;
} cloud_cmd_t;

/* ---- API ---- */
void cloud_cmds_init(void);
int  cloud_cmds_send(cloud_cmd_t *cmd);

/* ---- thread ---- */
void cloud_cmds_thread(void *arg);

#endif /* CLOUD_CMDS_H */