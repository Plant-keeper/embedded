/**
 * Project Name: PlantKeeper
 *
 * @created 23.08.2024
 * @file  main.cpp
 * @version 1.0.0
 * @see https://github.com/Plant-keeper
 *
 * @authors
 *   - Rafael Dousse
 *   - Eva Ray
 *   - Quentin Surdez
 *   - Rachel Tranchida
 *
 * @brief Main file of the project. The arduino will first start in access point mode to configure the wifi.
 *        Once the wifi is configured, the arduino will connect to the wifi and send the sensor data to the distant server.
 */

#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpServer.h>
#include <ArduinoHttpClient.h>
#include <vector>
#include "arduino_secrets.h"
#include "webpages.h"
#include "DHT.h"
#include "ArduinoJson.h"
#include "SI114X.h"

// Function prototypes
void listNetworks(std::vector<const char *> &networks);
void startAccessPoint();
void handleConfigRequest();
void connectToWiFi();
void printWEB();
void readSensors();
void printWifiStatus();
void sendSensorData();

// Global variables
char ssid[32];
char pass[64];
bool needsWiFiConfig = true;
bool connectedToWiFi = false;
bool incorrectPassword = false;
const int dry = 1023;
const int wet = 700;
const int minLight = 160;
const int maxLight = 800;

// Constants for the access point mode
const char *ssidArduino = SECRET_SSID;
const char *passArduino = SECRET_PASS;

int status = WL_IDLE_STATUS;

unsigned long previousMillis = 0;
// 10 seconds interval for the data to be sent
const long interval = 10000;

unsigned long previousMillisLed = 0;
// 1 seconds interval for led to blink
const long intervalLed = 1000;

// Instantiation of objects
std::vector<const char *> networks;
sensorData sensorDatas;
WiFiServer server(80);
WiFiClient client;
DHT dht(2, DHT11);
JsonDocument doc;
SI114X SI1145 = SI114X();

//--------------------------------------------IMPORTANT--------------------------------------------
// SERVER'S ADRESS IP NEEDS TO BE ADDED HERE
IPAddress serverAddress(178, 192, 219, 78);
// SERVER'S PORT NEEDS TO BE ADDED HERE
int port = 8080;

HttpClient httpClient(client, serverAddress, port);
//--------------------------------------------------------------------------------------------------

// Arduino's Ip should be always the same, it might change but the arduino usually has the same IP in access point mode
String ipArduino = "http://192.168.4.1";
// Wifi's Ip will change depending on the network
String ipWifi = "http://192.168.183.146";

void setup()
{
    Serial.begin(9600);
    dht.begin();
    while (!SI1145.Begin())
    {
        Serial.println("Si1145 is not ready!");
        delay(1000);
    }
    pinMode(LED_BUILTIN, OUTPUT);

    listNetworks(networks);
    startAccessPoint();
}

void loop()
{

    if (needsWiFiConfig)
    {
        client = server.available();
        if (client)
        {
            handleConfigRequest();
        }
    }
    else
    {
        client = server.available();
        if (client)
        {
            printWEB();
        }
    }

    if (connectedToWiFi)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            sendSensorData();
        }
        if (currentMillis - previousMillisLed >= intervalLed)
        {
            previousMillisLed = currentMillis;
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        }
    }
}

//--------------------------------------------WEB SERVER FUNCTIONS--------------------------------------------

/**
 * @brief List the nearby networks
 * @param network Vector to store the networks
 **/
void listNetworks(std::vector<const char *> &network)
{
    // scan for nearby networks:
    Serial.println("Scanning for networks...");

    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
        Serial.println("Couldn't get a WiFi connection");
        network.push_back("No networks found");
        return;
    }

    for (int thisNet = 0; thisNet < numSsid; thisNet++)
    {
        network.push_back(WiFi.SSID(thisNet));
    }
}

/**
 * @brief Start the Arduino in Access Point mode
 */
