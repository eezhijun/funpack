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
#include <arduinoFFT.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define LEDR        (22u)
#define LEDG        (23u)
#define LEDB        (24u)

#define TFT_CS        (8u)// PyBadge/PyGamer display control pins: chip select
#define TFT_RST      (10u)  // Display reset
#define TFT_DC        (9u) // Display data/command select
#define TFT_BACKLIGHT (7u) // Display backlight pin

#define SAMPLES 256
#define SAMPLING_FREQUENCY 16000

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[SAMPLES];
// number of samples read
volatile int samplesRead;
// FFT real and imaginary vectors
double vReal[SAMPLES];
double vImag[SAMPLES];
// final result from FFT
double ftsum = 0.0;

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);
arduinoFFT FFT = arduinoFFT();

void setup() {
  // put your setup code here, to run once:

/* 串口初始化 */
  Serial.begin(9600);
  while (!Serial);

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
    if (!PDM.begin(1, SAMPLING_FREQUENCY)) {
    Serial.println(F("Failed to start PDM!"));
    while (1);
  }

  Serial.println(F("system initialization complete! "));

}

void loop() {
  // put your main code here, to run repeatedly:

  delay(1000);    // wait a second
  
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK); 
  tft.setCursor(0, 0);
  tft.print("WERTHER STATION");
  tft.drawLine(0, 16, tft.width() - 1, 16, ST77XX_YELLOW);
  
  float temperature = HTS.readTemperature();  
  Serial.print(F("TEMPERATURE:"));
  Serial.print(temperature, 1);
  Serial.println(F(" °C"));
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // set text color to red and black background
  tft.setCursor(0, 20);
  tft.print("TEMPERATURE:");
  tft.print(temperature, 1);
  tft.drawCircle(200, 20, 3, ST77XX_RED);     // print degree symbol ( ° )
  tft.setCursor(208, 20);
  tft.print("C");
  tft.drawLine(0, 36, tft.width() - 1, 36, ST77XX_YELLOW);

  float humidity    = HTS.readHumidity();
  Serial.print(F("HUMIDITY:"));
  Serial.print(humidity, 1);
  Serial.println(F(" %"));
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // set text color to red and black background
  tft.setCursor(0, 40);
  tft.print("HUMIDITY:");
  tft.print(humidity, 1);
  tft.setCursor(160, 40);
  tft.print("%");
  tft.drawLine(0, 56, tft.width() - 1, 56, ST77XX_YELLOW);
  
  float pressure = BARO.readPressure();
  Serial.print(F("PRESSURE:"));
  Serial.print(pressure, 1);
  Serial.println(F(" kPa"));
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // set text color to red and black background
  tft.setCursor(0, 60);
  tft.print("PRESSURE:");
  tft.print(pressure, 1);
  tft.setCursor(170, 60);
  tft.print("kPa");
  tft.drawLine(0, 76, tft.width() - 1, 76, ST77XX_YELLOW);
  
  while (! APDS.colorAvailable()) {
    delay(1);
  }
  int r, g, b, a;
  APDS.readColor(r, g, b, a);
  Serial.print("r:");
  Serial.println(r);
  Serial.print("g:");
  Serial.println(g);
  Serial.print("b:");
  Serial.println(b);
  Serial.print("a:");
  Serial.println(a);
  if(a > 20)
  {
    
    Serial.println(F("DAY"));
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // set text color to red and black background
    tft.setCursor(0, 80);
    tft.print("AMBIENT:DAY  ");
  }
  else
  {
    Serial.println(F("NIGHT"));
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // set text color to red and black background
    tft.setCursor(0, 80);
    tft.print("AMBIENT:NIGHT");
  }
  tft.drawLine(0, 96, tft.width() - 1, 96, ST77XX_YELLOW);

  while (!samplesRead);
  uint32_t sample_max = 0;
  for (int i = 0; i < samplesRead; i++) {
    if(sampleBuffer[i] < 0)
      sampleBuffer[i] = -sampleBuffer[i];
    if(sampleBuffer[i] > sample_max) 
      sample_max= sampleBuffer[i];
  }
  uint32_t sample = 24*log10(sample_max*5); 
  samplesRead = 0;

  Serial.println(sample);
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // set text color to red and black background
  tft.setCursor(0, 100);
  tft.print("NOISE:");
  tft.print(sample, 1);
  tft.setCursor(110, 100);
  tft.print("dB");
  tft.drawLine(0, 116, tft.width() - 1, 116, ST77XX_YELLOW);
}

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();
  
  // read into the sample buffer
  int bytesRead = PDM.read(sampleBuffer, bytesAvailable);
  
   // 16-bit, 2 bytes per sample
  samplesRead = bytesRead / 2;
}
