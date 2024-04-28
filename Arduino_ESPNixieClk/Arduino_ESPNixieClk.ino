/*
https://github.com/laurivosandi/nixiesp12/blob/master/firmware/main.py
https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/
*/

#include <WiFiManager.h>
#include <TimeLib.h>
#include "timezones.h"
#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.hpp"
#include "ESP8266_ISR_Timer.h"
ESP8266Timer ITimer;

//NTP server and refresh interval.
#define MY_NTP_SERVER "at.pool.ntp.org"
#define updateInterval 360000

//WifiManager
WiFiManager wm;

//Assign output variables to GPIO pins.
const int clock1 = 3;
const int data = 2;
const int latch = 0;

// Dimmer settings
int dimming_timer_period = 100;
int dimming_duty_cycle = 25;
long dimmer_interrupt_count = 0;
long dimmer_duty_cycle_count = 0;

//Debug settings.
const bool DEBUG = true;
const bool DEBUG_PIN = false;

enum format{
  TIME,
  DATE,
  DATETIME
};
enum format displayFormat = TIME;

int boot_time = 0;
int ntp_sync_count = 0;
bool blink = false;
int lookup[] = {11, 9, 12, 8, 0, 4, 1, 3, 2, 10};

//Time structs
time_t no;
tm tm;

//"weak" function to set NTP update interval
uint32_t sntp_update_delay_MS_rfc_not_less_than_15000(){
  return 8 * 60 * 60 * 1000ul;//8 hours 
}

void ICACHE_RAM_ATTR time_is_set(){
  Serial.println("Updated NTP connection");
  if (!boot_time) {
      time(&no);
      boot_time = no;
  }
  ntp_sync_count++;
}

