#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266mDNS.h>        // Include the mDNS library
 #include <FastLED.h>           // Include the LED control library

#define LED_PIN 4               //Pin number for LED
#define NUM_LEDS 7              // Number of LEDs on strip

CRGB leds[NUM_LEDS];

/*
  leds[0] = CRGB(255, 0, 0 );
  leds[1] = CRGB(0, 255, 0 );
  leds[2] = CRGB(0 , 0 , 255);
  FastLED.show();
*/



ESP8266WiFiMulti wifiMulti;

WiFiServer wifiServer(80);

int PrimaryRGB[] = {255 , 255, 255};
int SecondaryRGB[] = {255, 255, 255};
int default_time_period = 1000;
int time_period = 1000;         //Time period is 1second
String light_mode = "basic";
bool powersaving = false;

char MAC_array[17][50];
int MAC_arraylength = 0;

String make_data()
{
  String ret = "{";
  ret += "\"PrimaryRGB\" : ";
  ret += "[";
  ret += PrimaryRGB[0];        
  ret += ", ";
  ret += PrimaryRGB[1];
  ret += ", ";
  ret += PrimaryRGB[2];
  ret += "]";
  ret += ", ";
  
  ret += "\"SecondaryRGB\" : ";
  ret += "[";
  ret += SecondaryRGB[0];       
  ret += ", ";
  ret += SecondaryRGB[1];
  ret += ", ";
  ret += SecondaryRGB[2];
  ret += "]";
  ret += ", ";

  ret += "\"light_mode\" : ";
  ret += "\"";
  ret += light_mode;
  ret += "\"";
  ret += ", ";

  ret += "\"powersaving\" : ";
  if(powersaving)
  {
    ret += "true";
  }
  else
  {
    ret += "false";
  }
  ret += ", ";

  ret += "\"MAC_array\" : ";
  ret += "[";
  for(int i = 0; i < MAC_arraylength; i++)
  {
    ret += "\"";
    ret += MAC_array[i];
    ret += "\"";
    if( i != MAC_arraylength - 1 )
    ret += ",";
  }
  ret += "]";

  ret += "}";

  return ret;
}

uint8_t* getMacBytesFromString(char* mac)
{
  uint8_t bytes[6];
  int values[6];
  int i;
  
  if( 6 == sscanf( mac, "%x:%x:%x:%x:%x:%x%*c",
      &values[0], &values[1], &values[2],
      &values[3], &values[4], &values[5] ) )
  {
      /* convert to uint8_t */
      for( i = 0; i < 6; ++i )
          bytes[i] = (uint8_t) values[i];
  }
  
  return bytes;
}

void setup() {
  Serial.begin(9600);

  //LED strip setup code
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();

  // WIFI connection code
  wifiMulti.addAP("Cringe", "12345678"); 
  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());  
  WiFi.mode(WIFI_STA);
  delay(100);

  wifiServer.begin();

  // MDNS (NSD) code
  if (!MDNS.begin("LightStrip")) {
    
    Serial.println("Error setting up MDNS responder!");
  }
  MDNS.addService("http", "tcp", 80);
  Serial.println("mDNS responder started");
}