void startAccessPoint()
{
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Starting Access Point...");
    WiFi.end();

    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        return;
    }

    status = WiFi.beginAP(ssidArduino, passArduino);

    if (status != WL_AP_LISTENING)
    {
        Serial.println("Creating access point failed");
        return;
    }

    IPAddress ip = WiFi.localIP();
    Serial.print("AP IP Address: ");
    Serial.println(ip);

    delay(2000);
    server.begin();
}

/**
 * @brief Handle the configuration request and extract SSID and Password
 */
void handleConfigRequest()
{
    Serial.println("Handling Config Request...");
    String header = "";
    String ssidParam = "";
    String passParam = "";
    String sensorParam = "";

    while (client.connected())
    {
        if (client.available())
        {
            char c = client.read();
            if (c == '\n')
            {
                if (header.length() == 0)
                {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println("Connection: close");
                    client.println();
                    client.println(generateConfigPage(networks, incorrectPassword));
                    client.println();
                    break;
                }
                else if (header.indexOf("POST /submit") >= 0)
                {
                    String postBody = "";
                    while (client.available())
                    {
                        postBody += (char)client.read();
                    }

                    // Extract SSID and password from the POST body
                    int ssidIndex = postBody.indexOf("ssid=") + 5;
                    int passIndex = postBody.indexOf("pass=") + 5;
                    int idIndex = postBody.indexOf("idsensor=") + 9;
                    ssidParam = postBody.substring(ssidIndex, postBody.indexOf('&', ssidIndex));
                    passParam = postBody.substring(passIndex, postBody.indexOf('&', passIndex)); // Assuming password is the last field
                    sensorParam = postBody.substring(idIndex);

                    ssidParam.toCharArray(ssid, 32);
                    passParam.toCharArray(pass, 64);
                    sensorDatas.sensorId = sensorParam.toInt();
                    Serial.println("SSID is : ");
                    Serial.println(ssid);
                    Serial.println("Password is : ");
                    Serial.println(pass);
                    Serial.println("SensorId is : ");
                    Serial.println(sensorDatas.sensorId);

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println("Connection: close");
                    client.println();
                    client.println(generateConnectingPage(ipWifi));
                    client.println();
                    delay(1000);

                    break;
                }
                else
                {
                    header = "";
                }
            }
            else if (c != '\r')
            {
                header += c;
            }
        }
    }
    if (ssidParam.length() > 0 && passParam.length() > 0 && sensorParam.length() > 0)
    {
        needsWiFiConfig = false;
        connectToWiFi();
    }
    client.flush();
    client.stop();
}

/**
 * @brief Function to connect to the Wi-Fi network using the credentials provided by the user.
 */
void connectToWiFi()
{
    // The counter is to make sure we cannot connect to the wifi since sometimes
    // the connection is not established the first time but the second time it works
    int counter = 0;
    WiFi.end();

    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        status = WiFi.begin(ssid, pass);
        printWifiStatus();
        delay(1000);

        if ((WiFi.status() == WL_CONNECT_FAILED) || (WiFi.status() == WL_DISCONNECTED))
        {
            if (counter == 5)
            {
                Serial.println("Connection failed!");
                printWifiStatus();
                incorrectPassword = true;
                needsWiFiConfig = true;
                ssid[0] = '\0';
                pass[0] = '\0';
                startAccessPoint();
                return;
            }
            counter++;
        }
    }
    printWifiStatus();

    Serial.println("Connected to WiFi!");
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    server.begin();
    connectedToWiFi = true;
    digitalWrite(LED_BUILTIN, LOW);

    delay(500);
}

/**
 * @brief Prints the web page with the sensor values
 */
