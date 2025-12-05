#pragma once
#include <time.h>
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
    int delayToSolenoidOff;
    int delayToPumpRefresh;
} irrigation_t;





// Estructura para una Tarea Programada
typedef struct {
    int8_t day_of_week;  // Día de la semana (0=Domingo, 1=Lunes, ..., 6=Sábado)
    int8_t hour;         // Hora (0-23)
    int8_t minute;       // Minuto (0-59)
    int8_t second;       // Segundo (0-59)
    //task_callback_t callback; // Función a la que se debe llamar
    //bool available_this_task; // Bandera para evitar ejecuciones múltiples
} ScheduledTask;

typedef struct {
    uint8_t numTasks;
    ScheduledTask* tasks;
    irrigation_t *irrigation;
} crontab_tasks_t;
