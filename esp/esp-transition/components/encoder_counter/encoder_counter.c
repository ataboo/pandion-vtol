#include "encoder_counter.h"

struct encoder_counter_impl {
    int count;
    gpio_num_t gpio_num;
    int direction;
};


static void IRAM_ATTR encoder_isr_handler(void* args) {
    encoder_counter_handle_t handle = (encoder_counter_handle_t)args;
    handle->count += handle->direction;
}

void encoder_set_direction(encoder_counter_handle_t handle, int direction) {
    handle->direction = direction;
}

encoder_counter_handle_t encoder_counter_init(encoder_cfg_t* config) {
    encoder_counter_handle_t handle = malloc(sizeof(struct encoder_counter_impl));
    handle->gpio_num = config->gpio_num;
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = (1ULL<<config->gpio_num),
        .intr_type = GPIO_PIN_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 1,
    };

    gpio_config(&gpio_cfg);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(config->gpio_num, encoder_isr_handler, (void*) handle);

    handle->count = 0;
    handle->direction = 0;

    return handle;
}

void encoder_counter_set_count(encoder_counter_handle_t handle, int count) {
    handle->count = count;
}

int encoder_counter_current_count(encoder_counter_handle_t handle) {
    return handle->count;
}

void encoder_counter_terminate(encoder_counter_handle_t handle) {
    gpio_isr_handler_remove(handle->gpio_num);
}

void encoder_counter_set_direction(encoder_counter_handle_t handle, int direction) {
    handle->direction = direction;
}
