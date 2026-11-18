#ifndef CONTROLLER_H
#define CONTROLLER_H
#ifdef __cplusplus
extern "C"{
#endif
#include "BH1750FVI.h"
#include "SHT30.h"
#include "SEN0193.h"
#include "Mqtt.h"

typedef struct {
    // BH1750FVI, SHT30, and SEN0193 sensor configurations
    sht30_t sht30;
    SEN0193_t sen0193;
} SensorEntities_t;

typedef struct {
    float temperature;  // Temperature in Celsius
    float humidity;     // Relative Humidity in percentage
    float light;        // Light level in lux
    float soil_moisture;  // Soil moisture level as a percentage
} SensorData_t;

// typedef struct {
//     bool pump_on;
//     bool fan_on;
//     bool light_on;
// } ActuatorStates_t;

esp_err_t controller_init(SensorEntities_t *sensors);
void read_sensors(SensorEntities_t *sensors, SensorData_t *data);
void publish_data(SensorData_t *data, const char *topic);
// void subscribe_commands(const char *topic, ActuatorStates_t *actuators);
//void control_irrigation(SensorData_t *data);

}  
#endif
#endif