#include "sensors.h"
#include <string.h>

#define DAQ_TASK_STACK   2048
#define DAQ_TASK_PRIO    2

static sensor_t *g_sensors_head = NULL;

/* ---- Default timeout (simple & readable) ---- */
static int default_is_timeout(sensor_t *s, uint32_t now)
{
    if (now > s->next_due_time)
        return 1;
    else
        return 0;
}

static void default_update_timeout(sensor_t *s, uint32_t now)
{
    uint32_t p = s->period_ms;
    uint32_t aligned = ((now + p - 1) / p) * p;
    s->next_due_time = aligned;
}

/* ---- Time helper ---- */
static uint32_t now_ms(void)
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

/* ---- Add sensor ---- */
void add_sensor(sensor_t *s)
{
    if (!s)
        return;

    s->next = g_sensors_head;
    g_sensors_head = s;
}

/* ---- Remove sensor ---- */
void remove_sensor(sensor_t *s)
{
    if (!g_sensors_head || !s)
        return;

    sensor_t *curr = g_sensors_head;
    sensor_t *prev = NULL;

    while (curr)
    {
        if (curr == s)
        {
            if (prev)
                prev->next = curr->next;
            else
                g_sensors_head = curr->next;

            curr->next = NULL;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

/* ---- Get sensor by name ---- */
sensor_t* get_sensor_by_name(const char *name)
{
    sensor_t *s = g_sensors_head;
    while (s)
    {
        if (s->name && strcmp(s->name, name) == 0)
            return s;
        s = s->next;
    }
    return NULL;
}
