# Arduino_ESPNixieClk 

Made with Arduino IDE version 2.1.1.  
Utilises Arduino core for ESP8266(https://github.com/esp8266/Arduino) installed according to these guidelines: https://github.com/esp8266/Arduino#installing-with-boards-manager  
  
Used libraries:  
https://github.com/tzapu/WiFiManager  
https://github.com/arduino-libraries/NTPClient  

The Make file is used to run the commands for adding all the files and libraries as well as to flash the microcontroller:  
make -f Make [command]  
Use command **addesp8266** to add the esp8266 core.  
Use command **submodules** to add necessary libraries and makeEspArduino files.  
Use command **espmake** to compile and flash the sketch onto the microcontroller.
