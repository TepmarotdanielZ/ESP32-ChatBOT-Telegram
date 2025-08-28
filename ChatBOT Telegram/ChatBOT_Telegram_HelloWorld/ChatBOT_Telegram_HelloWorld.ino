#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

/* WIFI CREDENTIALS */ 
const char* ssid = "YOUR_WIFI_SSID";         /* YOUR WIFI SSID (NAME) */ 
const char* password = "YOUR_WIFI_PASSWORD"; /* YOUR WIFI PASSWORD */ 

/* TELEGRAM BOT CREDENTIALS */   
#define BOT_TOKEN "YOUR_BOT_TOKEN"   /* TOKEN YOU GET FROM @BotFather */  
#define CHAT_ID   "YOUR_CHAT_ID"     /* YOUR PERSONAL CHAT ID FROM @UserInfoToBot */  

/* CREATE A SECURE WIFI CLIENT */ 
WiFiClientSecure secured_client;

/* CREATE A TELEGRAM BOT INSTANCE WITH YOUR BOT TOKEN */ 
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void setup() {
  /* START SERIAL COMMUNICATION FOR DEBUGGING */ 
  Serial.begin(115200);
  Serial.println();

  /* CONNECT TO WIFI */ 
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);

  /* WAIT UNTIL WIFI IS CONNECTED */ 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  /* REQUIRED FOR TELEGRAM SSL (DISABLE CERTIFICATE VERIFICATION) */ 
  secured_client.setInsecure();
}

void loop() {
  /* PRINT STATUS TO SERIAL MONITOR */ 
  Serial.println("Message sent to Telegram ✅");
  Serial.println("Hello World from ESP32!");
  Serial.println(" ");

  /* SEND "HELLO WORLD" MESSAGE TO YOUR TELEGRAM CHAT */ 
  bot.sendMessage(CHAT_ID, "Hello World from ESP32!", "");

  /* SMALL DELAY BEFORE REPEATING */ 
  delay(200);   /* WAIT 2 SECONDS BEFORE SENDING AGAIN */ 
}
