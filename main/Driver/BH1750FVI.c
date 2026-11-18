#include "BH1750FVI.h"

static const char *TAG = "BH1750FVI";
static i2c_master_bus_handle_t bh1750_bus_handle = NULL;
static i2c_master_dev_handle_t bh1750_dev_handle = NULL;

uint8_t bh1750_addr = Addr_LOW; // Default address  
uint8_t MTReg = 69; // Default measurement time register value  
float Sensitivity = 1.00f; // Default sensitivity scaling factor
float Resolution = 1.00f; // Default resolution scaling factor for Continuous_H mode
uint8_t bh1750_mode = Continuous_H; // Default mode
// i2c write function
static esp_err_t bh1750_write(uint8_t cmd) {
    if (bh1750_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE; // Device not initialized
    }
    return i2c_master_transmit(bh1750_dev_handle, &cmd, 1, -1);
}
// initializes the sensor with default settings
esp_err_t  bh1750_init(uint8_t Addr, uint8_t Mode){
    // configure the I2C bus 
    esp_err_t err;
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA,
        .scl_io_num = I2C_MASTER_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 6,
        .flags.enable_internal_pullup = true,
    };
    // Initialize the I2C bus
    err = i2c_new_master_bus(&bus_config, &bh1750_bus_handle);
    vTaskDelay(pdMS_TO_TICKS(100)); 
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(err));
        return err;
    }
    else {
        ESP_LOGI(TAG, "I2C bus initialized successfully");
    }
    // Set the sensor's I2C address and check err
    err = SetAddress(Addr);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set BH1750FVI address: %s", esp_err_to_name(err));
        return err;
    }
    else {
        ESP_LOGI(TAG, "BH1750FVI address set successfully");
    }
    // Power on the sensor and and check err
    err = PowerOn();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to power on BH1750FVI: %s", esp_err_to_name(err));
        return err;
    }
    else {
        ESP_LOGI(TAG, "BH1750FVI powered on successfully");
    }
    // Set the sensor mode and check err
    err = SetMode(Mode);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set BH1750FVI mode: %s", esp_err_to_name(err));
        return err;
    }
    else {
        ESP_LOGI(TAG, "BH1750FVI mode set successfully");
    }
    return ESP_OK;
}
// Power on the sensor
esp_err_t PowerOn(void){
    esp_err_t err = bh1750_write(Power_On);
    vTaskDelay(pdMS_TO_TICKS(10));
    return err;
}
// Sleep the sensor
esp_err_t Sleep(void){
    esp_err_t err = bh1750_write(Power_Down);
    vTaskDelay(pdMS_TO_TICKS(10));
    return err;
}
// Turn on and reset the sensor
esp_err_t  Reset(void){
    esp_err_t err = PowerOn();
    if (err != ESP_OK) return err;
    return bh1750_write(Reset_Sensor);
}
// Set the sensor's I2C address & turn on Addr pin
esp_err_t SetAddress(uint8_t Addr){
    bh1750_addr = Addr;
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = bh1750_addr,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    if (bh1750_dev_handle != NULL) {
        i2c_master_bus_rm_device(bh1750_dev_handle);
        bh1750_dev_handle = NULL;
    }
    esp_err_t err = i2c_master_bus_add_device(bh1750_bus_handle, &dev_config, &bh1750_dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add BH1750FVI device to I2C bus: %s", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}
// Sets the sensor mode
esp_err_t  SetMode(uint8_t Mode){
    switch(Mode){
        case Continuous_H:
            Resolution = 1.00f;
            break;
        case Continuous_H2:
            Resolution = 0.5f;
            break;
        case Continuous_L:
            Resolution = 4.00f;
            break;
        case OneTime_H:
            Resolution = 1.00f;
            break;
        case OneTime_H2:
            Resolution = 0.5f;
            break;
        case OneTime_L:
            Resolution = 4.00f;
            break;
        default:
            ESP_LOGW(TAG, "Invalid mode selected, defaulting to Continuous_H");
            Mode = Continuous_H;
            Resolution = 1.00f;
    }
    bh1750_mode = Mode;
    esp_err_t err = bh1750_write(Mode);
    vTaskDelay(pdMS_TO_TICKS(10));
    return ESP_OK;
}
// Sets the sensor's MTReg value
void SetMTReg(uint8_t MTRegValue){
    // MTReg must be between 31 and 254 according to datasheet
    MTReg = (MTRegValue < 31) ? 31 : (MTRegValue > 254) ? 254 : MTRegValue;
    // Shift the first 3 bytes of MT to the last 3
    // Then OR 64 (01000000) to add the 01000 prefix
    uint8_t highbyte = (MTReg >> 5) | 64;
    // Get rid of the first 3 bytes in MT by ANDing 31 (00011111)
    // Then OR 96 (01100000) to add the 011 prefix
    uint8_t lowbyte = (MTReg & 31) | 96;
    bh1750_write(highbyte);
    bh1750_write(lowbyte);
    vTaskDelay(pdMS_TO_TICKS(10));
}
// Sets the sensor's sensitivity scaling factor
void SetSensitivity(float SensitivityValue){
    // Convert sensitivity ratio to MT
    int MTValue = (int)(SensitivityValue * 69.0f);
    // constrain MT to [31, 254]
    MTValue = (MTValue < 31) ? 31 : (MTValue > 254) ? 254 : MTValue;
    Sensitivity = MTValue / 69.0;
    // Set the Measurement Time to MT
    SetMTReg(MTValue);
}
// Reads the light level in lux
float bh1750_read_light(void){
    float lux = 0.0f;
    if (bh1750_dev_handle == NULL) {
        ESP_LOGE(TAG, "Device not initialized");
        return -1.0f; // Indicate error
    }

    esp_err_t err;

    if (bh1750_mode == OneTime_H || bh1750_mode == OneTime_H2 || bh1750_mode == OneTime_L) {
        err = bh1750_write(bh1750_mode);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to trigger one-time measurement: %s", esp_err_to_name(err));
            return -1.0f; // Indicate error
        }
    }

    if (bh1750_mode == Continuous_L || bh1750_mode == OneTime_L) {
        vTaskDelay(pdMS_TO_TICKS(18));
    }
    else {
        vTaskDelay(pdMS_TO_TICKS(150));
    }

    uint8_t data[2];
    err = i2c_master_receive(bh1750_dev_handle, data, 2, -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from BH1750FVI: %s", esp_err_to_name(err));
        return -1.0f; // Indicate error
    }

    uint16_t raw_value = ((uint16_t)data[0] << 8) | data[1];

    lux = ((float)raw_value / 1.2f) * (69.0f / (float)MTReg);

    if (bh1750_mode == Continuous_H2 || bh1750_mode == OneTime_H2) {
        lux /= 2.0f;
    }
    // Return a float.  Only to 2 significant digits though
    return lux;
}