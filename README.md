# Learn about the iot part of PlantKeeper

## Original Contributors
Rafael Dousse, Eva Ray, Quentin Surdez and Rachel Tranchida

## Prerequisites

To run this project locally, please follow these steps:

__1. Hardware:__
-  You will need an __Arduino MKR WiFi 1010__ board. Other WiFi-capable boards (such as the ESP32) may work, but compatibility with libraries might vary.
- The sensors used in this project include the __DHT11__ (temperature and humidity), the __UV and Lumen sensor__, and the __capacitive soil moisture sensor__. Below are the default pin connections:
    - __DHT11:__ Digital Pin D2
    - __UV and Lumen sensor:__ SCL -> SCL (D12), SDA -> SDA (D11)
    - __Capacitive soil moisture sensor:__ Analog Pin A0
    - You can modify the pin configuration in the code if you wish to use different pins.

__2. Software:__
- The project is built using the __PlatformIO__ extension for __VSCode__. While it can be adapted for the Arduino IDE, this will require changes to the code structure and file extensions.
- You will also need a server to receive data from the Arduino. This can be set up locally using a __Flask server__, or you can deploy the server using the provided __Dockerfile__ in our  [repository](https://github.com/Plant-keeper/servers) dedicated to the servers.

__3. Server Configuration for the arduino:__

- Update the Arduino code to point to your server’s IP address and port. Modify the following lines:

    ```cpp
        // Replace with your server's IP address
        IPAddress serverAddress();  
        // Replace with your server's port
        int port = ; 

        String ipWifi = ""; 
    ```	
- The WiFi SSID and password can be set in the `arduino_secrets.h` file. This allows the Arduino to connect in Access Point mode.

- Additionally, update the `ipWifi` variable with the Arduino’s IP address on the local network. This IP will be displayed in the serial monitor after the first successful WiFi connection. You can update the code afterward, as this won't prevent it from running initially.

### Running the Project

__1. Uploading Code to the Arduino:__ After compiling and uploading the code, the Arduino will enter Access Point mode, broadcasting its own WiFi network.

__2. Connecting to the Arduino:__ Connect your smartphone to the Arduino’s WiFi. Once connected, open a browser and navigate to the IP address displayed in the serial monitor (usually `192.168.4.1`).

__3. Configuring WiFi Settings:__ On the configuration page, input your WiFi network credentials (SSID and password) and the Arduino’s ID. A webpage will be displayed informing you that the arduino is connecting to the WiFi. If the connection fails, the Arduino will revert to Access Point mode and you need to re-enter the WiFi credentials on the configuration page.

__4. Sending Data to the Server:__ Once connected to your WiFi network, the Arduino will begin sending sensor data to the server every 5 minutes. The current sensor data can also be viewed by navigating to the Arduino's new IP address, which is displayed in the serial monitor. This address should also be updated in the `ipWifi` variable.

#### Status Indicators

- The built-in LED on the Arduino provides visual feedback for connection status:
    - __ON:__ The Arduino is in Access Point mode, waiting for WiFi configuration.
    - __Blinking (1Hz):__ The Arduino is connected and actively sending data to the server.
    - __Irregular Blinking:__ The Arduino is connected but unable to communicate with the server (e.g., server is down).

> **_Note:_** The Arduino ID is provided by the backend when you register a plant on the website.

## Technical choices

In __PlantKeeper__, we designed the iot system to collect and transmit sensor data that monitors environmental
conditions around the plants. After considering various hardware options and challenges, we made several technical
choices that optimized the performance and reliability of the system:

### 1. Arduino for sensor data collection

We chose to use an __Arduino__ for collecting and sending sensor data. Initially, we attempted to use a __Raspberry Pi__
due to
its superior processing capabilities, but we encountered issues with the sensor library (__grovelibrary__) that proved
difficult to resolve in a short time, mainly due to limited online resources. Arduino, on the other hand, provided a
more
straightforward and well-supported solution. While it has lower processing power, it is well-suited for simple sensor
data acquisition tasks. Additionally, Arduino offers low power consumption, making it an efficient and cost-effective
choice for our use case.

### 2. C++ for Arduino programming

We programmed the Arduino using __C++__, the standard language for Arduino development. C++ is ideal for iot
systems
because it provides both __low-level control__ of hardware resources and __high-level abstractions__ for working with
complex
data types. Its lightweight nature ensures minimal overhead, making it highly suitable for the limited computational
resources of an Arduino. Furthermore, the availability of extensive libraries for sensors and peripherals in the Arduino
ecosystem made C++ an obvious choice.

### 3. Capacitive soil moisture sensor

To monitor the soil humidity, we used a __capacitive soil moisture sensor__. The sensor initially returns raw analog
values
between 0 and 1023, which represent moisture levels. Since these values are not intuitive for our application, we
transformed them into a percentage by using the `map()` function from the `common.h` library. This allows us to present
the
soil moisture data in a more user-friendly percentage format, giving a clearer understanding of the plant’s soil
condition. It is important to note that the sensor is not always accurate and the values can vary a lot depending on the
soil type and the plant. However, it is still useful to get a general idea of the soil moisture level.

### 4. Grove Sunlight Sensor for UV and Lumen Monitoring

We used the __Grove - Sunlight Sensor__ to measure UV and ambient light (lumen). This sensor uses the `SI114X.h` library
for data acquisition. During testing, we observed that in a pitch-black environment, the sensor still returned a lumen
value of 160, while in bright light, it would not exceed 800. Since our application needed to capture lumen values
between 0 and 2000, we applied a linear transformation using the `map()` function from the `common.h` library. This
adjusted
the sensor’s raw readings to fit our required range. It is important to note that the sensor is not always accurate and
the values can vary a lot depending on the light source and the environment.

### 5.Grove DHT11 Sensor for Temperature and ambient Humidity

For measuring ambient temperature and humidity, we used the __DHT11__ sensor from __Grove__ sensors, which is paired
with the __DHT.h__
library. The sensor directly provides the temperature in Celsius, making it easy to integrate into our system. Its
simplicity and reliability make it ideal for tracking the room conditions where the plant is located, providing
essential data for plant health monitoring. It is important to note that the sensor is not always accurate and the
values
can vary a lot depending on the environment.

### 6. Data Transmission via HTTP

The Arduino sends the sensor data to a proxy server via __HTTP__ every 5 minutes. This frequency is set to balance
between
timely updates and minimizing power consumption. Also, since we are monitoring plants, we don't need very frequent
updates since the plant's condition changes slowly and not in real-time.
The regular data transmission ensures that the user is always
up-to-date with the plant’s condition without overwhelming the server or network with excessive requests.

In summary, the iot system of PlantKeeper combines well-thought hardware and software choices to reliably
collect and transmit environmental data. By leveraging the power efficiency of Arduino and the flexibility of C++, along
with careful calibration of our sensors, we were able to design a system that is both functional and adaptable to the
needs of plant monitoring.

## Improvements and Future Work





## Simulating the Arduino (Without Hardware)

If you don’t have access to an Arduino board or the required sensors, you can still simulate the project’s functionality by running a Python script that mimics the Arduino's behavior. This simulation will send data to the server every 5 minutes.

__Steps to Run the Simulation:__

__1. Open the Simulation Script:__
- Navigate to the fakeArduino.py file located in the simulateArduino folder.

__2. Set Server Configuration:__
- In the Python script, update the server IP address and port to match your server’s configuration.

__3. Run the Script:__
 - Execute the script in your terminal using the following command:
    `python3 fakeArduino.py`

This will simulate the behavior of the Arduino, allowing you to see how the data is handled by the server without needing the actual hardware.