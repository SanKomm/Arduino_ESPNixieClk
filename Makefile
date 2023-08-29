SKETCH_FOLDER := Arduino_ESPNixieClk

#arduino-cli parameters
LIBS ?= libraries/WiFiManager,libraries/NTPClient
UPLOAD_PORT ?= COM6
UPLOAD_SPEED ?= 115200

all: $(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino.bin

$(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino.bin: $(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino
	arduino-cli compile --library=$(LIBS) \
	-b esp8266:esp8266:generic $(SKETCH_FOLDER)

deps:
	arduino-cli core install \
	--additional-urls=http://arduino.esp8266.com/stable/package_esp8266com_index.json esp8266:esp8266
	git submodule update --init

flash: $(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino.bin
	arduino-cli upload -b esp8266:esp8266:generic -p $(UPLOAD_PORT) $(SKETCH_FOLDER)



