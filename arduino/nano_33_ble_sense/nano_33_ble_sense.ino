/****************************************
 *  Getting started with the Arduino NANO 33 BLE Sense link：https://www.arduino.cc/en/Guide/NANO33BLESense
 *  nRF52840：https://github.com/arduino/ArduinoCore-nRF528x-mbedos
 *  RGB LED link：https://support.arduino.cc/hc/en-us/articles/360016724140-How-to-control-the-RGB-LED-and-Power-LED-of-the-Nano-33-BLE-boards-
 *  HTS221 温湿度传感器 link：https://www.arduino.cc/en/Reference/ArduinoHTS221
 *  LPS22HB 压力传感器 link：https://www.arduino.cc/en/Reference/ArduinoLPS22HB
 *  APDS9960 手势、颜色、光照强度和接近度传感器 link：https://www.arduino.cc/en/Reference/ArduinoAPDS9960 
 *  MP34DT05-A 音频传感器 link：https://www.arduino.cc/en/Reference/PDM
 *  Adafruit ST7735 and ST7789 Library： https://github.com/adafruit/Adafruit-ST7735-Library
 *  BLE link：https://www.arduino.cc/en/Reference/ArduinoBLE
****************************************/

#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_APDS9960.h>
#include <PDM.h>
#include <arduinoFFT.h>
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

/* LED引脚 */
#define LEDR        (22u)
#define LEDG        (23u)
#define LEDB        (24u)
/* TFT引脚 */
#define TFT_CS        (8u)// PyBadge/PyGamer display control pins: chip select
#define TFT_RST      (10u)  // Display reset
#define TFT_DC        (9u) // Display data/command select
#define TFT_BACKLIGHT (7u) // Display backlight pin
/* 噪声采样频率 */
#define SAMPLES 256
#define SAMPLING_FREQUENCY 16000

const int UPDATE_FREQUENCY = 4000;     // Update frequency in ms
const float CALIBRATION_FACTOR = -4.0; // Temperature calibration factor (Celsius)
String previousColor = "";
short previousTemperature = 0;
unsigned short previousHumidity = 0;
unsigned short previousPressure = 0;
unsigned short previousNoise = 0;
long previousMillis = 0; // last time readings were checked, in ms
float temperature = 0;
float humidity = 0;
float pressure = 0;
unsigned short noise = 0;
int r, g, b, a;

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[SAMPLES];
// number of samples read
volatile int samplesRead;

BLEService environmentService("181A"); // Standard Environmental Sensing service

BLEIntCharacteristic tempCharacteristic("2A6E",               // Standard 16-bit Temperature characteristic
                                        BLERead | BLENotify); // Remote clients can read and get updates

BLEUnsignedIntCharacteristic humidCharacteristic("2A6F", // Unsigned 16-bit Humidity characteristic
                                                 BLERead | BLENotify);

BLEUnsignedIntCharacteristic pressureCharacteristic("2A6D",               // Unsigned 32-bit Pressure characteristic
                                                    BLERead | BLENotify); // Remote clients can read and get updates

BLECharacteristic colorCharacteristic("936B6A25-E503-4F7C-9349-BCC76C22B8C3", // Custom Characteristics
                                      BLERead | BLENotify, 24);               // 

BLEUnsignedIntCharacteristic soundCharacteristic("836B6A25-E503-4F7C-9349-BCC76C22B8C3",               // Unsigned 32-bit sound characteristic
                                                    BLERead | BLENotify);                                  

BLEDescriptor colorLabelDescriptor("2901", "16-bit ints: r, g, b, a");

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);
arduinoFFT FFT = arduinoFFT();

