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
 */

#ifndef SENSORDATA_H
#define SENSORDATA_H


typedef struct
{
    int soilHumidity;
    int percentage;
    int visible;
    int IR;
    float UV;
    float airHumidity;
    float temperature;
} sensorData;

#endif