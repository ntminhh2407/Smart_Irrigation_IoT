#ifndef SEN0193_H
#define SEN0193_H

#ifdef __cplusplus
extern "C"{
#endif

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

/**
 * @brief Supported filtering modes for the sensor data
 */
typedef enum {
    SEN0193_FILTER_NONE = 0,        // No filtering applied
    SEN0193_FILTER_EMA,             // Exponential Moving Average filter
    SEN0193_FILTER_MEDIAN           // Median filter (Reserved for future use)
} SEN0193_filter_t;

/**
 * @brief Sensor handle structure containing configuration and state
 */
typedef struct 
{
    adc_oneshot_unit_handle_t handle;   // ADC unit handle
    adc_channel_t channel;              // ADC channel assigned to the sensor
    int air_value;                      // Calibrated ADC value when sensor is dry
    int water_value;                    // Calibrated ADC value when the sensor is wet
    int filtered_val;                   // ADC value when using the filter. If not using the filter, filtered_val = raw value 
    int is_first_read;                  // Flag to initialize filter on first execution
    SEN0193_filter_t filtering_mode;    // Currently active filtering algorithm
} SEN0193_t;

/**
 * @brief Initialize the ADC unit and channel for the soil moisture sensor
 * * @param sensor Pointer to the sensor configuration structure
 * @param unit ADC unit (e.g., ADC_UNIT_1)
 * @param channel ADC channel (e.g., ADC_CHANNEL_6)
 * @return esp_err_t ESP_OK on success, or error code from ADC driver
 */
esp_err_t SEN0193_Init(SEN0193_t *sensor, adc_unit_t unit, adc_channel_t channel);

/**
 * @brief Calibrate the sensor by sampling current environment conditions
 * * @param sensor Pointer to the sensor configuration structure
 * @param flag Calibration target: 0 for Dry Point (Air), 1 for Wet Point (Water)
 * @return None
 */
void SEN0193_Calibration(SEN0193_t *sensor, int flag);

/**
 * @brief Read and process the raw ADC value based on the selected filtering mode
 * * @param sensor Pointer to the sensor configuration structure
 * @return int The processed (filtered or raw) ADC value
 */
int SEN0193_Read_Raw(SEN0193_t *sensor);

/**
 * @brief Calculate the soil moisture level as a percentage
 * * @param sensor Pointer to the sensor configuration structure
 * @return float Moisture percentage ranging from 0.0% to 100.0%
 */
float SEN0193_Read_Percentage(SEN0193_t *sensor);

#ifdef __cplusplus
}
#endif

#endif