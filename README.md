# JH_Box
This is a module for environmental sensing
There are DHT22 Temperature and humidity sensor, PM2.5 sensor. 
And there ard OLED monitor, LoRa(SX1278), ESP01(ESP8266) module for communication.
I draw a circuit board by myself, use nuvoton nano100 as mcu.

I build a LoRa mesh network between my gateway and client. 
Client collect environmental data like temperature, humidity or PM2.5.
Client send data to gateway by LoRa . 
Gateway get the data, and upload to Thingspeak


https://imgur.com/7Plcxj8
