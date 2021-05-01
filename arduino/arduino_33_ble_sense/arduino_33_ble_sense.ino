
/****************************************
 * Getting started with the Arduino NANO 33 BLE Sense link：https://www.arduino.cc/en/Guide/NANO33BLESense
 *  nRF52840：https://github.com/arduino/ArduinoCore-nRF528x-mbedos
 *  RGB LED link：https://support.arduino.cc/hc/en-us/articles/360016724140-How-to-control-the-RGB-LED-and-Power-LED-of-the-Nano-33-BLE-boards-
 *  HTS221 温湿度传感器 link：https://www.arduino.cc/en/Reference/ArduinoHTS221
 *  LPS22HB 压力传感器 link：https://www.arduino.cc/en/Reference/ArduinoLPS22HB
 *  APDS9960 手势、颜色、光照强度和接近度传感器 link：https://www.arduino.cc/en/Reference/ArduinoAPDS9960 
 *  MP34DT05-A 音频传感器 link：https://www.arduino.cc/en/Reference/PDM
 *  Adafruit ST7735 and ST7789 Library： https://github.com/adafruit/Adafruit-ST7735-Library
****************************************/

#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_APDS9960.h>
#include <PDM.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[256];

// number of samples read
volatile int samplesRead;

#define LEDR        (22u)
#define LEDG        (23u)
#define LEDB        (24u)
#define D8              (8u)

#define TFT_CS        (8u)
#define TFT_RST      (10u) // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        (9u)

#define TFT_CS        (8u)// PyBadge/PyGamer display control pins: chip select
#define TFT_RST      (10u)  // Display reset
#define TFT_DC        (9u) // Display data/command select
#define TFT_BACKLIGHT (7u) // Display backlight pin


Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);
float p = 3.1415926;

void setup() {
  // put your setup code here, to run once:

/* 串口初始化 */
  Serial.begin(9600);
  while (!Serial);

  tft.init(135, 240); 
  tft.setRotation(3);
//  pinMode(TFT_BACKLIGHT, OUTPUT);
//  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
  tft.fillScreen(ST77XX_BLACK);
  
//  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
//  tft.println("Hello World!sssssssssssssssssssssss");
//  tft.setTextColor(ST77XX_GREEN);
//  tft.print(p, 6);
  
/* RGB 初始化 */
//  pinMode(RED, OUTPUT);
  pinMode(LEDB, OUTPUT);
//  pinMode(GREEN, OUTPUT);

  /* 温湿度初始化 */
 if (!HTS.begin()) {
    Serial.println(F("Failed to initialize humidity temperature sensor!"));
    while (1);
  }

  /* 压力传感器初始化 */
   if (!BARO.begin()) {
    Serial.println(F("Failed to initialize pressure sensor!"));
    while (1);
  }

 /* 手势、颜色、光照强度和接近度传感器初始化 */ 
    if (!APDS.begin()) {
    Serial.println(F("Error initializing APDS9960 sensor!"));
    while(1);
  }

/* 音频传感器初始化 */
    PDM.onReceive(onPDMdata);
    if (!PDM.begin(1, 16000)) {
    Serial.println(F("Failed to start PDM!"));
    while (1);
  }

  Serial.println(F("system initialization complete! "));

}

void loop() {
  // put your main code here, to run repeatedly:

  float temperature = HTS.readTemperature();  
  Serial.print(F("Temperature = "));
  Serial.print(temperature, 1);
  Serial.println(F(" °C"));
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("Temperature = ");
  tft.print(temperature, 1);
//  tft.println(" °C");

  float humidity    = HTS.readHumidity();
  Serial.print(F("Humidity    = "));
  Serial.print(humidity, 1);
  Serial.println(F(" %"));

  float pressure = BARO.readPressure();
  Serial.print(F("Pressure = "));
  Serial.print(pressure, 1);
  Serial.println(F(" kPa"));

  while (! APDS.colorAvailable()) {
    delay(5);
  }
  
  int r, g, b, a;
  APDS.readColor(r, g, b, a);
  Serial.print("r = ");
  Serial.println(r);
  Serial.print("g = ");
  Serial.println(g);
  Serial.print("b = ");
  Serial.println(b);
  Serial.print("a = ");
  Serial.println(a);

  if(a > 20)
  {
    Serial.println(F("day"));
  }
  else
  {
    Serial.println(F("night"));
  }

    // wait for samples to be read
//  if (samplesRead) {
//
//    // print samples to the serial monitor or plotter
//    for (int i = 0; i < samplesRead; i++) {
//      Serial.println(sampleBuffer[i]);
//    }
//
//    // clear the read count
//    samplesRead = 0;
//  }

//    uint32_t sample_max = 0;
//    for (int i = 0; i < samplesRead; i++) 
//    {
//      if(sampleBuffer[i] < 0)
//      {
//            sampleBuffer[i] = -sampleBuffer[i];
//      }
//      if(sampleBuffer[i] > sample_max)
//      {
//           sample_max= sampleBuffer[i]; 
//      } 
//    }
//    uint32_t sample = 24*log10(sample_max*5); //转换分贝
//    samplesRead = 0;
//    Serial.print(sample);
//    Serial.println(F("dB"));
  
   delay(100);
}

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();
  
  // read into the sample buffer
  int bytesRead = PDM.read(sampleBuffer, bytesAvailable);
  
   // 16-bit, 2 bytes per sample
  samplesRead = bytesRead / 2;
}