/*
*Description:  Appends the ESP MAC address to the accesspoint name. 
*Parameters:   macAdr - The MAC address in bytes.
*              tmp - pointer to the accesspoint name.
*/
void mac_address(byte macAdr[],char *tmp){
  int clkLen = strlen(tmp);

  for (unsigned int i = 0; i < 6; i++)
  {
      byte nib1 = (macAdr[i] >> 4) & 0x0F;
      byte nib2 = (macAdr[i] >> 0) & 0x0F;
      tmp[i*2+clkLen] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
      tmp[i*2+clkLen+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  
  int clkLen_2 = strlen(tmp);
  tmp[24] = '\0';
}

/*
*Description:  Writes an appropriate bit to the data pin.
*Parameters:   value - data that dictates bit value.
*/
void bitbang_bit(int value){
  if(value & 1){
    digitalWrite(data, HIGH);
  }
  else{
    digitalWrite(data, LOW);
  }
  if(DEBUG_PIN){
    Serial.println(digitalRead(data));
  }
  digitalWrite(clock1, HIGH);
  digitalWrite(clock1, LOW);
}

/*
*Description:  Writes a digit to the corresponding clock position.
*Parameters:   digit - value of the written digit.
*/
void bitbang_digit(int digit){
  int i = 0;
  bitbang_bit(blink);
  for(i=0;i<4;i++){
    bitbang_bit(lookup[digit] << i >> 3);
  }
  bitbang_bit(blink);
  bitbang_bit(blink);
  bitbang_bit(blink);
}


//Description:  Writes the current time to the Nixie clock.
void dump_time(){
  int hour = tm.tm_hour;
  int minute = tm.tm_min;
  int second = tm.tm_sec;

  if (DEBUG)
    {
      Serial.print("Time is ");
      Serial.print(hour);
      Serial.print(":");
      Serial.print(minute);
      Serial.print(":");
      Serial.println(second);
    }
    bitbang_digit(hour / 10);
    bitbang_digit(hour % 10);
    bitbang_digit(minute / 10);
    bitbang_digit(minute % 10);
    bitbang_digit(second / 10);
    bitbang_digit(second % 10);
}

//Description:  Writes the current date to the Nixie clock.
void dump_date(){
  int day = tm.tm_mday;
  int month = tm.tm_mon+1;
  int year = tm.tm_year-100;
  if (DEBUG)
    {
      Serial.print("Date is ");
      Serial.print(day);
      Serial.print("-");
      Serial.print(month);
      Serial.print("-");
      Serial.println(year);
    }
    bitbang_digit(day / 10);
    bitbang_digit(day % 10);
    bitbang_digit(month/ 10);
    bitbang_digit(month % 10);
    bitbang_digit((year-2000) / 10);
    bitbang_digit((year-2000) % 10);
}

//Calls porvided function and writes to latch
void use_func(void (*func)(void)){
  func();
  digitalWrite(latch, HIGH);
  digitalWrite(latch, LOW);
}

void clear_display() {
  for(int i=0;i<6;i++){
    bitbang_bit(LOW);
    bitbang_bit(HIGH);
    bitbang_bit(HIGH);
    bitbang_bit(HIGH);
    bitbang_bit(HIGH);
    bitbang_bit(LOW);
    bitbang_bit(LOW);
    bitbang_bit(LOW);
  }
  digitalWrite(latch, HIGH);
  digitalWrite(latch, LOW);
}

int counter = 0;

void ICACHE_RAM_ATTR dimmerTimerCallback() {
    dimmer_interrupt_count ++;
    int counter = dimmer_interrupt_count & 0xff;

    if (counter < dimming_duty_cycle) {
        dimmer_duty_cycle_count++;
    }
    if (counter == 0) {
        int m = millis() % 1000;
        blink = m < 500;
        time(&no);
        localtime_r(&no,&tm);
        use_func(dump_time);
        return;
    }
    if (counter == dimming_duty_cycle) {
        clear_display();
        return;
    }
}

void handleMetrics(){
  String buf = "";
  buf += "nixie_boot_time ";
  buf += boot_time;
  buf += "\n";

  buf += "nixie_ntp_sync_count ";
  buf += ntp_sync_count;
  buf += "\n";

  buf += "nixie_sketch_size_bytes ";
  buf += ESP.getSketchSize();
  buf += "\n";

  buf += "nixie_flash_space_bytes ";
  buf += ESP.getFlashChipRealSize();
  buf += "\n";

  buf += "nixie_free_heap_bytes ";
  buf += ESP.getFreeHeap();
  buf += "\n";


  buf += "nixie_dimmer_interrupt_count ";
  buf += dimmer_interrupt_count;
  buf += "\n";

  buf += "nixie_dimmer_duty_cycle_count ";
  buf += dimmer_duty_cycle_count;
  buf += "\n";

  wm.server->send(200, "text/plain", buf);
}

const char dimmerSliderSnippet[] = R"(
  <br/>
  <label for='dimming_duty_cycle_slider'>Dimming duty cycle</label>
  <input type="range" min="1" max="255" value="127" class="slider" id="dimming_duty_cycle_slider" onchange="document.getElementById('dimming_duty_cycle').value = this.value">
  <script>
  document.getElementById('dimming_duty_cycle').hidden = true;
  </script>
  )";
  
WiFiManagerParameter param_dimming_duty_cycle_slider(dimmerSliderSnippet);
WiFiManagerParameter param_dimming_duty_cycle("dimming_duty_cycle", "", "1000", 4);

void ICACHE_RAM_ATTR saveParamsCallback() {
  Serial.print("Dimming");
  dimming_duty_cycle = String(param_dimming_duty_cycle.getValue()).toInt();
  Serial.println(dimming_duty_cycle);
}

void setup() {
  Serial.begin(115200);
  
  //Set the pins
  pinMode(clock1, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(data, OUTPUT);
  digitalWrite(clock1, LOW);
  digitalWrite(latch, LOW);
  digitalWrite(data, LOW);
  
  //Uncomment and run it once, if you want to erase all the stored information.
  wm.resetSettings();

  //This is for getting the display format
  const char *time_select_str = R"(
  <label for='display'>Clock display format</label>
  <select name="timeDisplay" id="display" onchange="document.getElementById('key_custom').value = this.value">
    <option value="0">Time</option>
    <option value="1">Date</option>
    <option value="2">Time and Date</option>
  </select>
  <script>
    document.getElementById('display').value = "%d";
    document.querySelector("[for='key_custom']").hidden = true;
    document.getElementById('key_custom').hidden = true;
  </script>
  )";

  //Make the parameter with an initial value
  char displayBuffer[700];
  sprintf(displayBuffer, time_select_str, displayFormat);
  WiFiManagerParameter displayField(displayBuffer);

  //Create a hidden parameter to get selection from page
  char defaultValue[30];
  sprintf(defaultValue, "%d", displayFormat); // Need to convert to string to display a default value.
  WiFiManagerParameter displayData("key_custom", "Will be hidden", defaultValue, 2);

  //This is for getting the timezone
  WiFiManagerParameter timezoneField(timezones);

  WiFiManagerParameter timezoneData("key_custom2", "Will be hidden", "CET-1CEST,M3.5.0,M10.5.0/3", 30);

  //Add fields
  wm.addParameter(&displayData);
  wm.addParameter(&displayField);
  wm.addParameter(&timezoneData);
  wm.addParameter(&timezoneField);
  wm.addParameter(&param_dimming_duty_cycle);
  wm.addParameter(&param_dimming_duty_cycle_slider);
  wm.setShowInfoUpdate(false); // https://github.com/tzapu/WiFiManager/issues/1262
  wm.setShowInfoErase(false);
  wm.setSaveParamsCallback(saveParamsCallback);

  //Retrieve device MAC address in bytes.
  byte macAdr[6];
  WiFi.macAddress(macAdr);
  
  //connName size is length on "NixieClock-" + 12 MAC chars and 0 terminator.
  char connName[24] = "NixieClock-";
  mac_address(macAdr,connName);
  
  //Create connection point.
  wm.setConfigPortalBlocking(false);
  wm.autoConnect(connName);
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");

  //Timezone and NTP configuration
  configTime(timezoneData.getValue(), MY_NTP_SERVER);
  settimeofday_cb(time_is_set);
  displayFormat = (format)atoi(displayData.getValue());

  time(&no);
  localtime_r(&no,&tm);

  Serial.println("Starting config portal");

  wm.startConfigPortal();
  wm.server->on("/metrics", handleMetrics);

  Serial.println("Starting dimmer timer");
  ITimer.setInterval(dimming_timer_period, dimmerTimerCallback);
}

//Main loop
void loop(){
  
  wm.process();
}
