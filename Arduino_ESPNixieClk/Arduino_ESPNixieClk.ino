/*
https://github.com/laurivosandi/nixiesp12/blob/master/firmware/main.py
https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/
*/

#include <WiFiManager.h>
#include <TimeLib.h>

//NTP server and refresh interval.
#define MY_NTP_SERVER "at.pool.ntp.org"
#define updateInterval 360000

//Assign output variables to GPIO pins.
const int clock1 = 3;
const int data = 2;
const int latch = 0;

//Debug settings.
const bool DEBUG = true;
const bool DEBUG_PIN = false;

//Timer, timer interval, display toggle
unsigned long prevMil = 0;
unsigned long prevMil_10 = 0;
const long interval = 1000;
bool toggleDisplay = false;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char timeZone[][30] = {"EET-2EEST,M3.5.0/3,M10.5.0/4","GMT0BST,M3.5.0/1,M10.5.0","CET-1CEST,M3.5.0,M10.5.0/3"};

enum format{
  TIME,
  DATE,
  DATETIME
};
enum format displayFormat = TIME;

bool blink = false;
int lookup[] = {11, 9, 12, 8, 0, 4, 1, 3, 2, 10};

//Time structs
time_t no;
tm tm;

//"weak" function to set NTP update interval
uint32_t sntp_update_delay_MS_rfc_not_less_than_15000(){
  return 8 * 60 * 60 * 1000ul;//8 hours 
}

void time_is_set(){
  Serial.println("Update NTP connection.");
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

void setup() {
  Serial.begin(115200);
  
  //Set the pins
  pinMode(clock1, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(data, OUTPUT);
  digitalWrite(clock1, LOW);
  digitalWrite(latch, LOW);
  digitalWrite(data, LOW);

  //Input for display format in WifiManager. Limit characters to 4.
  //WiFiManagerParameter custom_output("State", "output", output, 4);
  WiFiManager wifiManager;
  
  //Uncomment and run it once, if you want to erase all the stored information.
  wifiManager.resetSettings();
  
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
  WiFiManagerParameter display_field(displayBuffer);

  //Create a hidden parameter to get selection from page
  char convertedValue[16];
  sprintf(convertedValue, "%d", displayFormat); // Need to convert to string to display a default value.
  WiFiManagerParameter display_data("key_custom", "Will be hidden", convertedValue, 2);
  
  //This is for getting the timezone
  const char *tz_select_str = R"(
  <br/><label for='zone'>Timezone selection</label>
  <select name="timeZone" id="zone" onchange="document.getElementById('key_custom2').value = this.value">
    <option value="0">Estonia</option>
    <option value="1">London</option>
    <option value="2">Paris</option>
  </select>
  <script>
    document.getElementById('zone').value = "%d";
    document.querySelector("[for='key_custom2']").hidden = true;
    document.getElementById('key_custom2').hidden = true;
  </script>
  )";

  //Make the parameter with an initial value
  char timezoneBuffer[700];
  sprintf(timezoneBuffer, tz_select_str, 0);
  WiFiManagerParameter timezone_field(timezoneBuffer);

  //Create a hidden parameter to get selection from page
  sprintf(convertedValue, "%d", 0); // Need to convert to string to display a default value.
  WiFiManagerParameter timezone_data("key_custom2", "Will be hidden", convertedValue, 2);

  //Add fields
  wifiManager.addParameter(&display_data);
  wifiManager.addParameter(&display_field);
  wifiManager.addParameter(&timezone_data);
  wifiManager.addParameter(&timezone_field);

  //Retrieve device MAC address in bytes.
  byte macAdr[6];
  WiFi.macAddress(macAdr);
  
  //connName size is length on "NixieClock-" + 12 MAC chars and 0 terminator.
  char connName[24] = "NixieClock-";
  mac_address(macAdr,connName);
  
  //Create connection point.
  wifiManager.autoConnect(connName);
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");

  //Timezone and NTP configuration
  configTime(timeZone[atoi(timezone_data.getValue())], MY_NTP_SERVER);
  settimeofday_cb(time_is_set);
  
  displayFormat = (format)atoi(display_data.getValue());
}

//Main loop
void loop(){
  
  //If displaying date and time, then switch the deisplay every 10 seconds.
  if(millis() - prevMil >= interval){
    prevMil = millis();
    time(&no);
    localtime_r(&no,&tm);
    
    //Check the display format and output appropriate info.
    switch(displayFormat){
      case 0:
        use_func(dump_time);
        break;
      case 1:
        use_func(dump_date);
        break;
      case 2:
        if(toggleDisplay){
          use_func(dump_date);
        }else{
          use_func(dump_time);
        }
        break;
      default:
        Serial.println("Display mode error.");
        break;
    }
    blink = !blink;
  }

  if(millis() - prevMil_10 >= 10*interval){
    prevMil_10 = millis();
    toggleDisplay = !toggleDisplay;
  }
}
