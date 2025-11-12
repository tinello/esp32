#include <time.h>
#include "irrigation.h"


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


void initialize_crontab_periodic_check(crontab_tasks_t *tasks);


