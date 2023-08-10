# nixie_esp

Made with Arduino IDE version 2.1.1.  
Utilises Arduino core for ESP8266(https://github.com/esp8266/Arduino) installed according to these guidelines: https://github.com/esp8266/Arduino#installing-with-boards-manager  
  
Used libraries:  
https://github.com/tzapu/WiFiManager  
https://github.com/bblanchon/ArduinoJson  
https://github.com/arduino-libraries/NTPClient  
Download the library zip files and extract them into the Arduino libraries folder e.g. %homedrive%%homepath%/Documents/Arduino/libraries  
makeEspArduino(https://github.com/plerup/makeEspArduino/tree/master) is utilised to compile and flash the sketch onto the ESP8266. makeEspArduino is installed according to the guidelines in the link.  
  
The flashing process:  
cd %homedrive%%homepath%/nixie_esp  
make -f %homedrive%%homepath%/makeEspArduino/makeEspArduino.mk flash 
   
If ESP8266 core is installed with git:  
cd %homedrive%%homepath%/makeEspArduino  
make -f makeEspArduino.mk ESP_ROOT=%homedrive%%homepath%/Documents/Arduino/hardware/esp8266com/esp8266 install  
cd %homedrive%%homepath%/nixie_esp  
espmake

 
