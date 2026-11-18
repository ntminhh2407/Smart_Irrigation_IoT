#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include <stdint.h>
#include "Pin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHT3X_CMD_MEAS_HIGH_CS      ((uint16_t)0x2C06)
#define SHT3X_CMD_MEAS_MEDIUM_CS    ((uint16_t)0x2C0D)
#define SHT3X_CMD_MEAS_LOW_CS       ((uint16_t)0x2C10)

/* ---- Clock Stretching DISABLED ---- */
#define SHT3X_CMD_MEAS_HIGH         ((uint16_t)0x2400)
#define SHT3X_CMD_MEAS_MEDIUM       ((uint16_t)0x240B)
#define SHT3X_CMD_MEAS_LOW          ((uint16_t)0x2416)

#define SHT30_ADDR  0x44 //Address of SHT30 

typedef struct {
    i2c_port_t i2c_port;
    uint8_t    i2c_addr;
} sht30_t;

typedef enum {
    SHT30_REPEAT_LOW,
    SHT30_REPEAT_MEDIUM,
    SHT30_REPEAT_HIGH
} sht30_repeat_t;
typedef enum {
    SHT30_MPS_0_5,
    SHT30_MPS_1,
    SHT30_MPS_2,
    SHT30_MPS_4,
    SHT30_MPS_10
} sht30_mps_t;

esp_err_t sht30_start_periodic(
    sht30_t *dev,
    sht30_mps_t mps,
    sht30_repeat_t repeat
);




esp_err_t sht30_init(sht30_t *dev);
esp_err_t sht30_start_periodic(
    sht30_t *dev,
    sht30_mps_t mps,
    sht30_repeat_t repeat
);
esp_err_t sht30_read(sht30_t *dev, float *temperature, float *humidity, uint16_t cmd);
esp_err_t sht30_soft_reset(sht30_t *dev);
esp_err_t sht30_heater_enable(sht30_t *dev);
esp_err_t sht30_heater_disable(sht30_t *dev);
#ifdef __cplusplus
}
#endif