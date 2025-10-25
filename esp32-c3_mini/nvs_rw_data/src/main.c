#include "esp_timer.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"
#include <stdio.h>


static const char *TAG = "NVS_TIMER";

static void periodic_timer_callback(void* arg) {
    ESP_LOGI(TAG, "Temporizador periódico disparado!");

    // 1️⃣ Inicializa NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Si la partición está corrupta o desactualizada, se borra y se reinicia
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // 2️⃣ Abre el espacio de nombres NVS (namespace)
    nvs_handle_t my_handle;
    err = nvs_open("almacen", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error al abrir NVS (%s)\n", esp_err_to_name(err));
        return;
    }

    // 3️⃣ Leer valor guardado previamente
    int32_t contador = 0;
    err = nvs_get_i32(my_handle, "contador", &contador);
    switch (err) {
        case ESP_OK:
            printf("Valor actual = %ld\n", contador);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("No hay valor previo, se inicializa.\n");
            contador = 0;
            break;
        default:
            printf("Error leyendo NVS (%s)\n", esp_err_to_name(err));
            nvs_close(my_handle);
            return;
    }

    // 4️⃣ Modificar y guardar
    contador++;
    err = nvs_set_i32(my_handle, "contador", contador);
    if (err != ESP_OK) {
        printf("Error escribiendo NVS (%s)\n", esp_err_to_name(err));
        nvs_close(my_handle);
        return;
    }

    // 5️⃣ Confirmar escritura (commit)
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        printf("Error al confirmar NVS (%s)\n", esp_err_to_name(err));
    }

    printf("Nuevo valor guardado: %ld\n", contador);

    // 6️⃣ Cerrar
    nvs_close(my_handle);
}

void app_main() {

    esp_timer_handle_t periodic_timer;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        .name = "periodic_task"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));


    // Iniciar el temporizador para que se ejecute cada 1,000,000 microsegundos (1 segundo)
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1 * 1000000));
}