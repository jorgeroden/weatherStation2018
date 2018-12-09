/* WeatherStation.ino - Weather station for high school STEM project.
 * The primary purpose of this STEM project is learning by making a
 * weather station to monitor air temperature, humidity
 * and barometric pressure.
 * How does it work?
 * An IoT board measures data from sensors and send them to the cloud so
 * that everyone can monitor this data with a smartdevice (phone, tablet).
 * For this project we use a ESP8266 based board
 * two cheap sensors BMP280 & DS18S20 and Thinkspeak.com IoT open platform
 * for data logging on the cloud.
 * Microcontroller is in sleep mode for energy saving.
 * It wakes up every 15 min to measure and send the data
 * (no data logging over the board).
 * For each physical property it gathers 50 samples and send the median value
 * to the cloud.
 *
 * Barometric pressure / Humidity sensor: BMP280
 * Temperature sensor: DS18S20
 * Pin connections for board Wemos D1 mini (ESP8266):
 * BMP280 -> Wemos board
 * SCL -> D1
 * SDA -> D2
 * 3.3V / GND
 * DS18 (one wire) -> Wemos board
 * Data -> D4
 * 3.3V / GND
 * Wemos -> Wemos
 * D0 -> RST (WakeUp_Pullup)
 * A0 -> Raindrops module (5V supply)
 *
 *
 *Pin connections for board Crowtail ESP8266 NODEMCU
 * www.elecrow.com/
 * 
 * BMP 280 on Groove I2C (GPIO14 - SCL, GPIO12 - SDA) 
 * DS18x20 on D2 (GPIO4)
 * 
 * WARNING (Only for crowtail board): 
 * SparkFun library is used for BMP280 sensor,
 * and SparkFunBME280.c file must be edited 
 * to set up I2C pins.
 * Wire.begin(12,14);// (SDA,SCL)
 * instead of Wire.begin();
 * 
 * COMPILER PARAMETERS ARDUINO IDE:
 * 
 * BOARD: Generic ESP8266 Module
 * Flash size: 4M(1M SPIFFS)
 * Reset method: nodemcu
 *
 *
 * Created for STEM teaching purpose by Jorge Munoz, Dec the 2nd 2017.
 * Released into the public domain.

*/

#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <OneWire.h>
#include "Wire.h"
#include <DallasTemperature.h>
#include "SparkFunBME280.h"
#include "Median.h"
//#define ONE_WIRE_BUS 2  //WEMOS board
//#define ONE_WIRE_BUS 4  //CROWTAIL board (D2) 
#define ONE_WIRE_BUS 2  //ADAFRUIT HUZZAH 

  
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18x20(&oneWire);
DeviceAddress insideThermometer;
WiFiClient client;
Median m;
BME280 bmp280;


const char* myAPIKey = "N6BG9PLJUYQY9RIN";
unsigned long myChannel = 207881;

//const char* ssid = "here-SSID-name";
//const char* password = "here-password";
//const char* ssid = "LPCA-iPad";
//const char* password = "l3g@cy1p@d16!";
//const char* ssid = "legacy-test";
//const char* password = "legacytest";
//const char* ssid = "legacy-guest";
//const char* password = "Gu3$tGu3$t";

const char* ssid = "FiOS-0EUZD";
const char* password = "rodeo34fog6382plea";

const char* server = "api.thingspeak.com";
//const int sleepMicroSec = 900e6; //15 min sleeping. Connect D0 to RST to wake up
const int sleepMicroSec1 = 60e6; //15 min sleeping. Connect D0 to RST to wake up
const int sleepMicroSec2 = 30e6; //2 min sleeping when WiFi connection fails. Connect D0 to RST to wake up

const int samples = 50;  //greater numbers can overflow memory
float temDS18[samples];  // Celsius DS18
float temBMP280[samples];//Celsius BMP280 
float preBMP280[samples]; // Pascal
float humBMP280[samples]; // %
float nRainVal[samples];

float tempC,pressure, humidity, tempCBMP, tempF, tempFBMP;

