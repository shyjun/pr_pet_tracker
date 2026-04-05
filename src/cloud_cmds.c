#include <stdio.h>
#include "cloud_cmds.h"

#define CLOUD_CMDS_QUEUE_LEN   10
#define CLOUD_CMDS_TASK_STACK  2048
#define CLOUD_CMDS_TASK_PRIO   2

/* ---- queue ---- */
static QueueHandle_t g_cmd_queue;

/* ---- handler ---- */
static void handle_command(pr_msg_t *msg)
{
    if (!msg)
        return;

    switch (msg->msgid)
    {
        case 1:
            printf("CMD 1 received\n");
            break;

        case 2:
            printf("CMD 2 received\n");
            break;

        default:
            printf("Unknown CMD: %u\n", msg->msgid);
            break;
    }
}

/* ---- thread ---- */
void cloud_cmds_thread(void *arg)
{
    (void)arg;

    pr_msg_t *msg;

    while (1)
    {
        if (xQueueReceive(g_cmd_queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            handle_command(msg);
            free_pr_msg(msg);
        }
    }
}

/* ---- send command ---- */
int cloud_cmds_send(pr_msg_t *msg)
{
    if (!msg)
        return -1;

    if (xQueueSend(g_cmd_queue, &msg, 0) != pdPASS)
        return -1;

    return 0;
}

/* ---- init ---- */
void cloud_cmds_init(void)
{
    g_cmd_queue = xQueueCreate(CLOUD_CMDS_QUEUE_LEN, sizeof(pr_msg_t *));

    xTaskCreate(cloud_cmds_thread,
                "cloud_cmds",
                CLOUD_CMDS_TASK_STACK,
                NULL,
                CLOUD_CMDS_TASK_PRIO,
                NULL);
}
