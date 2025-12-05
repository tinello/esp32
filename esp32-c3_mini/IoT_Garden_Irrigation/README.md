# IoT Garden Irrigation

- Al iniciar el programa y cada 2 horas, se sincroniza la hora con SNTP.
- Crontab con ejecucion de tareas de riego.
- Webserver:
    - Para ver el listado de tareas, estados y fecha actual.


## Endpoints

| Name                  | Endpoint                                                           |
|-----------------------|--------------------------------------------------------------------|
| Service Info          | http://[ESP32-IP]/                                                 |
| Irrigation Stated     | http://[ESP32-IP]/get_irrigation                                   |
| List of schedule task | http://[ESP32-IP]/get_scheduled                                    |

