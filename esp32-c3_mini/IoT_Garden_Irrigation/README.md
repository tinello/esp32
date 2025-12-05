# IoT Garden Irrigation

- To start, syncronice date with SNTP.
- Crontab with irrigation task.
- Webserver:
    - Service info
    - Irrigation Stated
    - List of schedule task


## Endpoints

| Name                  | Endpoint                                                           |
|-----------------------|--------------------------------------------------------------------|
| Service Info          | http://[ESP32-IP]/                                                 |
| Irrigation Stated     | http://[ESP32-IP]/get_irrigation                                   |
| List of schedule task | http://[ESP32-IP]/get_scheduled                                    |