void setup() {
  // put your setup code here, to run once:

/* 串口初始化 */
  Serial.begin(9600);
  // while (!Serial);

/* TFT屏幕初始化 */
  tft.init(135, 240); 
  tft.setRotation(3);
//  pinMode(TFT_BACKLIGHT, OUTPUT);
//  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
    
/* RGB 初始化 */
//  pinMode(RED, OUTPUT);
  pinMode(LEDB, OUTPUT);
//  pinMode(GREEN, OUTPUT);
  digitalWrite(LEDB, HIGH);

  /* 温湿度初始化 */
  if (!HTS.begin()) 
  {
    Serial.println(F("Failed to initialize humidity temperature sensor!"));
    while (1);
  }

  /* 压力传感器初始化 */
  if (!BARO.begin()) 
  {
    Serial.println(F("Failed to initialize pressure sensor!"));
    while (1);
  }

 /* 手势、颜色、光照强度和接近度传感器初始化 */ 
  if (!APDS.begin()) 
  {
    Serial.println(F("Error initializing APDS9960 sensor!"));
    while(1);
  }

/* 音频传感器初始化 */
  PDM.onReceive(onPDMdata);
  if (!PDM.begin(1, SAMPLING_FREQUENCY)) 
  {
    Serial.println(F("Failed to start PDM!"));
    while (1);
  }

/* 蓝牙初始化 */
  if (!BLE.begin()) 
  {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("WERTHER");
  BLE.setAdvertisedService(environmentService); // Advertise environment service

  // add the characteristic to the service
  environmentService.addCharacteristic(tempCharacteristic);     // Add temperature characteristic
  environmentService.addCharacteristic(humidCharacteristic);    // Add humidity characteristic
  environmentService.addCharacteristic(pressureCharacteristic); // Add pressure characteristic
  environmentService.addCharacteristic(colorCharacteristic);    // Add color characteristic
  colorCharacteristic.addDescriptor(colorLabelDescriptor); // Add color characteristic descriptor
  environmentService.addCharacteristic(soundCharacteristic);    // Add sound characteristic

  BLE.addService(environmentService); // Add environment service

  // set the initial value for the characeristic:
  tempCharacteristic.setValue(0);     // Set initial temperature value
  humidCharacteristic.setValue(0);    // Set initial humidity value
  pressureCharacteristic.setValue(0); // Set initial pressure value
  colorCharacteristic.setValue("");   // Set initial color value
  soundCharacteristic.setValue(0);    // Set initial sound value
  
  // start advertising
  BLE.advertise();

  Serial.print("Peripheral device MAC: ");
  Serial.println(BLE.address());
  Serial.println("Waiting for connections...");
  
  Serial.println(F("system initialization complete! "));

}

void loop() {
  // put your main code here, to run repeatedly:
  
  display_process();
 
  ble_process();

  delay(1000);    // wait a second
}

void onPDMdata() 
{
  // query the number of bytes available
  int bytesAvailable = PDM.available();
  
  // read into the sample buffer
  int bytesRead = PDM.read(sampleBuffer, bytesAvailable);
  
   // 16-bit, 2 bytes per sample
  samplesRead = bytesRead / 2;
}

void display_process(void)
{
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK); 
  tft.setCursor(0, 0);
  tft.print("WERTHER STATION");
  tft.drawLine(0, 16, tft.width() - 1, 16, ST77XX_YELLOW);
  
  temperature = HTS.readTemperature() + CALIBRATION_FACTOR;  
  Serial.print(F("TEMPERATURE:"));
  Serial.print(temperature, 1);
  Serial.println(F(" °C"));
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.setCursor(0, 20);
  tft.print("TEMPERATURE:");
  tft.print(temperature, 1);
  tft.drawCircle(200, 20, 3, ST77XX_RED);
  tft.setCursor(208, 20);
  tft.print("C");
  tft.drawLine(0, 36, tft.width() - 1, 36, ST77XX_YELLOW);

  humidity = HTS.readHumidity();
  Serial.print(F("HUMIDITY:"));
  Serial.print(humidity, 1);
  Serial.println(F(" %"));
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.setCursor(0, 40);
  tft.print("HUMIDITY:");
  tft.print(humidity, 1);
  tft.setCursor(160, 40);
  tft.print("%");
  tft.drawLine(0, 56, tft.width() - 1, 56, ST77XX_YELLOW);
  
  pressure = BARO.readPressure();
  Serial.print(F("PRESSURE:"));
  Serial.print(pressure, 1);
  Serial.println(F(" kPa"));
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.setCursor(0, 60);
  tft.print("PRESSURE:");
  tft.print(pressure, 1);
  tft.setCursor(170, 60);
  tft.print("kPa");
  tft.drawLine(0, 76, tft.width() - 1, 76, ST77XX_YELLOW);
  
  while (! APDS.colorAvailable()) 
  {
    delay(1);
  }
  APDS.readColor(r, g, b, a);
  Serial.print("R:");
  Serial.println(r);
  Serial.print("G:");
  Serial.println(g);
  Serial.print("B:");
  Serial.println(b);
  Serial.print("A:");
  Serial.println(a);
  if(a > 20)
  {
    
    Serial.println(F("DAY"));
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setCursor(0, 80);
    tft.print("AMBIENT:DAY  ");
  }
  else
  {
    Serial.println(F("NIGHT"));
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setCursor(0, 80);
    tft.print("AMBIENT:NIGHT");
  }
  tft.drawLine(0, 96, tft.width() - 1, 96, ST77XX_YELLOW);

  while (!samplesRead);
  uint16_t sample_max = 0;
  for (int i = 0; i < samplesRead; i++) 
  {
    if(sampleBuffer[i] < 0)
      sampleBuffer[i] = -sampleBuffer[i];
    if(sampleBuffer[i] > sample_max) 
      sample_max = sampleBuffer[i];
  }
  noise = 24 * log10(sample_max * 5); 
  samplesRead = 0;
  Serial.println(noise);
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.setCursor(0, 100);
  tft.print("NOISE:");
  uint8_t cnt = 0;
  uint16_t n = noise;
  while(n != 0)
  {
    n /= 10;
    ++cnt;
  }
  if(cnt == 2)
  {
      tft.print(" ");
      tft.print(noise, DEC);
  }
  else
  {
      tft.print(noise, DEC);
  }
  tft.setCursor(110, 100);
  tft.print("dB");
  tft.drawLine(0, 116, tft.width() - 1, 116, ST77XX_YELLOW);
}

