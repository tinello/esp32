# IoT Garden Irrigation

- To start, syncronice date with SNTP.
- Crontab with irrigation task.
- Webserver:
    - Service info
    - Irrigation Stated
    - List of schedule task


## Requirements

### Environment variables:

 - WIFI_SSID=my-wifi-ssid
 - WIFI_PASSWORD=my-wifi-password


## Development

### Visual Studio Code Extensions:

 - PlatformIO IDE (https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)



## Endpoints

| Name                  | Endpoint                                 |
|-----------------------|------------------------------------------|
| Service Info          | http://[ESP32-IP]/                       |
| Irrigation Stated     | http://[ESP32-IP]/get_irrigation         |
| List of schedule task | http://[ESP32-IP]/get_scheduled          |