void handle_cmd(String cmd)
{
  if(cmd.startsWith("cmd_pri"))
  {
    String working_cmd = cmd.substring(8);
    Serial.println("primarycolor : ");
    for(int i = 0; i < 3; i++)
    {
      int next_ind = working_cmd.indexOf(" ");
      int col = working_cmd.substring(0, next_ind).toInt();
      PrimaryRGB[i] = col;
      working_cmd = working_cmd.substring(next_ind+1);
      Serial.println(PrimaryRGB[i]);
    }  
  }

  if(cmd.startsWith("cmd_sec"))
  {
    String working_cmd = cmd.substring(8);
    Serial.println("secondary : ");
    for(int i = 0; i < 3; i++)
    {
      int next_ind = working_cmd.indexOf(" ");
      int col = working_cmd.substring(0, next_ind).toInt();
      SecondaryRGB[i] = col;
      working_cmd = working_cmd.substring(next_ind+1);
      Serial.println(SecondaryRGB[i]);
    }
  }

  if(cmd.startsWith("cmd_add"))
  {
    String working_cmd = cmd.substring(8);
    strncpy(MAC_array[MAC_arraylength], working_cmd.c_str(), sizeof(MAC_array[MAC_arraylength]));
    Serial.println("added MAC Address: ");
    Serial.println(MAC_array[MAC_arraylength]);
    MAC_arraylength++;
  }

  if(cmd.startsWith("cmd_del"))
  {
    String working_cmd = cmd.substring(8);
    working_cmd.trim();
    int index = 0;
    for(int i = 0; i < MAC_arraylength; i++)
    {
      if (strcmp(MAC_array[i], working_cmd.c_str())==0)
      {
        index = i;
      }
    }
    Serial.println(working_cmd);
    Serial.println("removed MAC Address: ");
    Serial.println(MAC_array[index]);
    
    for(int i = index; i < MAC_arraylength; i++)
    {
      strncpy(MAC_array[i], MAC_array[i+1], sizeof(MAC_array[MAC_arraylength]));
    }
    MAC_arraylength--;
  }

  if(cmd.startsWith("cmd_mod"))
  {
    String working_cmd = cmd.substring(8);
    working_cmd.trim();
    light_mode = working_cmd;
    Serial.println("lightmode : ");
    Serial.println(light_mode);
  }

  if(cmd.startsWith("cmd_pow"))
  {
    if(cmd[8]==0)
    {
      powersaving = false;
    }
    else
    {
      powersaving = true;
    }

    Serial.println("powersaving : ");
    Serial.println(powersaving);
  }
}

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

void setLEDColor() {
  if(light_mode == "basic"){
    for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(PrimaryRGB[0],PrimaryRGB[1],PrimaryRGB[2]);
  }
  if(light_mode == "static"){
    for(int i = 0; i < NUM_LEDS; i+=2) leds[i] = CRGB(SecondaryRGB[0],SecondaryRGB[1],SecondaryRGB[2]);
    for(int i = 1; i < NUM_LEDS; i+=2) leds[i] = CRGB(PrimaryRGB[0],PrimaryRGB[1],PrimaryRGB[2]);
  }
  if(light_mode == "dynamic"){
    if( (millis() % time_period) > time_period/2)
      for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(SecondaryRGB[0],SecondaryRGB[1],SecondaryRGB[2]);
    else          
      for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(PrimaryRGB[0],PrimaryRGB[1],PrimaryRGB[2]);  
  }
  if(light_mode == "dynamic2"){
    if ((millis() % time_period) > time_period/2 ){
      for(int i = 0; i < NUM_LEDS; i+=2) leds[i] = CRGB(SecondaryRGB[0],SecondaryRGB[1],SecondaryRGB[2]);
      for(int i = 1; i < NUM_LEDS; i+=2) leds[i] = CRGB(PrimaryRGB[0],PrimaryRGB[1],PrimaryRGB[2]);
    }
    else{
      for(int i = 1; i < NUM_LEDS; i+=2) leds[i] = CRGB(SecondaryRGB[0],SecondaryRGB[1],SecondaryRGB[2]);
      for(int i = 0; i < NUM_LEDS; i+=2) leds[i] = CRGB(PrimaryRGB[0],PrimaryRGB[1],PrimaryRGB[2]);
    }
  }
  if(light_mode == "ambient"){
    int r = ambient_color_interpolation(PrimaryRGB[0],SecondaryRGB[0]);
    int g = ambient_color_interpolation(PrimaryRGB[1],SecondaryRGB[1]);
    int b = ambient_color_interpolation(PrimaryRGB[2],SecondaryRGB[2]);

    for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(r,g,b);
  }
  FastLED.show();
}

void loop() {
  MDNS.update();
  setLEDColor();
  //socket 
  WiFiClient client = wifiServer.accept();
  if (client)
  {
    Serial.println("\n[Client connected]");
    client.println(make_data());
    while (client.connected())
    {
      
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
        handle_cmd(line);
        setLEDColor();
      }
      setLEDColor();
    }
    setLEDColor();
    while (client.available()) {
      // but first, let client finish its request
      // that's diplomatic compliance to protocols
      // (and otherwise some clients may complain, like curl)
      // (that is an example, prefer using a proper webserver library)
      client.read();
    }

    // close the connection:
    client.stop();
    Serial.println("[Client disconnected]");
  }
  
//  DNS.processNextRequest();
}
