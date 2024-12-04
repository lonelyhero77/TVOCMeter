#include <BMP180I2C.h>
#define I2C_ADDRESS 0x77



#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C 

#include <Adafruit_SGP30.h>
//#include <Adafruit_AHTX0.h>
//#include <Adafruit_BMP085.h>
//#include <Adafruit_NeoPixel.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.
#define DHTTYPE    DHT11     // DHT 11

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SGP30 sgp;
DHT_Unified dht(DHTPIN, DHTTYPE);
//Adafruit_BMP085 bmp;
BMP180I2C bmp180(I2C_ADDRESS);

uint32_t getAbsoluteHumidity(float temperature, float humidity){
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}

uint32_t delayMS;
uint32_t counter;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial){
    delay(10); // Wait for serial console to open!
  }
  Serial.println("Checking up the sensors...");
  
  //I2C Initialization
  Wire.begin();
  
  //DHT11 Initialization
  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  
  //SGP30 Initialization
  if(!sgp.begin()){
    Serial.println("SGP30 Sensor not found");
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!

  //SSD1306 Initialization
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("SSD1306 Initialization failed");
    while(true);
  }
  display.display(); // Shows splash
  delay(500);
  display.clearDisplay();

  //BMP180 Initialization
  if(!bmp180.begin()){
    Serial.println("Couldn't find a valid BMP180 sensor, check wiring!");
//    while(true);
  }
  bmp180.resetToDefaults();
  bmp180.setSamplingMode(BMP180MI::MODE_UHR);
}

int idx = 0;
void loop() {
//   Delay between measurements.
//  delay(delayMS);
//  // Get temperature event and print its value.
//  sensors_event_t event;
//  dht.temperature().getEvent(&event);
//  if (isnan(event.temperature)) {
//    Serial.println(F("Error reading temperature!"));
//  }
//  else {
//    Serial.print(F("Temperature: "));
//    Serial.print(event.temperature);
//    Serial.println(F("°C"));
//  }
//  // Get humidity event and print its value.
//  dht.humidity().getEvent(&event);
//  if (isnan(event.relative_humidity)) {
//    Serial.println(F("Error reading humidity!"));
//  }
//  else {
//    Serial.print(F("Humidity: "));
//    Serial.print(event.relative_humidity);
//    Serial.println(F("%"));
//  }

  // SENSOR DATA MINING
  // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals

  // total vocs
  // 0-300ppb Excellent
  // 300-750 ppb Acceptable(to be monitored)
  // 750+ ppb Dangerous (Immediate action required) 
  if(!bmp180.measurePressure()){
    Serial.println("could not start perssure measurement, is a measurement already running?");
    return;
  }
  //wait for the measurement to finish. proceed as soon as hasValue() returned true. 
  do
  {
    delay(100);
  } while (!bmp180.hasValue());
//  float pressure = bmp180.getPressure() / 100.0;
  float pressure = bmp180.getPressure();
  
//  float pressure = bmp.readPressure()/100.0; // change scale Pa to hPa
  if (!bmp180.measureTemperature())
    {
      Serial.println("could not start temperature measurement, is a measurement already running?");
      return;
    }
  
    //wait for the measurement to finish. proceed as soon as hasValue() returned true. 
    do
    {
      delay(100);
    } while (!bmp180.hasValue());
  float bmpTemp = bmp180.getTemperature();
  float dhtTemp = getDHTTemp();
  float avgTemp = (bmpTemp+dhtTemp)/2.0; // [°C]
  float humidity = getDHTHumidity(); // [%RH]
  sgp.setHumidity(getAbsoluteHumidity(avgTemp, humidity));
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  float tvoc = sgp.TVOC; // ppb to ppm conversion
  float eCO2 = sgp.eCO2;
  float rawH2 = sgp.rawH2;
  float rawEthanol = sgp.rawEthanol;
  
  Serial.print("TVOC "); Serial.print(tvoc); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(eCO2); Serial.println(" ppm");
  Serial.print("Raw H2 "); Serial.print(rawH2); Serial.print(" \t");
  Serial.print("Raw Ethanol "); Serial.print(rawEthanol); Serial.println("");
  
  Serial.print("Pressure "); Serial.println(pressure);
  Serial.print("BMP180 Temp: "); Serial.println(bmpTemp);
  Serial.print("DHT11 Temp: "); Serial.println(dhtTemp);
  Serial.print("DHT11 Humidity: "); Serial.println(humidity);
  Serial.print("Avg Temp: "); Serial.println(avgTemp);
 
  delay(delayMS);

//  counter++;
//  if (counter == 30) {
//    counter = 0;
//
//    uint16_t TVOC_base, eCO2_base;
//    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
//      Serial.println("Failed to get baseline readings");
//      return;
//    }
//    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
//    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
//  }
////
//  //DISPLAY
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    // display.setTextColor(WHITE, BLACK);
    display.setCursor(0, 0);
    // display.print("Sensing...");
//    printBattery();
    printAlert(tvoc);
    
    // Display important sensing value
    display.setCursor(0, 8);
    printTVOC(tvoc);
    
    // Display additional sensing values
    if( idx == 0 ){
      display.setCursor(0, 16);
      printeCO2(eCO2);
      display.setCursor(0, 24);
      printPressure(pressure);
    }
    if( idx == 1){
      display.setCursor(0, 16);
      printTemp(avgTemp);
      display.setCursor(0, 24);
      printHumidity(humidity);
    }
    
    display.display();
    
    idx++;
    if( idx == 2 ){ idx = 0; }
    delay(1000);
}

