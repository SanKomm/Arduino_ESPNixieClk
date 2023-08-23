ARDUINO_ROOT ?= $(abspath $(wildcard ../../Arduino/.) $(wildcard ../Arduino/.))
ESP_ROOT ?= $(ARDUINO_ROOT)/hardware/esp8266com/esp8266
SKETCH_FOLDER = $(ARDUINO_ROOT)/Arduino_ESPNixieClk

#makeEspArduino parameters
SKETCH ?= $(SKETCH_FOLDER)/Arduino_ESPNixieClk.ino
CUSTOM_LIBS ?= $(ESP_ROOT)/libraries \
	$(SKETCH_FOLDER)/libraries/WiFiManager \
	$(SKETCH_FOLDER)/libraries/NTPClient
EXCLUDE_DIRS ?= ESP8266mDNS
UPLOAD_PORT ?= COM6
UPLOAD_SPEED ?= 115200

#check for and add esp8266 core
addesp8266:
ifeq ($(wildcard $(ARDUINO_ROOT)/hardware/.),)
	cd $(ARDUINO_ROOT) && mkdir hardware
endif
ifeq ($(wildcard $(ARDUINO_ROOT)/hardware/esp8266com/.),)
	cd $(ARDUINO_ROOT)/hardware && mkdir esp8266com
endif
	cd $(ARDUINO_ROOT)/hardware/esp8266com && \
	git clone https://github.com/esp8266/Arduino.git esp8266
	cd $(ESP_ROOT) && git submodule update --init
	cd $(ESP_ROOT)/tools && python3 get.py

#add the missing libs and makeesp files
submodules:
	git submodule update --init

#flashes esp8266 with specified parameters
espmake:
	cd $(SKETCH_FOLDER)/makeEspArduino && $(MAKE) -f makeEspArduino.mk \
	ESP_ROOT=$(ESP_ROOT) \
	CUSTOM_LIBS=$(CUSTOM_LIBS) \
	EXCLUDE_DIRS=$(EXCLUDE_DIRS) \
	SKETCH=$(SKETCH) \
	UPLOAD_PORT=$(UPLOAD_PORT) \
	UPLOAD_SPEED=$(UPLOAD_SPEED) \
	flash
espclean:
	cd $(SKETCH_FOLDER)/makeEspArduino && $(MAKE) -f makeEspArduino.mk \
	ESP_ROOT=$(ESP_ROOT) \
	CUSTOM_LIBS=$(CUSTOM_LIBS) \
	EXCLUDE_DIRS=$(EXCLUDE_DIRS) \
	SKETCH=$(SKETCH) \
	UPLOAD_PORT=$(UPLOAD_PORT) \
	UPLOAD_SPEED=$(UPLOAD_SPEED) \
	clean