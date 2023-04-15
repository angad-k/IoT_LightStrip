# Alexa / Google Home controlled LED Strip

The goal is to control a WS2812B LED Strip using Google Home and Alexa. We have also developed an app for additional functionality and convenience to control the LED Strip.

# Requirements

## Hardware Requirements

- NodeMCU ESP8266
- WS2812B LED Strip

## Software Requirements

- FastLED (Installable through library manager in the Arduino IDE)
- Sinric Pro (Installable through library manager in the Arduino IDE)
- ESP8266WiFi.h
- ESP8266WiFiMulti.h
- ESP8266mDNS.h

# Schematic

![WhatsApp Image 2023-04-15 at 15 33 13](https://user-images.githubusercontent.com/54415525/232209219-43b85574-f6ab-40e1-9930-ee9bd90014a1.jpeg)

![WhatsApp Image 2023-04-15 at 15 34 46](https://user-images.githubusercontent.com/54415525/232209237-126dc887-5ccc-4e31-89a7-d4d5c5b12ec4.jpeg)

![WhatsApp Image 2023-04-15 at 15 40 29](https://user-images.githubusercontent.com/54415525/232209250-3f9bcf82-cadc-4bd8-9b4a-7b9fb00abd42.jpeg)

The above images are for representation purposes. In the actual setup, we use NodeMCU ESP8266 instead of Arduino Uno and we connect the D_in wire of the WS2812B to GPIO4 of ESP8266.

# Features

Check [this presentation](https://www.canva.com/design/DAFfnR6n9eM/Sm-5nXsn8KcHFliG1HMEUg/view?utm_content=DAFfnR6n9eM&utm_campaign=designshare&utm_medium=link&utm_source=publishsharelink#11) 
out to have a better understanding of the features along with video demonstrations.

Following is the list of features :

- Control Power, Brightness, Color of the entire LED Strip using Google Home. (Google Home treats the entire LED Strip as one light)
- Control Power, Brightness, 5 lighting modes (Basic, Static, Dynamic, Dynamic2 Ambient - check the presentation for demonstration) and colors for each lighting mode using Alexa.
- Control Power, Brightness, 5 lighting modes (Basic, Static, Dynamic, Dynamic2 Ambient - check the presentation for demonstration) and colors for each lighting mode using the Mobile App. (More Convenient)

# Setup

## Sinric Pro

- Make an account on Sinric Pro.
- Create a basic light device and an LED controller light device using the template provided [here](https://github.com/angad-k/IoT_LightStrip/blob/main/iot/sinric_sketch/LED_Controller_.h).

### Google Home

- Connect your Sinric Pro account in the Google Home app. 
- Device will automatically appear in the app. 
- You can now control the device through the app.

### Alexa

- Connect your Sinric Pro account in the Google Home app.
- Use the LED Device to control brightness, power and primary color.
- Use the LED controller Device to control lighting mode and secondary color.

## Mobile App

- You can download our app from releases.
- You need to be on the same WiFi network as the ESP8266.
- App will automically connect and control all settings of the device.
