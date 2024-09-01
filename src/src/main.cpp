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
 */

#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpServer.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"
#include <vector>
#include "webpages.h"
#include "DHT.h"
#include "ArduinoJson.h"
#include "SI114X.h"

// To check how it works, if time allows
/* #include <WiFiUdp.h>
#include <ArduinoMDNS.h> */

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

const char *ssidArduino = "Arduino_Config";
const char *passArduino = "password";

int status = WL_IDLE_STATUS;
std::vector<const char *> networks;

sensorData sensorDatas;

const int dry = 1023;
const int wet = 700;

// Instantiation of objects
WiFiServer server(80); // Server socket
WiFiClient client;
DHT dht(2, DHT11);
JsonDocument doc;
SI114X SI1145 = SI114X();

int port = 8080;
IPAddress serverAddress(192, 168, 183, 1);
HttpClient httpClient(client, serverAddress, port);

// It should be always the same, it might change but the arduino usually has the same IP in access point mode
String ipArduino = "192.168.4.1";

unsigned long previousMillis = 0;
const long interval = 10000;

typedef struct
{
    boolean existing;
    const char *ssid;
    const char *pass;
} wifi;

/* WiFiUDP udp;
MDNS mdns(udp);
 */
/* void nameFound(const char* name, IPAddress ip)
{
  if (ip != INADDR_NONE) {
    Serial.print("The IP address for '");
    Serial.print(name);
    Serial.print("' is ");
    Serial.println(ip);
  } else {
    Serial.print("Resolving '");
    Serial.print(name);
    Serial.println("' timed out.");
  }
} */

const char *ssidForTest = "MyArduino";
const char *passForTest = "password";

void setup()
{

    Serial.begin(9600);
    Serial.println("Starting...");

    while (!SI1145.Begin())
    {
        Serial.println("Si1145 is not ready!");
        delay(1000);
    }

    dht.begin();
    listNetworks(networks);
    if (WiFi.status() != WL_CONNECTED)
    {
        printWifiStatus();
        startAccessPoint(); // Start in AP mode to configure WiFi
    }
    else
    {
        connectToWiFi(); // Connect to WiFi if already configured
    }
}

void loop()
{
    /* const char *hostName = "MyArduino";
     int length = strlen(hostName);
       if (!mdns.isResolvingName()) {
         if (length > 0) {
           Serial.print("Resolving '");
           Serial.print(hostName);
           Serial.println("' via Multicast DNS (Bonjour)...");

           // Now we tell the mDNS library to resolve the host name. We give it a
           // timeout of 5 seconds (e.g. 5000 milliseconds) to find an answer. The
           // library will automatically resend the query every second until it
           // either receives an answer or your timeout is reached - In either case,
           // the callback function you specified in setup() will be called.

           mdns.resolveName(hostName, 5000);
         }
       }
        mdns.run(); */

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
            Serial.println("Client available... so printweb");
            printWEB();
        }
     } 

    /*     if (connectedToWiFi)
        {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= interval)
            {
                Serial.println("Sending sensor data...");
                previousMillis = currentMillis;
                sendSensorData();
            }
        } */
}

//--------------------------------------------WEB SERVER FUNCTIONS--------------------------------------------

/**
 * @brief List the nearby networks
 **/
void listNetworks(std::vector<const char *> &network)
{
    // scan for nearby networks:
    Serial.println("** Scan Networks **");

    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
        Serial.println("Couldn't get a WiFi connection");
        while (true)
            ;
    }

    // print the list of networks seen:
    Serial.print("number of available networks:");
    Serial.println(numSsid);

    // print the network number and name for each network found:
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
    Serial.println("Starting Access Point...");
    WiFi.end();

    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");

        while (true)
            ;
    }

    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    {
        Serial.println("Please upgrade the firmware");
    }

    int status = WL_IDLE_STATUS;

    status = WiFi.beginAP(ssidArduino, passArduino);

    if (status != WL_AP_LISTENING)
    {
        Serial.println("Creating access point failed");

        while (true)
            ;
    }
    /*
        mdns.begin(WiFi.localIP(), "arduino");

      // We specify the function that the mDNS library will call when it
      // resolves a host name. In this case, we will call the function named
      // "nameFound".
      mdns.setNameResolvedCallback(nameFound); */

    IPAddress ip = WiFi.localIP();
    Serial.print("AP IP Address: ");
    Serial.println(ip);

    delay(5000);
    server.begin();
}