void printBattery(){
  display.setTextColor(WHITE, BLACK);
  int sensorValue = analogRead(A0); //read the A0 pin value
  float voltage = sensorValue * (5.00 / 1023.00);
  // float voltage = sensorValue * (5.00 / 1023.00) * 2; //convert the value to a true voltage. USE THIS WHEN V_IN IS 9V AND USING VOLTAGE DIVIDER!
  display.setCursor(90,0);
  display.print(voltage); //print the voltage to LCD
  display.println(" V");

  //full charged alert
  if (voltage > 4.15){
    display.setCursor(0, 0);
    display.print("FULLY CHARGED");
  }

  //low battery alert
  if (voltage < 3.50){
    display.setCursor(0, 0);
    display.print("LOW BATTERY");
  }
}

void printAlert(float sensorValue){
  // total vocs
  // 0-300ppb Excellent
  // 300-750 ppb Acceptable(to be monitored)
  // 750+ ppb Dangerous (Immediate action required) 
  display.setCursor(0, 0);
  if(sensorValue < 300.0){
    display.print("TVOC is Good");
  }
  if(sensorValue > 300.0 && sensorValue < 750.0){
    display.print("TVOC is OK");
  }
  if(sensorValue > 750.0){
    display.print("TVOC is bad!");
  }
}

void printPressure(long sensorValue){
  display.print("Pressure: ");
  display.print(sensorValue);
//  display.print(" hPa");
  display.print(" Pa");
}

void printTVOC(float sensorValue){
  // display.println("TVOC:             ppb");
  display.print("TVOC:     ");
  display.print(sensorValue);
  display.print(" ppb");
}

void printeCO2(float sensorValue){
  // display.println("CO2eq:            ppm");
  display.print("CO2eq:    ");
  display.print(sensorValue); 
  display.print(" ppm");
}

void printTemp(int sensorValue){
  display.print("Temp:           ");
  display.print(sensorValue);
  // display.print(char(248), "C");
  // display.print((char)248, "C");
  display.print(" `C");
  // display.print((char)247, "C");
}

void printHumidity(int sensorValue){
  display.print("Humidity:        ");
  display.print(sensorValue);
  display.print(" %");
}

float getDHTTemp(){
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if(isnan(event.temperature)){
    Serial.println(F("Error reading temperature!"));
  }else{
    return event.temperature;
  }
}

float getDHTHumidity(){
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if(isnan(event.relative_humidity)){
    Serial.println(F("Error reading humidity!"));
  }else{
    return event.relative_humidity;
  }
}
