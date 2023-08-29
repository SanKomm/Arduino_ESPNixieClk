SKETCH_FOLDER := Arduino_ESPNixieClk

#arduino-cli parameters
LIBS ?= libraries/WiFiManager,libraries/NTPClient
UPLOAD_PORT ?= COM6
UPLOAD_SPEED ?= 115200

#check for and add esp8266 core
addesp8266:
	arduino-cli core install \
	--additional-urls=http://arduino.esp8266.com/stable/package_esp8266com_index.json esp8266:esp8266

#add the missing libs
submodules:
	git submodule update --init

#creates sketch bin
makebin: $(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino.bin
	arduino-cli compile --libraries=$(LIBS) \
	-b esp8266:esp8266:nodemcu $(SKETCH_FOLDER)

#flashes esp8266 with specified parameters
makeflash:
	arduino-cli upload -b esp8266:esp8266:nodemcu -p $(UPLOAD_PORT) $(SKETCH_FOLDER)

