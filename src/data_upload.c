#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "data_upload.h"

/* ---- upload methods list ---- */
static upload_method_t *g_methods = NULL;

/* ---- queue ---- */
static QueueHandle_t g_upload_queue;

/* ---- extern register ---- */
extern void register_ble(void);
extern void register_wifi(void);
extern void register_cellular(void);

void register_upload_methods(void)
{
    register_ble();
    register_wifi();
    register_cellular();
}

/* ---- add/remove upload method ---- */
void add_upload_method(upload_method_t *m)
{
    if (!m) return;
    m->next = g_methods;
    g_methods = m;
}

void remove_upload_method(upload_method_t *m)
{
    if (!g_methods || !m) return;

    upload_method_t *curr = g_methods, *prev = NULL;

    while (curr)
    {
        if (curr == m)
        {
            if (prev) prev->next = curr->next;
            else g_methods = curr->next;

            curr->next = NULL;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

/* ---- upload dispatcher ---- */
int upload_data(void *ptr)
{
    upload_method_t *m = g_methods;

    while (m)
    {
        if (m->upload_data)
        {
            if (m->upload_data(ptr) == 0)
                return 0;
        }
        m = m->next;
    }

    printf("no upload method available\n");
    return -1;
}

/* ---- push data ---- */
void push_data(pr_msg_t *msg)
{
    if (!msg) return;

    if (xQueueSend(g_upload_queue, &msg, 0) != pdPASS)
    {
        printf("upload queue full\n");
        free_pr_msg(msg);
    }
}

/* ---- upload thread ---- */
void data_upload_thread(void *arg)
{
    (void)arg;
    pr_msg_t *msg;

    while (1)
    {
        if (xQueueReceive(g_upload_queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            upload_data(msg->data);
            free_pr_msg(msg);
        }
    }
}

/* ---- init ---- */
void data_upload_init(void)
{
    g_upload_queue = xQueueCreate(10, sizeof(pr_msg_t *));

    register_upload_methods();

    xTaskCreate(data_upload_thread,
                "upload",
                2048,
                NULL,
                2,
                NULL);
}
