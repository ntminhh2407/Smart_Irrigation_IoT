#include "SHT30.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SHT30_CMD_SOFT_RESET        0x30A2
#define SHT30_MEASURE_DELAY_MS     20

#define SHT30_CMD_HEATER_ENABLE    ((uint16_t)0x306D)
#define SHT30_CMD_HEATER_DISABLE   ((uint16_t)0x3066)

#define SHT30_CMD_READ_STATUS 0xF32D


static const uint16_t sht30_periodic_cmd[5][3] = {
    //  LOW      MEDIUM    HIGH
    {0x202F,  0x2024,  0x2032},  // 0.5 mps
    {0x212D,  0x2126,  0x2130},  // 1 mps
    {0x222B,  0x2220,  0x2236},  // 2 mps
    {0x2329,  0x2322,  0x2334},  // 4 mps
    {0x272A,  0x2721,  0x2737}   // 10 mps
};

static esp_err_t sht30_write_cmd(sht30_t *dev, uint16_t cmd)
{
    if (!dev) return ESP_ERR_INVALID_ARG;

    uint8_t data[2] = {
        (uint8_t)(cmd >> 8),
        (uint8_t)(cmd & 0xFF)
    };

    return i2c_master_write_to_device(
        dev->i2c_port,
        dev->i2c_addr,
        data,
        sizeof(data),
        pdMS_TO_TICKS(100)
    );
}

esp_err_t sht30_init(sht30_t *dev)
{   
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SHT30_SDA,
        .scl_io_num = SHT30_SCL,
        .master.clk_speed = 100000
    };

    ESP_ERROR_CHECK(i2c_param_config(SHT30_I2C_PORT, &cfg));
    ESP_ERROR_CHECK(i2c_driver_install(SHT30_I2C_PORT, cfg.mode, 0, 0, 0));

    if (!dev)
        return ESP_ERR_INVALID_ARG;

    esp_err_t err;

    /* Reset sensor */
    err = sht30_soft_reset(dev);
    if (err != ESP_OK)
        return err;

    /* Check sensor presence */
    err = sht30_write_cmd(dev, SHT30_CMD_READ_STATUS);
    if (err != ESP_OK)
        return err;

    uint8_t status[3];
    err = i2c_master_read_from_device(
        dev->i2c_port,
        dev->i2c_addr,
        status,
        sizeof(status),
        pdMS_TO_TICKS(100)
    );

    return err;
}

esp_err_t sht30_start_periodic(
    sht30_t *dev,
    sht30_mps_t mps,
    sht30_repeat_t repeat
)
{
    if (!dev || mps > SHT30_MPS_10 || repeat > SHT30_REPEAT_HIGH)
        return ESP_ERR_INVALID_ARG;

    uint16_t cmd = sht30_periodic_cmd[mps][repeat];
    return sht30_write_cmd(dev, cmd);
}

esp_err_t sht30_read(
    sht30_t *dev,
    float *temperature,
    float *humidity,
    uint16_t cmd
)
{
    if (!dev || !temperature || !humidity)
        return ESP_ERR_INVALID_ARG;

    esp_err_t err;

    /* 1. Send measurement command */
    err = sht30_write_cmd(dev, cmd);
    if (err != ESP_OK)
        return err;

    /* 2. Delay (datasheet: max ~15ms for high repeatability)
       - Clock stretching enabled: delay vẫn OK
       - Clock stretching disabled: bắt buộc delay */
    vTaskDelay(pdMS_TO_TICKS(SHT30_MEASURE_DELAY_MS));

    /* 3. Read measurement result (6 bytes) */
    uint8_t rx[6];
    err = i2c_master_read_from_device(
        dev->i2c_port,
        dev->i2c_addr,
        rx,
        sizeof(rx),
        pdMS_TO_TICKS(100)
    );
    if (err != ESP_OK)
        return err;

    /* 4. Combine raw data */
    uint16_t raw_temp = ((uint16_t)rx[0] << 8) | rx[1];
    uint16_t raw_hum  = ((uint16_t)rx[3] << 8) | rx[4];

    /* 5. Convert to physical values (datasheet formula) */
    *temperature = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);
    *humidity    = 100.0f * ((float)raw_hum  / 65535.0f);

    return ESP_OK;
}

esp_err_t sht30_soft_reset(sht30_t *dev)
{
    if (!dev)
        return ESP_ERR_INVALID_ARG;

    esp_err_t err;

    /* 1. Send soft reset command */
    err = sht30_write_cmd(dev, SHT30_CMD_SOFT_RESET);
    if (err != ESP_OK)
        return err;

    /* 2. Wait for reset completion (datasheet ≥ 10 ms) */
    vTaskDelay(pdMS_TO_TICKS(10));

    return ESP_OK;
}

esp_err_t sht30_heater_enable(sht30_t *dev)
{
    if (!dev)
        return ESP_ERR_INVALID_ARG;

    return sht30_write_cmd(dev, SHT30_CMD_HEATER_ENABLE);
}

esp_err_t sht30_heater_disable(sht30_t *dev)
{
    if (!dev)
        return ESP_ERR_INVALID_ARG;

    return sht30_write_cmd(dev, SHT30_CMD_HEATER_DISABLE);
}