void printWEB()
{
    client = server.available();
    if (client)
    {
        // Read the sensors values to display them
        readSensors();
        String currentLine = "";
        while (client.connected())
        {

            if (client.available())
            {

                char c = client.read();

                if (c == '\n')
                {

                    if (currentLine.length() == 0)
                    {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        client.println(generateDataPage(ipArduino, sensorDatas));
                        client.println();

                        break;
                    }
                    else if (currentLine.indexOf("POST /reconfigure") >= 0)
                    {
                        Serial.println("Reconfiguring wifi");
                        needsWiFiConfig = true;
                        connectedToWiFi = false;
                        incorrectPassword = false;
                        ssid[0] = '\0';
                        pass[0] = '\0';
                        client.flush();
                        client.stop();
                        startAccessPoint();
                        return;
                    }
                    else
                    {
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {
                    currentLine += c;
                }
            }
        }
    }
    client.flush();
    client.stop();
}

/**
 * @brief Send the sensor data to the distant server
 */
void sendSensorData()
{
    int statusCode = 0;

    readSensors();
    String jsonData;
    serializeJson(doc, jsonData);

    httpClient.setHttpResponseTimeout(2000);

    // Send HTTP POST request
    httpClient.beginRequest();
    httpClient.post("/sensor-data");
    httpClient.sendHeader("Content-Type", "application/json");
    httpClient.sendHeader("Content-Length", String(jsonData.length()));
    httpClient.sendHeader("Accept", "*/*");
    httpClient.beginBody();
    httpClient.print(jsonData);
    httpClient.endRequest();
    Serial.println("Request sent!");

    // Get HTTP response
    statusCode = httpClient.responseStatusCode();
    String response = httpClient.responseBody();
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
}

//--------------------------------------------Other functions--------------------------------------------

/**
 * @brief Read the sensors values
 */
void readSensors()
{
    int tmpLight;

    sensorDatas.soilHumidity = analogRead(0);
    sensorDatas.temperature = dht.readTemperature();
    tmpLight = SI1145.ReadVisible();

    // Map sensor values to have a percentage
    sensorDatas.percentage = map(sensorDatas.soilHumidity, dry, wet, 0, 100);

    // Read the light sensor, using a threshold to avoid jumping values that can occures and
    // gives too high or too low values
    if (tmpLight >= minLight && tmpLight <= maxLight)
    {
        sensorDatas.light = map(tmpLight, minLight, maxLight, 0, 2000);
    }
    else if (tmpLight < minLight)
    {
        sensorDatas.light = 0;
    }
    else if (tmpLight > maxLight)
    {
        sensorDatas.light = 2000;
    }

    // Create JSON object
    doc["id"] = sensorDatas.sensorId;
    doc["temperature"] = sensorDatas.temperature;
    doc["humidity"] = sensorDatas.soilHumidity;
    doc["humidityPercentage"] = sensorDatas.percentage;
    doc["light"] = sensorDatas.light;
}

/**
 * @brief Function to print the wifi status
 */
void printWifiStatus()
{
    wl_status_t status = (wl_status_t)WiFi.status();
    Serial.print("WiFi status: ");
    switch (status)
    {
    case WL_NO_SHIELD:
        Serial.println("No shield is present");
        break;
    case WL_IDLE_STATUS:
        Serial.println("Idle status");
        break;
    case WL_NO_SSID_AVAIL:
        Serial.println("No SSID available");
        break;
    case WL_SCAN_COMPLETED:
        Serial.println("Scan completed");
        break;
    case WL_CONNECTED:
        Serial.println("Connected");
        break;
    case WL_CONNECT_FAILED:
        Serial.println("Connection failed");
        break;
    case WL_CONNECTION_LOST:
        Serial.println("Connection lost");
        break;
    case WL_DISCONNECTED:
        Serial.println("Disconnected");
        break;
    case WL_AP_LISTENING:
        Serial.println("AP listening");
        break;
    case WL_AP_CONNECTED:
        Serial.println("AP connected");
        break;
    case WL_AP_FAILED:
        Serial.println("AP failed");
        break;
    default:
        Serial.println("Unknown status");
        break;
    }
}