/**
 * @brief Hhandle the configuration request and extract SSID and Password
 */
void handleConfigRequest()
{
    Serial.println("Handling Config Request...");
    String header = "";
    String ssid_param = "";
    String pass_param = "";
    String tmpRequest = "";

    WiFiClient client = server.available();

    // listNetworks(networks);
    while (client.connected())
    {

        if (client.available())
        {
            char c = client.read();
            if (c == '\n')
            {
                if (header.indexOf("GET /get?ssid=") >= 0)
                {
                    Serial.println("SSID and Password received!");
                    int ssidIndex = header.indexOf("ssid=") + 5;
                    int passIndex = header.indexOf("pass=") + 5;
                    ssid_param = header.substring(ssidIndex, header.indexOf('&'));
                    pass_param = header.substring(passIndex, header.indexOf(' ', passIndex));

                    ssid_param.toCharArray(ssid, 32);
                    pass_param.toCharArray(pass, 64);
                    Serial.println("SSID is : ");
                    Serial.println(ssid);
                    Serial.println("Password is : ");
                    Serial.println(pass);

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println("Connection: close");
                    client.println();
                    client.println(generateSuccessPage());
                    client.println();
                    Serial.println("Sending Success Page...");
                    delay(1000);
                    break;
                }
                else if (header.length() == 0)
                {

                    Serial.println("Sending Config Page...");
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
                    Serial.println("POST request received!");

                    // Read the POST request body
                    String postBody = "";
                    while (client.available())
                    {
                        postBody += (char)client.read();
                    }

                    // Extract SSID and password from the POST body
                    int ssidIndex = postBody.indexOf("ssid=") + 5;
                    int passIndex = postBody.indexOf("pass=") + 5;
                    ssid_param = postBody.substring(ssidIndex, postBody.indexOf('&', ssidIndex));
                    pass_param = postBody.substring(passIndex); // Assuming password is the last field

                    ssid_param.toCharArray(ssid, 32);
                    pass_param.toCharArray(pass, 64);
                    Serial.println("SSID is : ");
                    Serial.println(ssid);
                    Serial.println("Password is : ");
                    Serial.println(pass);

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println("Connection: close");
                    client.println();
                    client.println(generateSuccessPage());
                    client.println();
                    Serial.println("Sending Success Page...");
                    delay(1000);

                    break;
                }
                else
                {
                    header = "";
                }
            }
            else if (c != '\r')
            {                // if you got anything else but a carriage return character,
                header += c; // add it to the end of the currentLine
                tmpRequest += c;
            }
        }
    }
    Serial.print("Header is : ");
    Serial.println(tmpRequest);
    if (ssid_param.length() > 0 && pass_param.length() > 0)
    {
        needsWiFiConfig = false;
        connectToWiFi();
    }

    client.stop();
}

/**
 * @brief Function to connect to the Wi-Fi network using the credentials provided by the user.
 */
void connectToWiFi()
{
    // Serial.print("Connecting to WiFi SSID: ");
    // Serial.println(ssid);
    WiFi.end();
    delay(1000);
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {

        //status = WiFi.begin(ssid, pass);
        status = WiFi.begin("SamsungR", "123456789");
        Serial.print("The Wifi is status : ");
        Serial.println(WiFi.status());
        Serial.println("LE WIFI ET MP SONT RENTRE EN DUREE");
        printWifiStatus();
        delay(1000);
        Serial.print(".");

        if ((WiFi.status() == WL_CONNECT_FAILED) || (WiFi.status() == WL_DISCONNECTED))
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
    }
    Serial.println();
    Serial.println("Connected to WiFi!");
    printWifiStatus();

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    server.begin();
    connectedToWiFi = true;
    delay(1000);
}

