#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "wifi/wifi.h"

#define LED_GPIO 2

void app_main(void)
{
    wifi_init_sta();

    gpio_set_direction(LED_GPIO,
                       GPIO_MODE_OUTPUT);

    int level = 0;

    while (1)
    {
        level = !level;
        gpio_set_level(LED_GPIO, level);

        printf("LED toggle: %d\n", level);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
