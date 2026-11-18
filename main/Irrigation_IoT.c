#include "Controller.h"
#include "Wifi.h"
void app_main(void)
{
    sht30_t sht30 = {
        .i2c_port = SHT30_I2C_PORT,
        .i2c_addr = SHT30_ADDR
    };
    sht30_init(&sht30);
    float temp, humi;
    while (1)
    {
        esp_err_t ret = sht30_read(&sht30, &temp, &humi, SHT3X_CMD_MEAS_HIGH_CS);

        if (ret == ESP_OK)
        {
            printf("Temperature: %.2f C | Humidity: %.2f %%\n", temp, humi);
        }
        else
        {
            printf("SHT30 read failed: %s\n", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(2000));   // delay 2 giây
    }


    esp_err_t err_bh1750 = bh1750_init(Addr_LOW, Continuous_H);
    if (err_bh1750 != ESP_OK) {
        ESP_LOGE("BH1750FVI", "Failed to initialize BH1750FVI: %s", esp_err_to_name(err_bh1750));
        return;
    }
    else {
        ESP_LOGI("BH1750FVI", "BH1750FVI initialized successfully");
    }
    while (1) {
        float lux = bh1750_read_light();
        lux = ema_filter(lux);
        if (lux >= 0.0f) {
            ESP_LOGI("BH1750FVI", "Light Level: %.2f lux", lux);
        }
        else {
            ESP_LOGE("BH1750FVI", "Failed to read light level");
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2 seconds before next reading
    }


    wifi_init_sta();
    vTaskDelay(pdMS_TO_TICKS(3000));
    if(wifi_is_connected()){
        ESP_LOGI("Wifi", "Wifi is connected!");
    }
    else   
         ESP_LOGI("Wifi", "Wifi connection is failed!");
    mqtt_app_start();
    vTaskDelay(pdMS_TO_TICKS(3000));
    char content[128];
    subscribe_single_topic("Control_system");

    float temperature = 28.5f;
    float light = 1000.0f;
    float humidity_air = 50.0f;
    float humidity_soil = 50.0f;
    snprintf(content, sizeof(content),
            "{\"Temperature\":%.2f,\"Light\":%.2f,\"Humidity_air\":%.2f, \"Humidity_soil\":%.2f}",
            temperature, light, humidity_air, humidity_soil);

    publish("Data_sensor", content);
    ESP_LOGI("mqtt", "published: %s", content);

    SEN0193_t SEN019;
    int sen019_raw_data;
    float sen019_data;
    SEN0193_Init(&SEN019, SEN0193_ADC_UNIT, SEN0193_CHANNEL);
    SEN0193_Calibration(&SEN019, 0);
    LOGI("SEN0193", "Has adjusted dry point of SEN0193 sensor...");
    vTaskDelay(pdMS_TO_TICKS(10000));
    SEN0193_Calibration(&SEN019, 1);
    LOGI("SEN0193", "Has adjusted wet point of SEN0193 sensor...");
    while (1)
    {
        sen019_raw_data = SEN0193_Read_Raw(&SEN019);
        sen019_data = SEN0193_Read_Percentage(&SEN019);
        LOGI("SEN0193", "Soil Moisture: Raw: %d, Percentage: %.2f%%", sen019_raw_data, sen019_data);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }



    SensorEntities_t sensors;
    SensorData_t data;
    sensors.sht30 = {
        .i2c_port = SHT30_I2C_PORT,
        .i2c_addr = SHT30_ADDR
    };
    esp_err_t err = controller_init(&sensors);
    if (err != ESP_OK) {
        ESP_LOGE("Controller", "Failed to initialize controller: %s", esp_err_to_name(err));
    }
    else {
        ESP_LOGI("Controller", "All sensors initialized successfully");
    }
    wifi_init_sta();
    vTaskDelay(pdMS_TO_TICKS(3000));
    mqtt_app_start();
    vTaskDelay(pdMS_TO_TICKS(3000));
    while (1) {
        read_sensors(&sensors, &data);
        publish_data(&data, "Data_sensor");
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    } 
}
