// Include the ESP8266 WiFi library.
#include <ESP8266WiFi.h>
// SparkFun Phant library.
#include <Phant.h>
// HTU21D Library
#include <SparkFunHTU21D.h>
#include <Wire.h>
// Blynk Library
#include <BlynkSimpleEsp8266.h>
// Epoch Time converter
#include <Time.h>
#include <TimeLib.h>

//Create an instance of the object
HTU21D mySensor;

////////////////////////////////////////////////////////////////////////////
// Blynk //
///////////
// Pin Definitions
#define LED_PIN           5
#define DIGITAL_PIN       12
#define LED_BTN_VAR       V2
#define LED_VAR           V1
#define LCD_VAR           V9

// Keys
const char BLYNK_AUTH[] = "put yourr auth code form blynk app";

// Global variables
WidgetLED board_led(LED_VAR);
WidgetLCD lcd(LCD_VAR);

////////////////////////////////////////////////////////////////////////////
// WiFi Definitions //
//////////////////////
const char WiFiSSID[] = "put your WiFi SSID";
const char WiFiPSK[] = "...and password";


////////////////////////////////////////////////////////////////////////////
// Phant //
///////////
// Keys
const char PhantHost[] = "data.sparkfun.com";
const char PublicKey[] = "put your key here";
const char PrivateKey[] = "...and here";

// Post Timing 
const unsigned long postRate = 60000; //milisecond
unsigned long lastPost = 0;


////////////////////////////////////////////////////////////////////////////
// OpenWeather //
/////////////////
const char OpenWeatherHost[] = "api.openweathermap.org";
const int httpPort = 80;

// Location data of my lab
const float LATITUDE = 0;// use your place 
const float LONGITUDE = 0;

// Constants
const int TIMEOUT     = 300;  // ms
const int BUFFER_SIZE = 22;   // bytes
const int DT_SIZE     = 12;   // bytes
const int TEMP_SIZE   = 7;    // bytes
const int DATE_SIZE   = 11;   // bytes MM/DD/YYYY
const int TIME_SIZE   = 9;    // bytes HH:MM:SS

// Global Variables
char temp[TEMP_SIZE];
time_t epochTime = 0;
char date[DATE_SIZE];
char Time[TIME_SIZE];

// Parsing state machine states
typedef enum {
  PARSER_START,
  PARSER_WEATHER,
  PARSER_DT,
  PARSER_TEMP,
  PARSER_END
} ParserState;
////////////////////////////////////////////////////////////////////////////


void setup() 
{
  initHardware(); // Setup input/output I/O pins
  connectWiFi(); // Connect to WiFi
  mySensor.begin();
  Blynk.config(BLYNK_AUTH);
  uint8_t led = 0;
  while ( Blynk.connect() == false ) 
  {
    led ^= 0x01;
    digitalWrite(LED_PIN, led);
    delay(200);
  }
  lcd.clear();
  digitalWrite(LED_PIN, LOW); // LED on to indicate connect success
}

void loop() 
{
  // This conditional will execute every lastPost milliseconds
  // (assuming the Phant post succeeded).
  if ((lastPost + postRate <= millis()) || lastPost == 0)
  {
    //Serial.println(F("Getting Weather Data!"));
    getWeather();
    // Convert epoch time into human readable format
    epochCoverter();

    //Serial.println(F("Posting to Phant!"));
    if (postToPhant())
    {
      lastPost = millis();
      Serial.println("Post Suceeded!");
    }
    else // If the Phant post failed
    {
      delay(500); // Short delay, then try again
      Serial.println(F("Post failed, will try again."));
    }
  }
  Blynk.run();
}

/****************************************************************
 * Blynk Callbacks
 ***************************************************************/

// Callback when weather button is pushed
BLYNK_WRITE(LED_BTN_VAR) 
{
  if ( param.asInt() == 1 ) 
  {
    board_led.off();
    digitalWrite(LED_PIN, 1);
  }
  else
  {
    board_led.on();
    digitalWrite(LED_PIN, 0);
  }
  lcd.print(0, 0, "Temp: ");
  lcd.print(7, 0, mySensor.readTemperature());
  lcd.print(0, 1, "Humi: ");
  lcd.print(7, 1, mySensor.readHumidity());
}


