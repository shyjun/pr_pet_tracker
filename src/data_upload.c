#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "data_upload.h"
#include "pr_msg.h"

/* ---- pending msgs list ---- */
typedef struct pending_msg_node {
    char *json_str;
    struct pending_msg_node *prev;
    struct pending_msg_node *next;
} pending_msg_node_t;

static pending_msg_node_t *g_pending_head = NULL;
static pending_msg_node_t *g_pending_tail = NULL;

/* ---- add to pending list ---- */
static void pending_msgs_enqueue(char *json_str)
{
    pending_msg_node_t *node = malloc(sizeof(pending_msg_node_t));
    assert(node != NULL);
    node->json_str = json_str;
    node->prev = NULL;
    node->next = NULL;

    if (!g_pending_head)
        g_pending_head = g_pending_tail = node;
    else
    {
        g_pending_tail->next = node;
        node->prev = g_pending_tail;
        g_pending_tail = node;
    }
}

/* ---- pop from pending list ---- */
static char *pending_msgs_dequeue(void)
{
    if (!g_pending_head)
        return NULL;

    pending_msg_node_t *node = g_pending_head;
    g_pending_head = node->next;

    if (g_pending_head)
        g_pending_head->prev = NULL;
    else
        g_pending_tail = NULL;

    char *json_str = node->json_str;
    free(node);
    return json_str;
}

/* ---- queue for trigger ---- */
static QueueHandle_t data_upload_thread_msgq;

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
int upload_data(char *json_str)
{
    upload_method_t *m = g_methods;

    while (m)
    {
        if (m->upload_data)
        {
            if (m->upload_data(json_str) == 0)
                return 0;
        }
        m = m->next;
    }

    printf("no upload method available\n");
    return -1;
}

/* ---- push data ---- */
void push_data(char *json_str)
{
    assert(json_str != NULL);
    pending_msgs_enqueue(json_str);
}

void trigger_data_upload()
{
    pr_msg_t *msg = alloc_pr_msg(MSG_TYPE_UPLOAD, NULL);
    assert(xQueueSend(data_upload_thread_msgq, &msg, 0) == pdPASS);
}

/* ---- upload thread ---- */
void data_upload_thread(void *arg)
{
    (void)arg;
    pr_msg_t *msg;

    while (1)
    {
        BaseType_t ret = xQueueReceive(data_upload_thread_msgq, &msg, portMAX_DELAY);
        assert(ret == pdTRUE);
        assert(msg_is_type(msg, MSG_TYPE_UPLOAD));
        free_pr_msg(msg);

        while(1)
        {
            char *json_str = pending_msgs_dequeue();
            if (!json_str)
                break;

            upload_data(json_str);
            free(json_str);
        }
    }
}

/* ---- init ---- */
void data_upload_init(void)
{
    data_upload_thread_msgq = xQueueCreate(10, sizeof(pr_msg_t *));
    assert(data_upload_thread_msgq != NULL);

    register_upload_methods();

    BaseType_t ret = xTaskCreate(data_upload_thread,
                "upload",
                2048,
                NULL,
                2,
                NULL);
    assert(ret == pdPASS);
}
