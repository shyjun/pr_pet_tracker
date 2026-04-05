
#include "sensors.h"
#include "queue.h"
#include "data_upload.h"
#include "pr_msg.h"

static QueueHandle_t g_daq_msgq;

/* externs from each sensor file */
#if (SENSOR_1_ENABLED==1)
extern void add_sensor_1(void);
#endif
extern void add_sensor_2(void);


void daq_thread(void *arg)
{
    uint8_t dummy;
    (void)arg;

    while (1)
    {
        uint32_t now = now_ms();
        uint32_t next_wakeup = UINT32_MAX;

        for (sensor_t *s = g_head; s; s = s->next)
        {
            if (s->is_timeout(s, now))
            {
                if (s->powered_off)
                {
                    if (s->power_on) s->power_on(s);
                    s->powered_off = 0;
                }

                if (s->sleeping)
                {
                    if (s->wake_up) s->wake_up(s);
                    s->sleeping = 0;
                }

                int read_success = 0;
                if (s->read) read_success = s->read(s);
                s->last_read_status = read_success;

                if (read_success)
                {
                    if (s->append_sensor_data)
                        s->append_sensor_data(s->last_read_data);
                }

                if (s->update_timeout)
                    s->update_timeout(s, now);
            }

            if (s->next_due_time < next_wakeup)
                next_wakeup = s->next_due_time;
        }

        uint32_t delay_ms = (next_wakeup > now) ? (next_wakeup - now) : 1;
        xQueueReceive(g_daq_msgq, &dummy, pdMS_TO_TICKS(delay_ms));
    }
}

int daq_post(pr_msg_t *msg)
{
    if (!msg) return -1;

    if (xQueueSend(g_daq_msgq, &msg, 0) != pdPASS)
    {
        free_pr_msg(msg);
        return -1;
    }

    return 0;
}

void daq_init(void)
{
    g_daq_msgq = xQueueCreate(1, sizeof(pr_msg_t *));

#if (SENSOR_1_ENABLED==1)
    add_sensor_1();
#endif
    add_sensor_2();

    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    for (sensor_t *s = g_head; s; s = s->next)
    {
        if (!s->is_timeout)
        {
            s->is_timeout = default_is_timeout;
        }

        if (!s->update_timeout)
        {
            s->update_timeout = default_update_timeout;
        }

        uint32_t p = s->period_ms;
        uint32_t aligned = ((now + p - 1) / p) * p;
        s->next_due_time = aligned;
    }

    xTaskCreate(daq_thread, "daq", 2048, NULL, 2, NULL);
}
