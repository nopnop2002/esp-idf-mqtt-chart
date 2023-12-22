# esp-idf-mqtt-chart
MQTT data visualization using esp-idf.   
Display MQTT data in a chart.   
I was inspired by [this](https://blog.postman.com/postman-supports-mqtt-apis/).   

![mqtt-chart-json-1](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/af71dc20-d279-4673-8972-49ca037f714e)

I used [this](https://github.com/Molorius/esp32-websocket) component.   
This component can communicate directly with the browser.   
I use [this](https://plotly.com/javascript/) Graphing Library.   


# Software requirements
ESP-IDF V4.4/V5.x.   
ESP-IDF V5.0 is required when using ESP32-C2.   
ESP-IDF V5.1 is required when using ESP32-C6.   

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-mqtt-chart
cd esp-idf-mqtt-chart/
git clone https://github.com/Molorius/esp32-websocket components/websocket
idf.py set-target {esp32/esp32s2/esp32s3/esp32c2/esp32c3/esp32c6}
idf.py menuconfig
idf.py flash monitor
```

# Configuration

![config-top](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/c8debaa7-6a17-4f0a-a2d3-57d3e93fa2d9)
![config-app](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/8f454ed9-0e5f-480e-b0df-20a092fb26e2)

## WiFi Setting

![config-wifi-1](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/54313903-d98e-47bb-a596-beae7b631d1e)

You can connect using the mDNS hostname instead of the IP address.   
![config-wifi-2](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/9317f1df-8538-47ea-be5e-25a1a48b9ae2)

## Broker Setting
MQTT broker is specified by one of the following.
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```mqtt-broker.local```   
- Fully Qualified Domain Name   
 ```broker.emqx.io```

You can download the MQTT broker from [here](https://github.com/nopnop2002/esp-idf-mqtt-broker).   

![config-broker-1](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/abe826f4-41d2-4ba3-8b99-ca0a77915208)

Specifies the username and password if the server requires a password when connecting.   

![config-broker-2](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/46e84d5a-c701-4e8c-8af7-50a305adbf9c)


# Supported MQTT data formats
- JSON data format   
MQTT payload is in JSON format as shown below.   
You can check it working using mqtt_json.sh.   
```
{
        "sin":  0,
        "cos":  -1,
        "tan":  0,
}
```

- Simple data format   
MQTT payload is a simple number.   
You can check it working using mqtt_simple.sh.   


# How to use
- Start this application.   
- Start ```mqtt_json.sh``` or ```mqtt_simple.sh```.   
- Enter the following in the address bar of your web browser.   
```
http:://{IP of ESP32}/
or
http://esp32-server.local/
```
- You can change the number of X-axis.   
![mqtt-chart-json-2](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/428bacd1-ca45-4980-a0d3-d32556b1a708)

- You can change the Y-axis range to auto scale or manual scale.   
![mqtt-chart-json-3](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/b80295ff-186f-47be-936d-01d033c768a8)

# Built-in menu
You can use the built-in menu.   
Download plot as PNG/Zoom Area/Pan/Zoom In/Zoom Out/Hover/Tooltips.   

![mqtt-chart-menu-1](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/0dec3938-0417-4cfe-bab5-fc7e3ac8d617)
![mqtt-chart-menu-2](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/f8a0c145-86cc-45a8-89a9-0ab662482bfe)
![mqtt-chart-menu-3](https://github.com/nopnop2002/esp-idf-mqtt-chart/assets/6020549/da768b5f-42d5-4fc4-bded-ba309883cbf2)

# WEB Pages   
WEB pages are stored in the html folder.   
You can change it as you like.   

# Limitations
Up to 3 axes can be displayed at one time.   


