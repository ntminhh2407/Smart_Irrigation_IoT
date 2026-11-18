#ifndef BH1750FVI_H
#define BH1750FVI_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "Pin.h"

// I2C master configuration
#define I2C_MASTER_SDA BH175FVI_SDA
#define I2C_MASTER_SCL BH175FVI_SCL
#define I2C_MASTER_NUM BH175FVI_I2C_PORT    
#define I2C_MASTER_FREQ_HZ 100000   
// I2C device addresses
#define Addr_LOW 0x23 // Device address when address pin LOW
#define Addr_HIGH 0x5C // Device address when address pin High

// Sensor Commands
#define Power_Down 0x00
#define Power_On 0x01
#define Reset_Sensor 0x07

// Measurement Modes
#define Continuous_H  0x10 // 1 lx resolution (16 bit), 120ms sampling time
#define Continuous_H2  0x11 // 0.5 lx resolution, 120ms sampling time
#define Continuous_L  0x13 // 4 lx resolution (15 bit), 16ms sampling time
#define OneTime_H  0x20
#define OneTime_H2  0x21
#define OneTime_L  0x23 //As well as address value

// the i2c address variable, either 0x23 or 0x5C 
extern uint8_t bh1750_addr;
// the measurement time register variable 
extern uint8_t MTReg;
// The sensitivity scaling factor
extern float Sensitivity;
// The resolution scaling factor: 0.5 for H2, 1 for H, and 4 for L
extern float Resolution;
// the current sensor mode
extern uint8_t bh1750_mode;
// initializes the sensor with default settings
esp_err_t  bh1750_init(uint8_t Addr, uint8_t Mode);
// Reads the light level in lux
float bh1750_read_light(void);
// Power on the sensor
esp_err_t PowerOn(void); 
// Sleep the sensor
esp_err_t Sleep(void); 
// Turn on and reset the sensor
esp_err_t Reset(void);
// Set the sensor's I2C address & turn on Addr pin
esp_err_t SetAddress(uint8_t Addr);
// Sets the sensor mode
esp_err_t SetMode(uint8_t Mode);
void SetMTReg(uint8_t MTRegValue);
void SetSensitivity(float SensitivityValue);

#endif