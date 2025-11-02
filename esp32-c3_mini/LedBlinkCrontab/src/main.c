#include <time.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Para debug en el monitor
#include "esp_rom_sys.h"  // Para ets_delay_us()

static const char *TAG = "CRONTAB";
static const char *TAG_LED = "LEDBLINK";

typedef enum {
    WAITING,
    START,
    DELAY_TO_ON,
    LED_ON,
    DELAY_TO_OFF,
    LED_OFF,
    FINISH
} led_state_t;

typedef struct {
    gpio_num_t pin;
    led_state_t state;
    uint8_t delayOn;
    uint8_t delayOff;
} led_t;

// Estructura para una Tarea Programada
typedef struct {
    int8_t day_of_week;  // Día de la semana (0=Domingo, 1=Lunes, ..., 6=Sábado)
    int8_t hour;         // Hora (0-23)
    int8_t minute;       // Minuto (0-59)
    int8_t second;       // Segundo (0-59)
    //task_callback_t callback; // Función a la que se debe llamar
    bool available_this_task; // Bandera para evitar ejecuciones múltiples
} ScheduledTask;


typedef struct {
    led_t *led;
    time_t *now;
} led_time_t;

// --- Array con todas nuestras tareas programadas ---
ScheduledTask tasks[] = {
    // Tarea 1: Todos los Lunes a las 10:34
    {
        .day_of_week = -1, // Cualquier día
        .hour = -1,
        .minute = -1,
        .second = 0,
        //.callback = tarea_del_lunes,
        .available_this_task = true
    },
    // Tarea 2: Todos los días a las 12:00
    // (Usamos -1 para indicar "cualquier día de la semana")
    {
        .day_of_week = -1, // Cualquier día
        .hour = -1,
        .minute = -1,
        .second = 30,
        //.callback = tarea_diaria_mediodia,
        .available_this_task = true
    }
};

const uint8_t num_tasks = sizeof(tasks) / sizeof(ScheduledTask);


void led_init(led_t *led, gpio_num_t pin) {
    led->pin = pin;
    led->state = WAITING;
    led->delayOn = 5;
    led->delayOff = 5;
}

const char* get_led_state_name(led_state_t state) {
    switch (state) {
        case WAITING:       return "WAITING";
        case START:         return "START";
        case DELAY_TO_ON:   return "DELAY_TO_ON";
        case LED_ON:        return "LED_ON";
        case DELAY_TO_OFF:  return "DELAY_TO_OFF";
        case LED_OFF:       return "LED_OFF";
        case FINISH:        return "FINISH";
        
        // Es buena práctica tener un 'default' por si acaso
        default:            return "UNKNOWN_STATE";
    }
}

void led_blink(led_t *led) {
    ESP_LOGI(TAG_LED, "led_blink: state %s, on delay %d, off delay %d", get_led_state_name(led->state), led->delayOn, led->delayOff);
    switch (led->state) {
        case WAITING:
            //led->state = START;
            break;
        case START:
            led->state = DELAY_TO_ON;
            break;
        case DELAY_TO_ON:
            if(led->delayOn > 0) {
                led->delayOn--;
            } else {
                led->state = LED_ON;
            }
            break;
        case LED_ON:
            gpio_set_level(led->pin, 0);
            led->state = DELAY_TO_OFF;
            break;
        case DELAY_TO_OFF:
            if(led->delayOff > 0) {
                led->delayOff--;
            } else {
                led->state = LED_OFF;
            }
            break;
        case LED_OFF:
            gpio_set_level(led->pin, 1);
            led->state = FINISH;
            break;
        case FINISH:
            led->delayOn = 5;
            led->delayOff = 5;
            led->state = WAITING; // Dejo el estado listo para el siguiente parpadeo
            for (uint8_t i = 0; i < num_tasks; i++) {
                if (!tasks[i].available_this_task) {
                    tasks[i].available_this_task = true; // Marcar la tarea como disponible
                }
            }
            break;
        default:
            break;
    }
}

static void led_blink_periodic_timer_callback(void* led) {
    led_blink((led_t*)led);
}

static void crontab_periodic_timer_callback(void* led_time) {
    led_time_t *ledTime = (led_time_t*)led_time;

    time_t *now = ledTime->now;
    led_t *led = ledTime->led;

    time(now);
    struct tm timeinfo;
    localtime_r(now, &timeinfo);

    // Recorremos todas las tareas para ver si alguna debe ejecutarse
    for (uint8_t i = 0; i < num_tasks; i++) {
        bool day_match = (tasks[i].day_of_week == -1) || (tasks[i].day_of_week == timeinfo.tm_wday);
        bool hour_match = (tasks[i].hour == -1) || (tasks[i].hour == timeinfo.tm_hour);
        bool minute_match = (tasks[i].minute == -1) || (tasks[i].minute == timeinfo.tm_min);
        bool second_match = (tasks[i].second == -1) || (tasks[i].second == timeinfo.tm_sec);

        if (day_match && hour_match && minute_match && second_match && tasks[i].available_this_task) {
            ESP_LOGI(TAG, "crontab_periodic_timer_callback: Task %d executed", tasks[i].second);
            tasks[i].available_this_task = false;  // Marcar la tarea como no disponible/en ejecución
            led->state = START; // Iniciar el parpadeo del LED
        }
    }
}

void app_main() {

    // Delay para esperar a que cargue el monitor
    vTaskDelay( (4 * 1000) / portTICK_PERIOD_MS);

    led_t led;
    led_init(&led, GPIO_NUM_8);

    gpio_reset_pin(GPIO_NUM_8);
    gpio_set_direction(GPIO_NUM_8, GPIO_MODE_OUTPUT);
    gpio_set_level(led.pin, 1);

    esp_timer_handle_t led_blink_periodic_timer;
    const esp_timer_create_args_t led_blink_periodic_timer_args = {
        .callback = &led_blink_periodic_timer_callback,
        .arg = &led,
        .name = "led_blink_periodic_timer_callback"
    };

    ESP_ERROR_CHECK(esp_timer_create(&led_blink_periodic_timer_args, &led_blink_periodic_timer));

    // Iniciar el temporizador para que se ejecute cada 1,000,000 microsegundos (1 segundo)
    ESP_ERROR_CHECK(esp_timer_start_periodic(led_blink_periodic_timer, 1 * 1000000));


    // CRONTAB
    time_t now;
    setenv("TZ", "CST+3", 1);
    tzset();

    led_time_t led_time;
    led_time.now = &now;
    led_time.led = &led;

    esp_timer_handle_t corntab_periodic_timer;
    const esp_timer_create_args_t corntab_periodic_timer_args = {
        .callback = &crontab_periodic_timer_callback,
        .arg = &led_time,
        .name = "corntab_periodic_timer_callback"
    };

    ESP_ERROR_CHECK(esp_timer_create(&corntab_periodic_timer_args, &corntab_periodic_timer));

    // Iniciar el temporizador para que se ejecute cada 500,000 microsegundos (0.5 segundos)
    ESP_ERROR_CHECK(esp_timer_start_periodic(corntab_periodic_timer, 500 * 1000));


    // Para no salir de la función y con delay para no bloquear el sistema, y consumo de CPU
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}