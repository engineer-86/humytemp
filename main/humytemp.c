#include <stdio.h>
#include <assert.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_err.h"
#include "esp_log.h"

#include "i2cdev.h"
#include "aht.h"


#include "wifi/wifi.h"

#define I2C_PORT I2C_NUM_0
#define I2C_SDA  21
#define I2C_SCL  22
#define I2C_FREQ 400000

static const char *TAG = "humytemp";

typedef struct {
    float t;
    float rh;
    esp_err_t err;
} aht_sample_t;

static QueueHandle_t s_aht_q;
static aht_t s_aht;


static void i2c_init(void)
{
    ESP_ERROR_CHECK(i2cdev_init());
    ESP_LOGI(TAG, "i2cdev initialized (port=%d SDA=%d SCL=%d freq=%d)",
             (int)I2C_PORT, I2C_SDA, I2C_SCL, I2C_FREQ);
}

static void aht_read_task(void *arg)
{
    (void)arg;

    while (1) {
        aht_sample_t s = {.t = 0.0f, .rh = 0.0f, .err = ESP_FAIL};

        s.err = aht_get_data(&s_aht, &s.t, &s.rh);

        // nicht blockieren; wenn Queue voll -> drop
        (void)xQueueSend(s_aht_q, &s, 0);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void bt_scan_task(void *arg)
{
    
}


static void aht_log_task(void *arg)
{
    (void)arg;

    aht_sample_t s;
    while (1) {
        if (xQueueReceive(s_aht_q, &s, portMAX_DELAY) == pdTRUE) {
            if (s.err == ESP_OK) {
                ESP_LOGI(TAG, "AHT10: T=%.2f °C  RH=%.2f %%", s.t, s.rh);
            } else {
                ESP_LOGE(TAG, "AHT10 read failed: %s", esp_err_to_name(s.err));
            }
        }
    }
}

void app_main(void)
{
    i2c_init();
    wifi_init_sta();

    // AHT init (wie bei dir)
    s_aht = (aht_t){0};
    ESP_ERROR_CHECK(aht_init_desc(&s_aht, AHT_I2C_ADDRESS_GND, I2C_PORT, I2C_SDA, I2C_SCL));
    s_aht.i2c_dev.cfg.master.clk_speed = I2C_FREQ;
    ESP_ERROR_CHECK(aht_init(&s_aht));
    ESP_LOGI(TAG, "AHT10 initialized (0x38)");

    // Queue für Samples
    s_aht_q = xQueueCreate(8, sizeof(aht_sample_t));
    assert(s_aht_q);

    // 2 Tasks starten
    xTaskCreate(aht_read_task, "aht_read", 4096, NULL, 5, NULL);
    xTaskCreate(aht_log_task,  "aht_log",  4096, NULL, 4, NULL);
    

    // app_main kann zurückkehren (Tasks laufen weiter)
}
