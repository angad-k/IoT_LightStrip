/*
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProLight.h>
#include <FastLED.h>

#include "LED_Controller_.h"

#include <map>

#define WIFI_SSID  "Cringe"
#define WIFI_PASS  "12345678"
#define APP_KEY    "4740cb6a-b154-489c-98fb-136fccb401fb"     // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET "dd250388-4db4-4b66-a83a-f62fea32f387-89f3b93d-d50e-45e4-96d6-58ee6332ecbb"  // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define LIGHT_ID   "642b5b370f750d41bd1f232c"   // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define CONTROLLER_ID "642b78e80f750d41bd1f2e32"
#define BAUD_RATE  9600              // Change baudrate to your need for serial log

#define NUM_LEDS 7
#define LED_PIN  4  

CRGB leds[NUM_LEDS];

int default_time_period = 1000;
int time_period = 1000;         //Time period is 1second
String light_mode = "basic";

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

Color secondary_color;

// Colortemperature lookup table
using ColorTemperatures = std::map<uint16_t, Color>;
ColorTemperatures colorTemperatures{
    //   {Temperature value, {color r, g, b}}
    {2000, {255, 138, 18}},
    {2200, {255, 147, 44}},
    {2700, {255, 169, 87}},
    {3000, {255, 180, 107}},
    {4000, {255, 209, 163}},
    {5000, {255, 228, 206}},
    {5500, {255, 236, 224}},
    {6000, {255, 243, 239}},
    {6500, {255, 249, 253}},
    {7000, {245, 243, 255}},
    {7500, {235, 238, 255}},
    {9000, {214, 225, 255}}};

struct DeviceState {                                   // Stores current device state with following initial values:
    bool  powerState       = false;                    // initial state is off
    Color color            = colorTemperatures[9000];  // color is set to white (9000k)
    int   colorTemperature = 9000;                     // color temperature is set to 9000k
    int   brightness       = 100;                      // brightness is set to 100
} device_state;

SinricProLight& myLight = SinricPro[LIGHT_ID];  // SinricProLight device
LED_Controller_ &lED_Controller_ = SinricPro[CONTROLLER_ID];

int ambient_color_interpolation(int val1, int val2){
  float diff = millis()%(2*time_period);
  float lin_val;
  if(diff < time_period)
    lin_val = val1+((val2-val1)*diff)/time_period;
  else{
    diff -= time_period;
    lin_val = val2+((val1-val2)*diff)/time_period;
  }
    
  return int(lin_val);
}

void setStripe() {
    Color primary_color;
    primary_color.r = map(device_state.color.r * device_state.brightness, 0, 255 * 100, 0, 255);  // calculate red value and map between 0 and 1023 for analogWrite
    primary_color.g = map(device_state.color.g * device_state.brightness, 0, 255 * 100, 0, 255);  // calculate green value and map between 0 and 1023 for analogWrite
    primary_color.b = map(device_state.color.b * device_state.brightness, 0, 255 * 100, 0, 255);  // calculate blue value and map between 0 and 1023 for analogWrite

  if(light_mode == "basic"){
    for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(primary_color.r,primary_color.g,primary_color.b);
  }
  if(light_mode == "static"){
    for(int i = 0; i < NUM_LEDS; i+=2) leds[i] = CRGB(secondary_color.r,secondary_color.g,secondary_color.b);
    for(int i = 1; i < NUM_LEDS; i+=2) leds[i] = CRGB(primary_color.r,primary_color.g,primary_color.b);
  }
  if(light_mode == "dynamic"){
    if( (millis() % time_period) > time_period/2)
      for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(secondary_color.r,secondary_color.g,secondary_color.b);
    else          
      for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(primary_color.r,primary_color.g,primary_color.b);
  }
  if(light_mode == "dynamic2"){
    if ((millis() % time_period) > time_period/2 ){
      for(int i = 0; i < NUM_LEDS; i+=2) leds[i] = CRGB(secondary_color.r,secondary_color.g,secondary_color.b);
      for(int i = 1; i < NUM_LEDS; i+=2) leds[i] = CRGB(primary_color.r,primary_color.g,primary_color.b);
    }
    else{
      for(int i = 1; i < NUM_LEDS; i+=2) leds[i] = CRGB(secondary_color.r,secondary_color.g,secondary_color.b);
      for(int i = 0; i < NUM_LEDS; i+=2) leds[i] = CRGB(primary_color.r,primary_color.g,primary_color.b);
    }
  }
  if(light_mode == "ambient"){
    int r = ambient_color_interpolation(primary_color.r,secondary_color.r);
    int g = ambient_color_interpolation(primary_color.g,secondary_color.g);
    int b = ambient_color_interpolation(primary_color.b,secondary_color.b);

    for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(r,g,b);
  }
  if (device_state.powerState == false) {  // turn off
    for(int i = 0; i < NUM_LEDS; i++){
      leds[i] = CRGB(0,0,0);
    }    
  }
  FastLED.show(); 
}

bool onPowerState(const String& deviceId, bool& state) {
    device_state.powerState = state;  // store the new power state
    setStripe();                      // update the mosfets
    return true;
}

bool onBrightness(const String& deviceId, int& brightness) {
    device_state.brightness = brightness;  // store new brightness level
    setStripe();                           // update the mosfets
    return true;
}

bool onAdjustBrightness(const String& deviceId, int& brightnessDelta) {
    device_state.brightness += brightnessDelta;  // calculate and store new absolute brightness
    brightnessDelta = device_state.brightness;   // return absolute brightness
    setStripe();                                 // update the mosfets
    return true;
}

bool onColor(const String& deviceId, byte& r, byte& g, byte& b) {
    device_state.color.r = r;  // store new red value
    device_state.color.g = g;  // store new green value
    device_state.color.b = b;  // store new blue value
    setStripe();               // update the mosfets
    return true;
}

bool onColorTemperature(const String& deviceId, int& colorTemperature) {
    device_state.color            = colorTemperatures[colorTemperature];  // set rgb values from corresponding colortemperauture
    device_state.colorTemperature = colorTemperature;                     // store the current color temperature
    setStripe();                                                          // update the mosfets
    return true;
}

bool onIncreaseColorTemperature(const String& devceId, int& colorTemperature) {
    auto current = colorTemperatures.find(device_state.colorTemperature);  // get current entry from colorTemperature map
    auto next    = std::next(current);                                     // get next element
    if (next == colorTemperatures.end()) next = current;                   // prevent past last element
    device_state.color            = next->second;                          // set color
    device_state.colorTemperature = next->first;                           // set colorTemperature
    colorTemperature              = device_state.colorTemperature;         // return new colorTemperature
    setStripe();
    return true;
}

bool onDecreaseColorTemperature(const String& devceId, int& colorTemperature) {
    auto current = colorTemperatures.find(device_state.colorTemperature);  // get current entry from colorTemperature map
    auto next    = std::prev(current);                                     // get previous element
    if (next == colorTemperatures.end()) next = current;                   // prevent before first element
    device_state.color            = next->second;                          // set color
    device_state.colorTemperature = next->first;                           // set colorTemperature
    colorTemperature              = device_state.colorTemperature;         // return new colorTemperature
    setStripe();
    return true;
}

// ModeController
bool onSetMode(const String& deviceId, const String& instance, String &mode) {
  Serial.printf("[Device: %s]: Modesetting for \"%s\" set to mode %s\r\n", deviceId.c_str(), instance.c_str(), mode.c_str());
  light_mode = mode;
  return true;
}

// ColorController
bool onColor2(const String &deviceId, byte &r, byte &g, byte &b) {
  Serial.printf("[Device: %s]: Color set to red=%d, green=%d, blue=%d\r\n", deviceId.c_str(), r, g, b);
  secondary_color.r = r;
  secondary_color.g = g;
  secondary_color.b = b;
  return true; // request handled properly
}

// ModeController
void updateMode(String instance, String mode) {
  lED_Controller_.sendModeEvent(instance, mode, "PHYSICAL_INTERACTION");
}

// ColorController
void updateColor2(byte r, byte g, byte b) {
  lED_Controller_.sendColorEvent(r, g, b);
}

void setupWiFi() {
    Serial.printf("WiFi: connecting");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.printf(".");
        delay(250);
    }
    Serial.printf("connected\r\nIP is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro() {
    myLight.onPowerState(onPowerState);                              // assign onPowerState callback
    myLight.onBrightness(onBrightness);                              // assign onBrightness callback
    myLight.onAdjustBrightness(onAdjustBrightness);                  // assign onAdjustBrightness callback
    myLight.onColor(onColor);                                        // assign onColor callback
    myLight.onColorTemperature(onColorTemperature);                  // assign onColorTemperature callback
    myLight.onDecreaseColorTemperature(onDecreaseColorTemperature);  // assign onDecreaseColorTemperature callback
    myLight.onIncreaseColorTemperature(onIncreaseColorTemperature);  // assign onIncreaseColorTemperature callback


    // ModeController
    lED_Controller_.onSetMode("modeInstance1", onSetMode);


    // ColorController
    lED_Controller_.onColor(onColor2);

    SinricPro.begin(APP_KEY, APP_SECRET);  // start SinricPro
    Serial.println("Connected to SinricPro");
}

void setupFastLED(){
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();
  Serial.println("LED Setup done");
}

void setup() {
    Serial.begin(BAUD_RATE);  // setup serial

    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxRefreshRate(120);
    setupWiFi();        // connect wifi
    setupSinricPro();  // setup SinricPro
    FastLED.clear();
    Serial.println("PAIN");
    FastLED.show();     // setup LED control
}

void loop() {
  setStripe();   
  SinricPro.handle();  // handle SinricPro communication
}