#pragma once
#include "driver/gpio.h"

typedef enum {
    WAITING,
    START,
    DELAY_TO_SOLENOID_ON,
    SOLENOID_ON,
    DELAY_TO_PUMP_ON,
    PUMP_ON,
    DELAY_TO_IRRIGATION,
    PUMP_OFF,
    DELAY_TO_SOLENOID_OFF,
    SOLENOID_OFF,
    DELAY_TO_PUMP_REFRESH,
    FINISH
} irrigation_state_t;

typedef struct {
    gpio_num_t solenoid_pin;
    gpio_num_t pump_pin;
    gpio_num_t status_led_pin;
    irrigation_state_t state;
    int delayToSolenoidOn;
    int delayToPumpOn;
    int delayToIrrigation;
    int delayToPumpOff;
    int delayToSolenoidOff;
    int delayToPumpRefresh;
} irrigation_t;

void initialize_irrigation_periodic_check(irrigation_t *irrigation);