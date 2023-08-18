# Arduino_ESPNixieClk 

Made with Arduino IDE version 2.1.1.  
Utilises Arduino core for ESP8266(https://github.com/esp8266/Arduino) installed according to these guidelines: https://github.com/esp8266/Arduino#installing-with-boards-manager  
  
Used libraries:  
https://github.com/tzapu/WiFiManager  
https://github.com/arduino-libraries/NTPClient  

The Make file is used to run the commands for adding all the files and libraries as well as to flash the microcontroller.
Use command **addesp8266** to add the necessary esp8266 core. Use command **addespmake** to add the makefiles for flashing an esp8266.
Use command **espmake** to compile and flash the sketch onto the microcontroller.
