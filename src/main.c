#include <stdio.h>
#include <assert.h>
#include "FreeRTOS.h"
#include "task.h"

#include "sensors.h"
#include "data_upload.h"

/* ---- platform specific ---- */
static void init_peripherals(void)
{
    /* clocks, GPIO, I2C, SPI, UART, etc */
}

/* ---- system init task ---- */
static void system_init_task(void *arg)
{
    (void)arg;

    /* init peripherals */
    init_peripherals();

    /* register sensors + start DAQ thread */
    daq_init();

    /* init upload framework + thread */
    data_upload_init();

    /* optionally delete this task */
    vTaskDelete(NULL);
}

int main(void)
{
    /* basic HW init (very early) */
    init_peripherals();

    /* create init task (keeps main clean) */
    BaseType_t ret = xTaskCreate(system_init_task,
                "sys_init",
                2048,
                NULL,
                3,
                NULL);
    assert(ret == pdPASS);

    /* start scheduler */
    vTaskStartScheduler();

    /* should never reach here */
    while (1)
    {
    }

    return 0;
}