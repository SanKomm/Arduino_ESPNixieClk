/*
https://github.com/laurivosandi/nixiesp12/blob/master/firmware/main.py
https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/
*/

/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Time.h>

//NTP offset and refresh interval
#define utcOffsetInSeconds 3*3600
#define eightHrInterval 28800000

// Assign output variables to GPIO pins
const int clock1 = 3;
const int data = 2;
const int latch = 0;
const bool DEBUG = false;
const bool DEBUG_PIN = false;

//Timer, timer interval, display toggle
unsigned long prevMil = 0;
const long interval = 10000;
bool toggleDisplay = false;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, eightHrInterval);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

void setup() {
  Serial.begin(115200);
  
  //Sätesta väljaviigud
  pinMode(clock1, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(data, OUTPUT);
  digitalWrite(clock1, LOW);
  digitalWrite(latch, LOW);
  digitalWrite(data, LOW);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  
  timeClient.begin();
  server.begin();
  
}

bool TIME = true;
bool DATE = false;
bool DATETIME = false;
bool blink = false;
int lookup[] = {11, 9, 12, 8, 0, 4, 1, 3, 2, 10};

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

void bitbang_digit(int digit){
  int i = 0;
  bitbang_bit(blink);
  for(i=0;i<4;i++){//siin oli error 0-3 ulatus
    bitbang_bit(lookup[digit] << i >> 3);
  }
  bitbang_bit(blink);
  bitbang_bit(blink);
  bitbang_bit(blink);
}

void dump_time(int hour, int minute, int second)
{    
  if (DEBUG)
    {
      Serial.print(daysOfTheWeek[timeClient.getDay()]);
      Serial.print(" ");
      Serial.println(timeClient.getFormattedTime());
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

void dump_date(int day, int month, int year)
{
  if (DEBUG)
    {
      time_t epochTime = timeClient.getEpochTime();
      Serial.println(epochTime);
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
    bitbang_digit(year / 10);
    bitbang_digit(year % 10);
}

void loop(){
  timeClient.update();
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            if (header.indexOf("GET /DATE/") >= 0) {
              Serial.println("Date");
              DATE = true;
              TIME = false;
              DATETIME = false;
            } else if (header.indexOf("GET /TIME/") >= 0) {
              Serial.println("TIME");
              DATE = false;
              TIME = true;
              DATETIME = false;
            } else if (header.indexOf("GET /DATETIME/") >= 0){
              Serial.println("DATE/TIME");
              DATE = false;
              TIME = false;
              DATETIME = true;
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");

            client.println("<p>Date or Time Display setting:</p>");
            if (DATE) {
              client.println("<p><a href=\"/DATETIME/\"><button class=\"button\">DATE</button></a></p>");
            } else if (TIME) {
              client.println("<p><a href=\"/DATE/\"><button class=\"button\">TIME</button></a></p>");
            } else if (DATETIME){
              client.println("<p><a href=\"/TIME/\"><button class=\"button\">DATE/TIME</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  int hour = timeClient.getHours();
  int minute = timeClient.getMinutes();
  int second = timeClient.getSeconds();

  time_t epochTime = timeClient.getEpochTime(); //u. long annab vale aja, peab olema time_t
  struct tm *ptm = gmtime(&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year + 1900;

  if(millis() - prevMil >= interval){
    prevMil = millis();
    toggleDisplay = !toggleDisplay;
    Serial.println(toggleDisplay);
  }

  blink = !blink;
  if(TIME){
    dump_time((hour) % 24, minute, second);
    digitalWrite(latch, HIGH);
    digitalWrite(latch, LOW);
  }else if(DATE){
    dump_date(monthDay, currentMonth, (currentYear-2000));
    digitalWrite(latch, HIGH);
    digitalWrite(latch, LOW);
  }else if(DATETIME){
    if(toggleDisplay){
      dump_date(monthDay, currentMonth, (currentYear-2000));
      Serial.println("Showing date");
    }else{
      dump_time((hour) % 24, minute, second);
      Serial.println("Showing time");
    }

    digitalWrite(latch, HIGH);
    digitalWrite(latch, LOW);
  }

  delay(1000);
}
