#include "Controller.h"

static float ema_filter(float new_value)
{
    static bool initialized = false;
    static float filtered = 0.0f;
    const float alpha = 0.3f;
    if (!initialized) {
        filtered = new_value;
        initialized = true;
    } else {
        filtered = filtered + alpha * (new_value - filtered);
    }
    return filtered;
}

esp_err_t controller_init(SensorEntities_t *sensors)
{
    esp_err_t err;
    // Initialize BH1750FVI sensor
    err = bh1750_init(Addr_LOW, Continuous_H);
    if (err != ESP_OK) {
        ESP_LOGE("Controller", "Failed to initialize BH1750FVI: %s", esp_err_to_name(err));
    }
    else {
        ESP_LOGI("Controller", "BH1750FVI initialized successfully");
    }
    // Initialize SHT30 sensor
    err = sht30_init(&sensors->sht30);
    if (err != ESP_OK) {
        ESP_LOGE("Controller", "Failed to initialize SHT30: %s", esp_err_to_name(err));
    }
    else {
        ESP_LOGI("Controller", "SHT30 initialized successfully");
    }
    // Initialize SEN0193 sensor
    err = SEN0193_Init(&sensors->sen0193, SEN0193_ADC_UNIT, SEN0193_CHANNEL);
    if (err != ESP_OK) {
        ESP_LOGE("Controller", "Failed to initialize SEN0193: %s", esp_err_to_name(err));
    }
    else {
        ESP_LOGI("Controller", "SEN0193 initialized successfully");
    }
    SEN0193_Calibrate(&sensors->sen0193, 0);
    ESP_LOGI("Controller", "Calibration completed, waiting for 10 seconds for get dry point of SEN0193...");
    vTaskDelay(pdMS_TO_TICKS(10000));
    SEN0193_Calibrate(&sensors->sen0193, 1);
    return ESP_OK;

}
void read_sensors(SensorEntities_t *sensors, SensorData_t *data)
{
    // Read BH1750FVI sensor
    data->light = bh1750_read_light();
    vTaskDelay(pdMS_TO_TICKS(100));  
    data->light = ema_filter(data->light);
    // Read SHT30 sensor
    sht30_read(&sensors->sht30, &data->temperature, &data->humidity, SHT3X_CMD_MEAS_HIGH_CS);
    vTaskDelay(pdMS_TO_TICKS(100));
    data->temperature = ema_filter(data->temperature);
    data->humidity = ema_filter(data->humidity);
    // Read SEN0193 sensor
    data->soil_moisture = SEN0193_Read_Percentage(&sensors->sen0193);
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI("Controller", "Sensor readings - Temperature: %.2f °C, Humidity: %.2f %%, Light: %.2f lux, Soil Moisture: %.2f %%", 
             data->temperature, data->humidity, data->light, data->soil_moisture);
}

void publish_data(SensorData_t *data, const char *topic)
{
    char content[128];
    snprintf(content, sizeof(content),
             "{\"Temperature\":%.2f,\"Light\":%.2f,\"Humidity\":%.2f, \"Soil_Moisture\":%.2f}",
             data->temperature, data->light, data->humidity, data->soil_moisture);
    publish(topic, content);
    ESP_LOGI("Controller", "Published data: %s", content);
}