
#include <assert.h>
#include <jansson.h>
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
extern const char* get_ID(void);

/* ---- register all sensors ---- */
static void register_sensors(void)
{
#if (SENSOR_1_ENABLED==1)
    add_sensor_1();
#endif
    add_sensor_2();
}

void handle_daq_msg(pr_msg_t *msg)
{

}

#if 0  // sample format
{
  "uuid": "",
  "time": "",
  "sensors": {
    "hr": {
      "rate": "",
      "max": ""
    },
    "gps": {
      "long": "",
      "lat": "",
      "height": ""
    }
  }
}
#endif

void init_daq_json_fields(json_t *json_root)
{
    int ret = 0;
    ret |= json_object_set_new(json_root, "ID", json_string(get_ID()));
    assert(ret == 0);
    ret |= json_object_set_new(json_root, "time", json_integer(now_ms()));
    assert(ret == 0);
}

void daq_thread(void *arg)
{
    pr_msg_t *msg;
    uint32_t now;
    uint32_t next_wakeup;
    json_t *json_root;
    (void)arg;

    while (1)
    {
        now = now_ms();
        json_root = NULL;
        next_wakeup = UINT32_MAX;

        for (sensor_t *s = g_sensors_head; s; s = s->next)
        {
            if (s->is_timeout(s, now))
            {
                if (s->powered_off)
                {
                    if (s->power_on)
                        s->power_on(s);
                    s->powered_off = 0;
                }

                if (s->sleeping)
                {
                    if (s->wake_up)
                        s->wake_up(s);
                    s->sleeping = 0;
                }

                int read_success = 0;
                if (s->read)
                    read_success = s->read(s);
                s->last_read_status = read_success;

                if (read_success)
                {
                    if (NULL == json_root)
                    {
                        /* create json object */
                        json_root = json_object();
                        init_daq_json_fields(json_root);
                    }

                    if (s->append_sensor_data)
                        s->append_sensor_data(s, json_root);
                }

                if (s->put_to_sleep_after_read)
                {
                    if (s->sleep)
                        s->sleep(s);
                    s->sleeping = 1;
                }

                if (s->poweroff_after_read)
                {
                    if (s->power_off)
                        s->power_off(s);
                    s->powered_off = 1;
                }

                if (s->update_timeout)
                    s->update_timeout(s, now);
            }

            if (s->next_due_time < next_wakeup)
                next_wakeup = s->next_due_time;
        }

        if (NULL != json_root)
        {
            /* valid sensor data available */
            char *json_str = json_dumps(json_root, JSON_COMPACT);
            json_decref(json_root);
            push_data(json_str);
        }

        uint32_t delay_ms = (next_wakeup > now) ? (next_wakeup - now) : 1;
        BaseType_t ret = xQueueReceive(g_daq_msgq, &msg, pdMS_TO_TICKS(delay_ms));
        if (ret == pdTRUE)
        {
            assert(msg_is_type(msg, MSG_TYPE_DAQ));
            handle_daq_msg(msg);
            free_pr_msg(msg);
        }
    }
}

int daq_post(pr_msg_t *msg)
{
    assert(msg != NULL);
    assert(xQueueSend(g_daq_msgq, &msg, 0) == pdPASS);
    return 0;
}

void daq_init(void)
{
    g_daq_msgq = xQueueCreate(1, sizeof(pr_msg_t *));
    assert(g_daq_msgq != NULL);

    register_sensors();

    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    for (sensor_t *s = g_sensors_head; s; s = s->next)
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

    BaseType_t ret = xTaskCreate(daq_thread, "daq", 2048, NULL, 2, NULL);
    assert(ret == pdPASS);
}
