#arduino-cli parameters
SKETCH_FOLDER := Arduino_ESPNixieClk
UPLOAD_PORT ?= COM6

all: $(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino.bin

$(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino.bin: $(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino
	arduino-cli compile -b esp8266:esp8266:generic $(SKETCH_FOLDER)

deps:
	arduino-cli core install \
	--additional-urls=http://arduino.esp8266.com/stable/package_esp8266com_index.json esp8266:esp8266
	arduino-cli lib install wifimanager
	arduino-cli lib install	ntpclient

flash: $(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino.bin
	arduino-cli upload -b esp8266:esp8266:generic -p $(UPLOAD_PORT) $(SKETCH_FOLDER)



