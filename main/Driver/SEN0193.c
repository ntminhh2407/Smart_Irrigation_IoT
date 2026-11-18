#include "SEN0193.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* -------------------------------------------------------------------------- */
/* PRIVATE HELPER FUNCTIONS                          */
/* -------------------------------------------------------------------------- */

/**
 * @brief Samples the ADC multiple times and returns the arithmetic mean
 * * @param sensor Pointer to the sensor structure
 * @return Averaged ADC raw value
 */
static int get_averaged_raw_val(SEN0193_t *sensor)
{
    uint32_t sum = 0;
    int samples = 50;
    int delay_between_samples_ms = 50;

    for(int i = 0; i < samples; i++)
    {
        int val;
        adc_oneshot_read(sensor->handle, sensor->channel, &val);
        sum += val;
        vTaskDelay(pdMS_TO_TICKS(delay_between_samples_ms));
    }
    return (sum / samples);
}

/**
 * @brief Apply Exponential Moving Average (EMA) filter to raw data
 * * @param sensor Pointer to the sensor configuration structure
 * @param raw_val The latest raw ADC reading
 * @param alpha Smoothing factor (0.0 to 1.0). Lower values increase smoothing.
 */
void EMA_Filter(SEN0193_t *sensor, int raw, float alpha)
{
    if (sensor->is_first_read){
        sensor->filtered_val = raw;
        sensor->is_first_read = 0;
    }
    else {
        sensor->filtered_val = (0.2*raw) + (1.0 - 0.2)*sensor->filtered_val;
    }
}

/* -------------------------------------------------------------------------- */
/* PUBLIC API FUNCTIONS                            */
/* -------------------------------------------------------------------------- */

esp_err_t SEN0193_Init (SEN0193_t *sensor, adc_unit_t unit, adc_channel_t channel)
{
    sensor->channel = channel;
    sensor->air_value = 4095; //Default
    sensor->water_value = 0; //Default
    sensor->is_first_read = 1;
    sensor->filtering_mode = SEN0193_FILTER_EMA;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &sensor->handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12
    };
    return adc_oneshot_config_channel(sensor->handle, channel, &config);
}

void SEN0193_Calibration(SEN0193_t* sensor, int flag)
{
    if (flag == 0)
    {
        sensor->air_value = get_averaged_raw_val(sensor);
    }
    else if (flag == 1)
    {
        sensor->water_value = get_averaged_raw_val(sensor);
    }
}

int SEN0193_Read_Raw(SEN0193_t *sensor)
{
    int raw;
    adc_oneshot_read(sensor->handle, sensor->channel, &raw);

    switch(sensor->filtering_mode)
    {
        case SEN0193_FILTER_EMA:
            EMA_Filter(sensor, raw, 0.2);
            break;
        case SEN0193_FILTER_NONE:
        default:
            sensor->filtered_val = raw;
            break;
    }
    return sensor->filtered_val;
}

float SEN0193_Read_Percentage(SEN0193_t *sensor)
{
    int raw;
    adc_oneshot_read(sensor->handle, sensor->channel, &raw);
    switch(sensor->filtering_mode)
    {
        case SEN0193_FILTER_EMA:
            EMA_Filter(sensor, raw, 0.2);
            break;
        case SEN0193_FILTER_NONE:
        default:
            sensor->filtered_val = raw;
            break;
    }
    float percent;
    percent = 100.0 * (sensor->filtered_val - sensor->air_value) / (sensor->water_value - sensor->air_value);
    if (percent > 100.0) percent = 100.0;
    else if (percent < 0.0) percent = 0.0;
    return percent;    
}