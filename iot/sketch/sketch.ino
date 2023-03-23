#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266mDNS.h>        // Include the mDNS library
 
ESP8266WiFiMulti wifiMulti;

WiFiServer wifiServer(80);

int PrimaryRGB[] = {255, 255, 255};
int SecondaryRGB[] = {255, 255, 255};
String light_mode = "static";
bool powersaving = false;

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

  ret += "}";

  return ret;
}

void setup() {
  Serial.begin(9600);

  // WIFI connection code
  wifiMulti.addAP("R2D2", "ChaChaCha"); 
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

void loop() {
  MDNS.update();

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
        String line = client.readStringUntil('\r');
        Serial.print(line);
        // wait for end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n')
        {
//          client.println("{'data' : 'Hello from ESP8266'}");
          break;
        }
      }
    }

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