IPAddress ipZero(0,0,0,0);
// --- raindrops module variables ----
//int nRainIn = A0;
//float rainValue;
//String strRaining;
//------------------------------------
void setup() {

  Serial.begin(9600);
  pinMode(0, WAKEUP_PULLUP);   
  ds18x20.setResolution(insideThermometer, 12); //11 bits resolution 0.125°C 375 ms
  bmp280.settings.commInterface = I2C_MODE;
  bmp280.settings.I2CAddress = 0x76;
  bmp280.settings.runMode = 3; //Normal mode
  bmp280.settings.tStandby = 0; //tStandby 0.5ms
  bmp280.settings.filter = 0; // filter off
  bmp280.settings.tempOverSample = 1;
  bmp280.settings.pressOverSample = 1; // 1 through
  bmp280.settings.humidOverSample = 1; //1 through
  Serial.println("-----------------------------------------");  
  Serial.println("\n\nWake up");
  Serial.print("Program Started\n");
  Serial.print("Starting BME280... result of .begin(): 0x");
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  Serial.println(bmp280.begin(), HEX);
  // Wake up every period (defined in var sleepMicroSec)
  // and takes a number of samples (samples var).
  // Calculates median value of each physical property
  
  for (int i = 0; i < samples; i++){

      preBMP280[i] = bmp280.readFloatPressure(); // Pa
      humBMP280[i] = bmp280.readFloatHumidity(); // %
      temBMP280[i] = bmp280.readTempC();//Celsius
      ds18x20.requestTemperatures();
      temDS18[i] = ds18x20.getTempCByIndex(0);
      //nRainVal[i] = analogRead(nRainIn);
      
  }
  
  tempC = m.median(temDS18, 0, samples - 1);
  pressure = m.median(preBMP280, 0, samples - 1);
  humidity = m.median(humBMP280, 0, samples - 1);
  tempCBMP = m.median(temBMP280, 0, samples - 1);
  //float rainValue = m.median(nRainVal, 0, samples - 1);
  pressure = pressure * 0.01; // Pascal to milibar
  tempF = (1.8 * tempC) + 32;
  tempFBMP = (1.8 * tempCBMP) + 32;
  
  // If ESP-12 can not be coneccted to WIFI access point
  // battery will be discharged. Then if the connection
  // waiting time is over 1 min 30 secs then go to sleep mode till next
  // time cycle (defined in var sleepMicroSec).

 /* Serial.println("-----------------------------------------");  
  Serial.println("\n\nWake up");
  WiFi.begin(ssid, password);  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  */
  /*while (WiFi.status() != WL_CONNECTED) {
   delay(1000);
   Serial.print(wifiAttempts);Serial.println(" of 60");
   wifiAttempts ++;
   if (wifiAttempts > 60){ //More than 1 min waiting time
                           //then sleep mode.
     Serial.println(".");
     
     Serial.println("NO WIFI CONNECTION, GOING TO SLEEP MODE TO SAVE BATTERY.");
     Serial.println("THESE DATA HAVE NOT BEEN SENT TO THE CLOUD IN THIS CYCLE.");
     showMeasurements();
     ESP.deepSleep(sleepMicroSec);
    }
   }*/
  
  /*if (WiFi.localIP() == ipZero ){
    Serial.println("");
    Serial.println("waiting for an IP Address...");
    
 
    }
  else{
   */
  WiFi.begin(ssid, password); 
  Serial.println("");
  Serial.println("waiting for an IP Address...");
  
  while (WiFi.localIP() == ipZero) { //The formula for retries
                                     // WiFi.status() != WL_CONNECTION
                                     // seems not to work. 
    
     Serial.println("NO WIFI CONNECTION, GOING TO SLEEP MODE TO SAVE BATTERY.");
     Serial.println("THESE DATA HAVE NOT BEEN SENT TO THE CLOUD IN THIS CYCLE.");
     Serial.println("I WILL WAKE UP IN 2 MINUTES TO TRY IT AGAIN.");
     showMeasurements();
     ESP.deepSleep(sleepMicroSec2);
       
  }
  
   Serial.println("");
   Serial.println("WiFi connected, sending data to the cloud...");
   Serial.print("IP address :\t");  
   Serial.println(WiFi.localIP());
   showMeasurements();
   ThingSpeak.begin(client);   
   ThingSpeak.setField(1, tempF);
   ThingSpeak.setField(2, pressure);
   ThingSpeak.setField(3, humidity);
   ThingSpeak.setField(4, tempFBMP);
   ThingSpeak.writeFields(myChannel, myAPIKey);
  
   Serial.println("SLEEPING... I WILL WAKE UP IN 15 MINUTES.");
   ESP.deepSleep(sleepMicroSec1);
  //}
    
  

}
void showMeasurements(){
  
  Serial.print("T Celsius (DS18 sensor) = \t");Serial.println(tempC);
  Serial.print("T Farenh (DS18 sensor) = \t");Serial.println(tempF);
  Serial.print("P (mbar) (BMP280 sensor) = \t");Serial.println(pressure);
  Serial.print("HR (%) (BMP280 sensor) = \t");Serial.println(humidity);
  Serial.print("Temp (BMP280 sensor) = \t");Serial.println(tempCBMP);
 
  }

void loop() {


}
