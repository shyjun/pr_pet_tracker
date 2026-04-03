
#include "sensors.h"

/* externs from each sensor file */
extern void add_sensor_1(void);
extern void add_sensor_2(void);


void daq_thread(void *arg)
{
    (void)arg;

    while (1)
    {
        uint32_t now = now_ms();
        uint32_t next_wakeup = UINT32_MAX;

        for (sensor_t *s = g_head; s; s = s->next)
        {
            if (s->is_timeout(s, now))
            {
                /* power handling */
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

                /* read */
                if (s->read) s->read(s);

                /* append hook (user can implement) */
                if (s->append_sensor_data)
                    s->append_sensor_data(s->data);

                /* go back to sleep */
                if (s->sleep)
                {
                    s->sleep(s);
                    s->sleeping = 1;
                }

                /* schedule next */
                s->next_due_time += s->period_ms;
            }

            if (s->next_due_time < next_wakeup)
                next_wakeup = s->next_due_time;
        }

        uint32_t delay_ms = (next_wakeup > now) ? (next_wakeup - now) : 1;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void daq_init(void)
{
    /* register all sensors */
    add_sensor_1();
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