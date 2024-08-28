#ifndef WEBPAGES_H
#define WEBPAGES_H

String generateConfigPage(std::vector<const char *> networks)
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
        <input type="submit" value="Submit">
    </form>
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
    <h1>Configuration Successful!</h1>
    <p>The device is now connecting to the specified WiFi network.</p>
</body>
</html>
)rawliteral";
    return webpage;
}

#endif
