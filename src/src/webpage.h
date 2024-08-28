/**
 * @file webpage.h
 * @brief: HTML code for the webpage
 *
 * @author: Rafael Dousse
 * @date 23.08.2024
 * @version 1.0
 */

#ifndef WEBPAGE_H
#define WEBPAGE_H

String generateWebpage(float temperature, float humidity, int soilMoisture1, int soilMoisture2) {
    String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Hello Plant</title>
</head>
<body>
    <h1>Hello, Plant!</h1>
    <p>Current Temperature: )rawliteral";
    webpage += String(temperature) + " &deg;C</p>";
    webpage += "<p>Current Humidity: " + String(humidity) + " %</p>";
    webpage += "<p>Soil Moisture 1: " + String(soilMoisture1) + " %</p>";
    webpage += "<p>Soil Moisture 2: " + String(soilMoisture2) + " %</p>";
    webpage += R"rawliteral(
</body>
</html>
)rawliteral";
    return webpage;
}


#endif