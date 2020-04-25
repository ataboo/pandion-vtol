#include "flight_control.h"

static xQueueHandle timer_queue;

static void IRAM_ATTR timer_tick_isr(void *para) 
{
    timer_spinlock_take(IBUS_TIMER_GROUP);
    
    uint64_t timer_counter_value = timer_group_get_counter_value_in_isr(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);

    timer_event_t evt;
    evt.timer_counter_value = timer_counter_value;

    timer_group_clr_intr_status_in_isr(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);
    timer_group_enable_alarm_in_isr(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);

    xQueueSendFromISR(timer_queue, &evt, NULL);
    timer_spinlock_give(IBUS_TIMER_GROUP);
}

xQueueHandle init_timer() {
    if (timer_queue != NULL) {
        return NULL;
    }

    timer_queue = xQueueCreate(IBUS_UART_QUEUE_SIZE, sizeof(timer_event_t));
    
    timer_config_t timer_cfg = {
        .divider = IBUS_TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = true,
#ifdef TIMER_GROUP_SUPPORTS_XTAL_CLOCK
        .clk_src = TIMER_SRC_CLK_APB
#endif
    };

    timer_init(IBUS_TIMER_GROUP, IBUS_TIMER_IDX, &timer_cfg);
    timer_set_counter_value(IBUS_TIMER_GROUP, IBUS_TIMER_IDX, 0x00000000ULL);
    timer_set_alarm_value(IBUS_TIMER_GROUP, IBUS_TIMER_IDX, IBUS_TIMER_MS_ALARM);
    timer_enable_intr(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);
    timer_isr_register(IBUS_TIMER_GROUP, IBUS_TIMER_IDX, timer_tick_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);

    return timer_queue;
}