/**
 * @brief Prints the web page with the sensor values
 */
void printWEB()
{
    String tmpHeader = "";
    client = server.available();

    if (client)
    {
        readSensors();
        Serial.println("new client yeay");
        Serial.print("object client : ");
        Serial.println(client);
        Serial.print("client connected : ");
        Serial.println(client.connected());
        Serial.print("client available : ");
        Serial.println(client.available());
        String currentLine = "";
        while (client.connected())
        {

            if (client.available())
            {

                char c = client.read();
                Serial.write(c);
                Serial.println(currentLine);

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
                        Serial.println("reconfigure");
                        needsWiFiConfig = true;
                        connectedToWiFi = false;
                        incorrectPassword = false;
                        ssid[0] = '\0';
                        pass[0] = '\0';
                        client.stop();
                        startAccessPoint();
                        return;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                    tmpHeader += c;
                }
            }
        }
        Serial.print("current line is : ");
        Serial.println(tmpHeader);
    }
    client.stop();
    Serial.println("client disconnected nooo");
}

int statusCode = 0;
/**
 * @brief Send the sensor data to the distant server
 */
void sendSensorData()
{
    readSensors();
    String jsonData;
    serializeJson(doc, jsonData);
    Serial.print("JSON data: ");
    Serial.println(jsonData);

    httpClient.setHttpResponseTimeout(5000);

    if (!statusCode)
    {
        // Send HTTP POST request
        Serial.println("Sending HTTP begin request...");
        httpClient.beginRequest();
        Serial.println("Request begun!");
        if (httpClient.connected())
        {
            Serial.println("Client connected!");
        }
        else
        {
            Serial.println("Client not connected!");
        }
        if (httpClient.patch("/sensor-data") != 0)
        {
            Serial.println("Post request failed!");
            httpClient.stop();
            return;
        }
        else
        {
            Serial.println("Post request sent!");
        }

        Serial.println("Patch request sent!");
        httpClient.sendHeader("Content-Type", "application/json");
        Serial.println("Header sent!");
        httpClient.sendHeader("Content-Length", jsonData.length());
        Serial.println("Header sent!");
        httpClient.beginBody();
        Serial.println("Body begun!");
        httpClient.print(jsonData);
        Serial.println("json data sent!");
        httpClient.endRequest();
        Serial.println("Request sent!");

        // Get HTTP response
        statusCode = httpClient.responseStatusCode();
        String response = httpClient.responseBody();
        Serial.print("Status code: ");
        Serial.println(statusCode);
        Serial.print("Response: ");
        Serial.println(response);

        statusCode = 0;
    }
}
//--------------------------------------------Others function--------------------------------------------

/**
 * @brief Read the sensors values
 */
void readSensors()
{
    // Read the sensors values
    sensorDatas.soilHumidity = analogRead(0);
   
    sensorDatas.airHumidity = dht.readHumidity();
    sensorDatas.temperature = dht.readTemperature();

    // Map sensor values to percentage
    sensorDatas.percentage = map(sensorDatas.soilHumidity, dry, wet, 0, 100);
   
    sensorDatas.visible = SI1145.ReadVisible();
    sensorDatas.IR = SI1145.ReadIR();
    sensorDatas.UV = ((float)SI1145.ReadUV() / 100); // Value must be divided by 100 to get the correct value
                                         // Reason can be found in the datasheet of the sensor :
                                         // https://www.silabs.com/documents/public/data-sheets/Si1145-46-47.pdf
                                         // on page 16

    // Create JSON object
    doc["temperature"] =  sensorDatas.temperature;
    doc["airHumidity"] =  sensorDatas.airHumidity;
    doc["soilHumidity"] =  sensorDatas.soilHumidity;
    doc["moisture1"] =  sensorDatas.percentage;
    doc["visible"] =  sensorDatas.visible;
    doc["IR"] =  sensorDatas.IR;
    doc["UV"] =  sensorDatas.UV;
}

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