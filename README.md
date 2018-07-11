# esp32-slcan
This repository has an arduino sketch to create a slcan USB/Bluetooth device using the ESP32, SN65HVD230 CAN transceiver and 128x32 SSD1306.
<br>Requires the use of the following Arduino libraries
<br><br>https://github.com/nhatuan84/arduino-esp32-can-demo
<br>Adafruit_GFX
<br>Adafruit_SSD1306

Current "ESP32 ESP-WROOM-32 Wemos D1" uses CP2102 USB to TTL which is limited to 500kbps
Schematic for the device to follow

![Outside](./outside.jpg?raw=true “Outside”)
![Inside Above](./inside_above.jpg?raw=true “Inside Above”)
![Inside Under](./inside_under.jpg?raw=true “Inside Under”)