void connectWiFi()
{
  byte ledStatus = LOW;
  //Serial.println();
  //Serial.println(F("Connecting to: " + String(WiFiSSID)));
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // This initiates a WiFI connection
  // as a WPA, WPA2,or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void initHardware()
{
  Serial.begin(115200);
  pinMode(DIGITAL_PIN, INPUT_PULLUP); // Setup an input to read
  pinMode(LED_PIN, OUTPUT); // Set LED as output
  digitalWrite(LED_PIN, HIGH); // LED off
  // Don't need to set ANALOG_PIN as input, 
  // that's all it can be.
}

int postToPhant()
{
  // LED turns on when we enter, it'll go off when we 
  // successfully post.
  digitalWrite(LED_PIN, LOW);

  WiFiClient phantClient;
  // Declare an object from the Phant library - phant
  Phant phant(PhantHost, PublicKey, PrivateKey);
  

  // Add the four field/value pairs defined by our stream:
  phant.add("date", date);
  phant.add("time", Time);
  phant.add("wetherTemp", temp);
  phant.add("sensorTemp", mySensor.readTemperature());
  phant.add("sensorHumidity", mySensor.readHumidity());
  
  // Now connect to data.sparkfun.com, and post our data:
  if (!phantClient.connect(PhantHost, httpPort)) 
  {
    // If we fail to connect, return 0.
    return 0;
  }
  // If we successfully connected, print our Phant post:
  phantClient.print(phant.post());

  // Read all the lines of the reply from server and print them to Serial
  while(phantClient.available())
  {
    String line = phantClient.readStringUntil('\r');
    //Serial.print(line); // Trying to avoid using serial
  }

  // Before we exit, turn the LED off.
  digitalWrite(LED_PIN, HIGH);
  yield();
  return 1; // Return success
}




// Perform an HTTP GET request to openweathermap.org
int getWeather() 
{  
  unsigned long time_ = millis();
  char buf[BUFFER_SIZE];
  char dt_str[DT_SIZE];
  char readByte;
  char* dummy;

  WiFiClient weatherClient;
  ParserState parser_state = PARSER_START;
  
  
  // Flush read buffer
  weatherClient.flush();
  
  // Set string buffers to null
  memset(dt_str, 0, DT_SIZE);
  
  // Attempt to make a connection to the remote server
  if ( !weatherClient.connect(OpenWeatherHost, httpPort) ) 
  {
    return -1;
  }
  
  // Construct HTTP GET request
  String cmd = "GET /data/2.5/weather?units=metric&lat=";
  cmd += String(LATITUDE);
  cmd += "&lon=";
  cmd += String(LONGITUDE);
  cmd += "&appid=677a825ac1f8b2b0ae519a69fbb09e92";  
  //This is my personal API, you can get your own at https://home.openweathermap.org/users/sign_up
  cmd += " HTTP/1.1\nHost: ";
  cmd += OpenWeatherHost;
  cmd += "\nConnection: close\n\n";
  
  yield();
  
  // Send out GET request to site
  weatherClient.print(cmd);
  
  // Parse data
  while ( (millis() - time_) < TIMEOUT ) 
  {  
    // Get web page data and push to window buffer
    if ( weatherClient.available() ) 
    {
      readByte = weatherClient.read();
      pushOnBuffer(readByte, buf, BUFFER_SIZE);
      time_ = millis();
    }
    
    // Depending on the state of the parser, look for string
    switch ( parser_state ) 
    {
      case PARSER_START:
        if ( memcmp(buf, "\"weather\":", 10) == 0 ) 
        {
          parser_state = PARSER_TEMP;
        }
        break;
      
      
      case PARSER_TEMP:
        if ( memcmp(buf, "\"temp\":", 7) == 0 ) 
        {
          parser_state = PARSER_DT;
          memcpy(temp, buf + 7, 4);
        }
        break;
        
        
      case PARSER_DT:
        if ( memcmp(buf, "\"dt\":", 5) == 0 ) 
        {
          parser_state = PARSER_END;
          memcpy(dt_str, buf + 5, 10); // This assumes 10 digits!
          epochTime = strtol(dt_str, &dummy, 10);
        }
        break;
        
      default:
        break;
    }
  }
  return 1; // Data successfully acquired
}

// Shift buffer to the left by 1 and append new char at end
void pushOnBuffer(char c, char *buf, uint8_t len) 
{
  for ( uint8_t i = 0; i < len - 1; i++ ) 
  {
    buf[i] = buf[i + 1];
  }
  buf[len - 1] = c;
}


void epochCoverter()
{
    TimeElements tm;
    breakTime(epochTime, tm);
    String td;
    td += String(tm.Month)+"/"+String(tm.Day)+"/"+String(tm.Year+1970);
    td.toCharArray(date, DATE_SIZE);
    td = "";
    td += String(tm.Hour-5)+":"+String(tm.Minute)+":"+String(tm.Second);
    td.toCharArray(Time, TIME_SIZE);
}

