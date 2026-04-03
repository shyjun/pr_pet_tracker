#include "data_upload.h"

/* ---- instance ---- */
static upload_method_t ble_method;

/* ---- implementations ---- */
static int ble_init(upload_method_t *m)
{
    /* init BLE */
    return 0;
}

static int ble_upload(void *data)
{
    /* data is JSON string */
    /* send via BLE */

    return -1; /* change to 0 on success */
}

static int ble_deinit(upload_method_t *m)
{
    return 0;
}

/* ---- register ---- */
void register_ble(void)
{
    ble_method.init        = ble_init;
    ble_method.upload_data = ble_upload;
    ble_method.deinit      = ble_deinit;
    ble_method.next        = NULL;

    if (ble_method.init)
        ble_method.init(&ble_method);

    add_upload_method(&ble_method);
}