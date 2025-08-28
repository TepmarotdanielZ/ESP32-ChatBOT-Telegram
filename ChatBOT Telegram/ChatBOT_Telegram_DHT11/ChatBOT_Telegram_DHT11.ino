/* 
   ESP32 TELEGRAM BOT + DHT11 SENSOR

   - READS TEMPERATURE & HUMIDITY FROM DHT11
   - SENDS DATA TO TELEGRAM CHAT

*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHT.h"

/* WIFI CREDENTIALS */
const char* ssid = "YOUR_WIFI_SSID";         /* YOUR WIFI SSID (NAME) */ 
const char* password = "YOUR_WIFI_PASSWORD"; /* YOUR WIFI PASSWORD */ 

/* TELEGRAM BOT CREDENTIALS  */
#define BOT_TOKEN "YOUR_BOT_TOKEN"   /* TOKEN FROM @BotFather */  
#define CHAT_ID   "YOUR_CHAT_ID"     /* YOUR CHAT ID FROM @userinfobot */  


/* DHT11 CONFIG  */
#define DHTPIN 4         /* PIN DHT11 */ 
#define DHTTYPE DHT11    /* SENSOR TYPE = DHT11 */ 
DHT dht(DHTPIN, DHTTYPE);  

/* TELEGRAM CLIENT */
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void setup() {
  /* START SERIAL COMMUNICATION */ 
  Serial.begin(115200);
  Serial.println();

  /* START DHT11 SENSOR */ 
  dht.begin();

  /* CONNECT TO WIFI */ 
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED ✅");
  Serial.println("Data sent to Telegram ✅");

  /* REQUIRED FOR TELEGRAM SSL */ 
  secured_client.setInsecure();

  /* SEND START MESSAGE TO TELEGRAM */ 
  bot.sendMessage(CHAT_ID, "ESP32 DHT11 Bot Started 🤖", "");
}

void loop() {
  
  /* READ DATA FROM DHT11 */ 
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  /* CHECK IF READ FAILED */ 
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  /* PRINT DATA TO SERIAL MONITOR */ 
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("% | Temperature: "));
  Serial.print(t);
  Serial.println(F("°C"));

  /* FORMAT MESSAGE FOR TELEGRAM */ 
  String message = "🌡 Temperature: " + String(t) + " °C\n";
  message += "💧 Humidity: " + String(h) + " %";

  /* SEND TO TELEGRAM */ 
  bot.sendMessage(CHAT_ID, message, "");
  

  delay(200);  /* WAIT 2 SECONDS BEFORE NEXT MESSAGE */ 
}
