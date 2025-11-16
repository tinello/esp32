#include "esp_timer.h"
#include "crontab.h"
#include "esp_log.h"

static const char *TAG = "CRONTAB";





/**************************************************************
 * Funciones para la verificación periódica del cron
 **************************************************************/

static void periodic_crontab_timer_callback(void *arg) {
    crontab_tasks_t *tasks = (crontab_tasks_t *)arg;

    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    for (size_t i = 0; i < tasks->numTasks; i++) {
        ScheduledTask *task = &(tasks->tasks[i]);
        bool day_match = (task->day_of_week == -1) || (task->day_of_week == timeinfo.tm_wday);
        bool hour_match = (task->hour == -1) || (task->hour == timeinfo.tm_hour);
        bool minute_match = (task->minute == -1) || (task->minute == timeinfo.tm_min);
        bool second_match = (task->second == -1) || (task->second == timeinfo.tm_sec);

        //ESP_LOGI(TAG, "crontab_periodic_timer_callback: Task %d executed, irrigation state: %d", i, tasks->irrigation->state);


        if (day_match && hour_match && minute_match && second_match && 
            /*task->available_this_task &&*/ tasks->irrigation->state == WAITING) {
            ESP_LOGI(TAG, "Start Irrigation: Task %d, day: %d, hour: %d, minute: %d, second: %d", i, task->day_of_week, task->hour, task->minute, task->second);
            tasks->irrigation->state = START; // Iniciar el parpadeo del LED
        }
    }
}

void initialize_crontab_periodic_check(crontab_tasks_t *crontab_tasks) {
    const int micro = 1000;
    const int milli = 800;
    const int period = milli * micro;

    esp_timer_handle_t periodic_timer;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_crontab_timer_callback,
        .arg = crontab_tasks,
        .name = "crontab_periodic_task"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, period));
}

