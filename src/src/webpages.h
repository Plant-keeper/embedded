/**
 * Project Name: PlantKeeper
 *
 * @created 23.08.2024
 * @file  webpages.h
 * @version 1.0.0
 * @see https://github.com/Plant-keeper
 *
 * @authors
 *   - Rafael Dousse
 *   - Eva Ray
 *   - Quentin Surdez
 *   - Rachel Tranchida
 */

#ifndef WEBPAGES_H
#define WEBPAGES_H
#include "sensorData.h"


String generateConfigPage(std::vector<const char *> networks, bool passwordFailed)
{     
    String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WiFi Configuration</title>
</head>
<style>
  body {
            background: #0a0a23;
            color: #fff;
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            text-align: center;
        }

        input, select {
            width: 50%;
            padding: 10px;
            margin-bottom: 15px;
            border-radius: 5px;
            border: none;
        }

        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            cursor: pointer;
        }
</style>
<body>
    <h1>WiFi Configuration</h1>  
  <label for="ssid">Select a network:</label>
       <form action="/submit" method="POST">
<select name="ssid" id="ssid"> )rawliteral";
    for (auto network : networks)
    {
        webpage += "<option value=\"" + String(network) + "\">" + String(network) + "</option>";
    }
    webpage += R"rawliteral(
</select>
<br><br>

        <label for="pass">Password:</label><br>
        <input type="password" id="pass" name="pass"><br><br>
        <label for="idsensor">Enter your sensor id:</label><br>
        <input type="number" id="idsensor" name="idsensor"><br><br>
        <input type="submit" value="Submit">
    </form>
    )rawliteral";
    if(passwordFailed) {
        webpage += "<p style=\"color: red;\">You've entered an incorrect password. Please try again.</p>";
    }
    
      webpage += R"rawliteral(
</body>
</html>
)rawliteral";
    return webpage;
}

String generateSuccessPage()
{
    String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WiFi Configuration</title>
</head>
<style>
 body {
            background: #0a0a23;
            color: #fff;
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            text-align: center;
        }

</style>
<body>
    <h1>Your sensor is connecting!</h1>
    <p>The device is now connecting to the specified WiFi network. If you cannot access to your datas
    then you might have entered an incorect password. Check the previous ip page and reconnect to the wifi in that case </p>
    <p> To access to your datas, click on this link <a href="http://192.168.183.146"> datas </a>   </p>
</body>
</html>
)rawliteral";
    return webpage;
}


String generateDataPage(String IP, sensorData data) {
    String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Hello Plant</title>
</head>
<style>
 body {
            background: #0a0a23;
            color: #fff;
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            text-align: center;
        }

        input[type="submit"] {
        background-color: #4CAF50;
        color: white;
        cursor: pointer;
    }

</style>
<body>
    <h1>Hello, Plant!</h1>
    <p>Current Temperature: )rawliteral";
    webpage += String(data.temperature) + " &deg;C</p>";
    webpage += "<p>Current Humidity: " + String(data.airHumidity) + " %</p>";
    webpage += "<p> Value 1: " + String(data.soilHumidity) + "</p>";
    webpage += "<p>Soil Moisture 1: " + String(data.percentage) + " %</p>";
    webpage += "<p>Visible Light: " + String(data.visible) + "</p>";
    webpage += "<p>Infra Light: " + String(data.IR) + "</p>";
    webpage += "<p>UV Light: " + String(data.UV) + "</p>";
    webpage += R"rawliteral(
    <p>Click here on the button below to change the WiFi configuration:</p>
   
    <form id="reconfigForm" action="/reconfigure" method="post">
        <button type="button" onClick="submitFormAndRedirect()">Changer la configuration Wi-Fi</button>
    </form>

    <script>
        function submitFormAndRedirect() {
            // Submit the form
            document.getElementById('reconfigForm').submit();
            
            // Wait a moment before redirecting to allow form submission
            setTimeout(function() {
                window.location.href = 'http://192.168.4.1';
            }, 3000); // Adjust the timeout as necessary
        }
    </script>


</body>
</html>
)rawliteral";
    return webpage;
}
#endif
