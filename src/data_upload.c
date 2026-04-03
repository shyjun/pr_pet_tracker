#include <stdio.h>
#include "data_upload.h"

/* ---- head of linked list ---- */
static upload_method_t *g_head = NULL;

/* ---- add ---- */
void add_upload_method(upload_method_t *m)
{
    if (!m)
        return;

    m->next = g_head;
    g_head = m;
}

/* ---- remove ---- */
void remove_upload_method(upload_method_t *m)
{
    if (!g_head || !m)
        return;

    upload_method_t *curr = g_head;
    upload_method_t *prev = NULL;

    while (curr)
    {
        if (curr == m)
        {
            if (prev)
                prev->next = curr->next;
            else
                g_head = curr->next;

            curr->next = NULL;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

/* ---- extern register functions ---- */
extern void register_ble(void);
extern void register_wifi(void);
extern void register_cellular(void);

/* ---- register all ---- */
void register_upload_methods(void)
{
    register_ble();
    register_wifi();
    register_cellular();
}

/* ---- upload dispatcher ---- */
int upload_data(void *ptr)
{
    upload_method_t *m = g_head;

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