void ble_process(void)
{
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if(central)
  {
    Serial.print("Connected to central MAC: ");
    Serial.println(central.address()); // Central's BT address:
    digitalWrite(LEDB, LOW); // Turn on the LED to indicate the connection
    while (central.connected()) 
    {
      long currentMillis = millis();
      // After UPDATE_FREQUENCY ms have passed, check temperature & humidity
      if (currentMillis - previousMillis >= UPDATE_FREQUENCY) 
      {
        previousMillis = currentMillis;
        if (temperature != previousTemperature) 
        { // If reading has changed
          Serial.print("Temperature: ");
          Serial.println(temperature);
          tempCharacteristic.writeValue((int16_t)(temperature * 100)); // Update characteristic
          previousTemperature = temperature;          // Save value
        }
        if (humidity != previousHumidity) 
        { // If reading has changed
            Serial.print("Humidity: ");
            Serial.println(humidity);
            humidCharacteristic.writeValue((uint16_t)(humidity * 100));
            previousHumidity = humidity;
        }
        if (pressure != previousPressure) 
        { // If reading has changed
          Serial.print("Pressure: ");
          Serial.println(pressure);
          pressureCharacteristic.writeValue((uint16_t)(pressure * 100));
          previousPressure = pressure;
        }     
        if(a > 20)
        {
          Serial.print("Day a: ");
          Serial.println(a);
          colorCharacteristic.writeValue((uint8_t)true);
        }
        else
        {
          Serial.print("Night a: ");
          Serial.println(a);
          colorCharacteristic.writeValue((uint8_t)false);
        }
        if(noise != previousNoise) 
        { // If reading has changed
          Serial.print("Noise: ");
          Serial.println(noise);
          soundCharacteristic.writeValue((uint16_t)(noise));
          previousNoise = noise;
        } 
        display_process();
      }
    }
    digitalWrite(LEDB, HIGH); // When the central disconnects, turn off the LED
    Serial.print("Disconnected from central MAC: ");
    Serial.println(central.address());
   }
}


