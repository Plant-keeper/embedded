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

// Global variables
char ssid[32];
char pass[64];
bool needsWiFiConfig = true;

const char *ssidArduino = "Arduino_Config";
const char *passArduino = "password";

WiFiServer server(80); // Server socket
// WiFiWebServer server(80);
/* WiFiClient client; */
int status = WL_IDLE_STATUS;
WiFiClient client;
std::vector<const char *> networks;

// Instantiate DHT and JSon object
DHT dht(2, DHT11);
JsonDocument doc;
// Global variables used in loop
int val1;
int val2;
int percentage1;
int percentage2;
float h;
float t;

const int dry = 1023;
const int wet = 700;

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

void setup()
{

    Serial.begin(9600);

    Serial.println("Starting...");

    if (WiFi.status() != WL_CONNECTED)
    {
        listNetworks(networks);
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
}

//--------------------------------------------WEB SERVER FUNCTIONS--------------------------------------------

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
 * Function to start the Arduino in Access Point mode
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
 * Function to handle the configuration request and extract SSID and Password
 */
void handleConfigRequest()
{
    Serial.println("Handling Config Request...");
    String header = "";
    String ssid_param = "";
    String pass_param = "";
    String tmpRequest = "";

    WiFiClient client = server.available();

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
                    client.println(generateConfigPage(networks));
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
 * Function to connect to the Wi-Fi network using the credentials provided by the user.
 */
void connectToWiFi()
{
    Serial.print("Connecting to WiFi SSID: ");
    Serial.println(ssid);
    WiFi.end();
    delay(1000);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {

        status = WiFi.begin(ssid, pass);
        Serial.print("Wifi status : ");
        Serial.println(WiFi.status());
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Connected to WiFi!");

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    server.begin();
    delay(1000);
}

/**
 * Function that handles the HTTP response to display a web page.
 * It receives client requests, sends the HTML page, and manages the disconnection.
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

                if (c == '\n')
                {
                    if (currentLine.length() == 0)
                    {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        client.println(generateDataPage(t, h, percentage1, percentage2));
                        client.println();

                        break;
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

//--------------------------------------------END OF WEB SERVER FUNCTIONS--------------------------------------------

void readSensors()
{
    // Read the sensors values
    val1 = analogRead(0);
    val2 = analogRead(1);

    h = dht.readHumidity();
    t = dht.readTemperature();

    // Map sensor values to percentage
    percentage1 = map(val1, dry, wet, 0, 100);
    percentage2 = map(val2, dry, wet, 0, 100);

    // Create JSON object
    doc["temperature"] = t;
    doc["humidity"] = h;
    doc["val1"] = val1;
    doc["val2"] = val2;
    doc["moisture1"] = percentage1;
    doc["moisture2"] = percentage2;
    serializeJson(doc, Serial);

    
}