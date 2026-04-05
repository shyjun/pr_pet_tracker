#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "pr_msg.h"

typedef struct sensor sensor_t;

struct sensor {
    const char *name;
    void (*init)(sensor_t *s);
    int  (*read)(sensor_t *s);
    int  (*is_timeout)(sensor_t *s, uint32_t now);
    void (*update_timeout)(sensor_t *s, uint32_t now);

    void (*power_on)(sensor_t *s);
    void (*power_off)(sensor_t *s);
    void (*sleep)(sensor_t *s);
    void (*wake_up)(sensor_t *s);

    void (*append_sensor_data)(void *data);

    uint32_t period_ms;
    uint32_t next_due_time;

    uint8_t powered_off;
    uint8_t sleeping;

    void *data;
    void *last_read_data;
    int last_read_status;

    sensor_t *next;
};

/* core API */
void daq_init(void);
void daq_thread(void *arg);
int daq_post(pr_msg_t *msg);

void add_sensor(sensor_t *s);
void remove_sensor(sensor_t *s);

#endif