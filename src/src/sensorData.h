/**
 * Project Name: PlantKeeper
 *
 * @created 23.08.2024
 * @file  sensorData.h
 * @version 1.0.0
 * @see https://github.com/Plant-keeper
 *
 * @authors
 *   - Rafael Dousse
 *   - Eva Ray
 *   - Quentin Surdez
 *   - Rachel Tranchida
 * 
 * @brief Sensor data structure
 */

#ifndef SENSORDATA_H
#define SENSORDATA_H

/**
 * @brief Sensor data structure containing the sensor id, soil humidity, percentage humidity, light and temperature
 */
typedef struct
{
    int sensorId;
    int soilHumidity;
    int percentage;
    int light;
    float temperature;
} sensorData;

#endif