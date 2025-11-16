#include "esp_timer.h"
#include "irrigation.h"
#include "esp_log.h"
#include "driver/gpio.h"


static const char *TAG = "IRRIGATION";


void irrigation_process(irrigation_t *irrigation) {
    ESP_LOGI(TAG, "Irrigation state: %d", irrigation->state);
    switch (irrigation->state) {
        case WAITING:
            break;
        case START:
            gpio_set_level(irrigation->status_led_pin, 0);
            irrigation->state = DELAY_TO_SOLENOID_ON;
            break;
        case DELAY_TO_SOLENOID_ON:
            if(irrigation->delayToSolenoidOn > 0) {
                irrigation->delayToSolenoidOn--;
            } else {
                irrigation->state = SOLENOID_ON;
            }
            break;
        case SOLENOID_ON:
            gpio_set_level(irrigation->solenoid_pin, 1);
            irrigation->state = DELAY_TO_PUMP_ON;
            break;
        case DELAY_TO_PUMP_ON:
            if(irrigation->delayToPumpOn > 0) {
                irrigation->delayToPumpOn--;
            } else {
                irrigation->state = PUMP_ON;
            }
            break;
        case PUMP_ON:
            gpio_set_level(irrigation->pump_pin, 1);
            irrigation->state = DELAY_TO_IRRIGATION;
            break;
        case DELAY_TO_IRRIGATION:
            if(irrigation->delayToPumpOff > 0) {
                irrigation->delayToPumpOff--;
            } else {
                irrigation->state = PUMP_OFF;
            }
            break;
        case PUMP_OFF:
            gpio_set_level(irrigation->pump_pin, 0);
            irrigation->state = DELAY_TO_SOLENOID_OFF;
            break;
        case DELAY_TO_SOLENOID_OFF:
            if(irrigation->delayToSolenoidOff > 0) {
                irrigation->delayToSolenoidOff--;
            } else {
                irrigation->state = SOLENOID_OFF;
            }
            break;
        case SOLENOID_OFF:
            gpio_set_level(irrigation->solenoid_pin, 0);
            irrigation->state = DELAY_TO_PUMP_REFRESH;
            break;
        case DELAY_TO_PUMP_REFRESH:
            if(irrigation->delayToPumpRefresh > 0) {
                irrigation->delayToPumpRefresh--;
            } else {
                irrigation->state = FINISH;
            }
            break;
        case FINISH:
            gpio_set_level(irrigation->status_led_pin, 1);
            irrigation->delayToSolenoidOn = 5;
            irrigation->delayToPumpOn = 5;
            irrigation->delayToIrrigation = 5;
            irrigation->delayToPumpOff = 5;
            irrigation->delayToSolenoidOff = 5;
            irrigation->delayToPumpRefresh = 5;
            irrigation->state = WAITING;
            break;
        default:
            break;
    }
}




/**************************************************************
 * Funciones para la verificación periódica de riego
 **************************************************************/

static void periodic_irrigation_timer_callback(void *arg) {
    irrigation_t *irrigation = (irrigation_t *)arg;
    irrigation_process(irrigation);
}

void initialize_irrigation_periodic_check(irrigation_t *irrigation) {
    const int micro = 1000;
    const int milli = 1000;
    const int second = 1;
    const int period = second * milli * micro;

    esp_timer_handle_t periodic_timer;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_irrigation_timer_callback,
        .arg = irrigation,
        .name = "irrigation_periodic_task"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, period));
}
