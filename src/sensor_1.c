#include "sensors.h"

#if (SENSOR_1_ENABLED==1)

#define SENSOR_1_NAME "sensor_1"

/* ---- static instance ---- */
static sensor_t sensor1;
static int s1_last_read_data;

/* ---- implementations ---- */
static void s1_init(sensor_t *s)
{
    s->name = SENSOR_1_NAME;
}

static int s1_read(sensor_t *s)
{
    /* read sensor */
    return 0;
}

static void s1_power_on(sensor_t *s)  { /* hw */ }
static void s1_power_off(sensor_t *s) { /* hw */ }
static void s1_sleep(sensor_t *s)     { /* hw */ }
static void s1_wake(sensor_t *s)      { /* hw */ }

static void s1_append(void *s, json_t *json_obj)
{
    int ret = 0;
    json_t *s1_obj;

    s1_obj = json_object();
    assert(s1_obj != NULL);

    ret |= json_object_set_new(s1_obj, "name", json_string(s->name));
    assert(ret == 0);

    ret |= json_object_set_new(s1_obj, "val", integer(s->last_read_data));
    assert(ret == 0);

    ret |= json_object_set_new(json_obj, "s1_data", s1_obj);
    assert(ret == 0);
}


static void s1_append(sensor_t *s, json_t *json_obj)
{
    (void)s;
}

/* ---- register function ---- */
void add_sensor_1(void)
{
    sensor1.init  = s1_init;
    sensor1.read  = s1_read;

    sensor1.power_on  = s1_power_on;
    sensor1.power_off = s1_power_off;
    sensor1.sleep     = s1_sleep;
    sensor1.wake_up   = s1_wake;

    sensor1.append_sensor_data = s1_append;

    sensor1.period_ms = 10000;

    sensor1.powered_off = 1;
    sensor1.sleeping    = 0;
    sensor1.put_to_sleep_after_read = 0;
    sensor1.poweroff_after_read = 0;

    sensor1.data = NULL;
    sensor1.last_read_data = &s1_last_read_data;
    sensor1.next = NULL;

    /* call init BEFORE adding */
    if (sensor1.init)
        sensor1.init(&sensor1);

    add_sensor(&sensor1);
}

#endif