#######
ARDUINO_ROOT ?= $(shell find $(HOME) -name 'Arduino' 2>/dev/null | grep -v AppData)
#ARDUINO_ROOT ?= $(HOME)/Documents/Arduino
ESP_ROOT ?= $(ARDUINO_ROOT)/hardware/esp8266com/esp8266

#makeEspArduino parameters
SKETCH ?= $(ARDUINO_ROOT)/Arduino_ESPNixieClk/Arduino_ESPNixieClk.ino
CUSTOM_LIBS ?= $(ESP_ROOT)/libraries \
	$(ARDUINO_ROOT)/Arduino_ESPNixieClk/libraries/WiFiManager \
	$(ARDUINO_ROOT)/Arduino_ESPNixieClk/libraries/NTPClient
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
	cd $(ARDUINO_ROOT)/Arduino_ESPNixieClk/makeEspArduino && $(MAKE) -f makeEspArduino.mk \
	ESP_ROOT=$(ESP_ROOT) \
	CUSTOM_LIBS=$(CUSTOM_LIBS) \
	EXCLUDE_DIRS=$(EXCLUDE_DIRS) \
	SKETCH=$(SKETCH) \
	UPLOAD_PORT=$(UPLOAD_PORT) \
	UPLOAD_SPEED=$(UPLOAD_SPEED) \
	flash
espclean:
	cd $(ARDUINO_ROOT)/Arduino_ESPNixieClk/makeEspArduino && $(MAKE) -f makeEspArduino.mk \
	ESP_ROOT=$(ESP_ROOT) \
	CUSTOM_LIBS=$(CUSTOM_LIBS) \
	EXCLUDE_DIRS=$(EXCLUDE_DIRS) \
	SKETCH=$(SKETCH) \
	UPLOAD_PORT=$(UPLOAD_PORT) \
	UPLOAD_SPEED=$(UPLOAD_SPEED) \
	clean