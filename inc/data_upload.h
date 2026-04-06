#ifndef DATA_UPLOAD_H
#define DATA_UPLOAD_H

#include <stdint.h>

/* ---- upload method interface ---- */
typedef struct upload_method upload_method_t;

struct upload_method {
    int (*init)(upload_method_t *m);
    int (*upload_data)(char *json_str);
    int (*deinit)(upload_method_t *m);

    upload_method_t *next;
};

/* ---- API ---- */
void register_upload_methods(void);
void add_upload_method(upload_method_t *m);
void remove_upload_method(upload_method_t *m);

int upload_data(char *json_str);
void push_data(char *json_str);
void data_upload_init(void);

/* ---- thread ---- */
void data_upload_thread(void *arg);

#endif /* DATA_UPLOAD_H */
