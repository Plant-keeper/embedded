# Learn about the embedded part of PlantKeeper

## How to launch locally

## Technical choices

In __PlantKeeper__, we designed the embedded system to collect and transmit sensor data that monitors environmental
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

We programmed the Arduino using __C++__, the standard language for Arduino development. C++ is ideal for embedded
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
essential data for plant health monitoring. It is important to note that the sensor is not always accurate and the values
can vary a lot depending on the environment.

### 6. Data Transmission via HTTP

The Arduino sends the sensor data to a proxy server via __HTTP__ every 5 minutes. This frequency is set to balance between
timely updates and minimizing power consumption. Also, since we are monitoring plants, we don't need very frequent 
updates since the plant's condition changes slowly and not in real-time.
The regular data transmission ensures that the user is always
up-to-date with the plant’s condition without overwhelming the server or network with excessive requests.

## Improvements and Future Work