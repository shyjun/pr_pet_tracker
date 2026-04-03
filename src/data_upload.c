#include <stdio.h>
#include <stdlib.h>
#include "data_upload.h"

/* ---- upload methods list ---- */
static upload_method_t *g_methods = NULL;

/* ---- data list ---- */
static data_node_t *g_data_head = NULL;
static data_node_t *g_data_tail = NULL;

/* ---- sync ---- */
static QueueHandle_t g_upload_queue;

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
void push_data(char *data)
{
    data_node_t *node = pvPortMalloc(sizeof(data_node_t));
    if (!node) return;

    node->data = data;
    node->next = NULL;

    if (!g_data_head)
    {
        g_data_head = g_data_tail = node;
    }
    else
    {
        g_data_tail->next = node;
        g_data_tail = node;
    }

    /* notify upload thread */
    uint8_t msg = 1;
    xQueueSend(g_upload_queue, &msg, 0);
}

/* ---- pop one ---- */
static data_node_t* pop_data(void)
{
    if (!g_data_head) return NULL;

    data_node_t *node = g_data_head;
    g_data_head = node->next;

    if (!g_data_head)
        g_data_tail = NULL;

    return node;
}

/* ---- upload thread ---- */
void data_upload_thread(void *arg)
{
    (void)arg;
    uint8_t msg;

    while (1)
    {
        /* wait for trigger */
        xQueueReceive(g_upload_queue, &msg, portMAX_DELAY);

        /* process all available data */
        while (1)
        {
            data_node_t *node = pop_data();
            if (!node)
                break;

            upload_data(node->data);

            vPortFree(node);
        }
    }
}

/* ---- init ---- */
void data_upload_init(void)
{
    g_upload_queue = xQueueCreate(10, sizeof(uint8_t));

    register_upload_methods();

    xTaskCreate(data_upload_thread,
                "upload",
                2048,
                NULL,
                2,
                NULL);
}