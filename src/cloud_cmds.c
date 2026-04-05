#include <stdio.h>
#include "cloud_cmds.h"

#define CLOUD_CMDS_QUEUE_LEN   10
#define CLOUD_CMDS_TASK_STACK  2048
#define CLOUD_CMDS_TASK_PRIO   2

/* ---- queue ---- */
static QueueHandle_t g_cmd_queue;

/* ---- handler ---- */
static void handle_command(cloud_cmd_t *cmd)
{
    if (!cmd)
        return;

    switch (cmd->cmd_id)
    {
        case 1:
            printf("CMD 1 received\n");
            break;

        case 2:
            printf("CMD 2 received\n");
            break;

        default:
            printf("Unknown CMD: %d\n", cmd->cmd_id);
            break;
    }
}

/* ---- thread ---- */
void cloud_cmds_thread(void *arg)
{
    (void)arg;

    cloud_cmd_t cmd;

    while (1)
    {
        /* wait forever for command */
        if (xQueueReceive(g_cmd_queue, &cmd, portMAX_DELAY) == pdTRUE)
        {
            handle_command(&cmd);
        }
    }
}

/* ---- send command ---- */
int cloud_cmds_send(cloud_cmd_t *cmd)
{
    if (!cmd)
        return -1;

    if (xQueueSend(g_cmd_queue, cmd, 0) != pdPASS)
        return -1;

    return 0;
}

/* ---- init ---- */
void cloud_cmds_init(void)
{
    g_cmd_queue = xQueueCreate(CLOUD_CMDS_QUEUE_LEN, sizeof(cloud_cmd_t));

    xTaskCreate(cloud_cmds_thread,
                "cloud_cmds",
                CLOUD_CMDS_TASK_STACK,
                NULL,
                CLOUD_CMDS_TASK_PRIO,
                NULL);
}