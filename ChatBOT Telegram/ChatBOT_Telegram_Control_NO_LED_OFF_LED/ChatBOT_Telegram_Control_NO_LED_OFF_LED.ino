
/* 
  HOW IT WORKS: 
    SEND /ledon  → LED on pin 13 turns ON.
    SEND /ledoff → LED on pin 13 turns OFF.
    SEND /status → Bot replies with current LED state.
*/


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

const char* ssid = "YOUR_WIFI_SSID";         /* YOUR WIFI SSID (NAME) */ 
const char* password = "YOUR_WIFI_PASSWORD"; /* YOUR WIFI PASSWORD */ 

/* TELEGRAM BOT CREDENTIALS */
#define BOT_TOKEN "YOUR_BOT_TOKEN"   /* TOKEN YOU GET FROM  @BotFather */ 
#define CHAT_ID   "YOUR_CHAT_ID"     /* YOUR PERSONAL CHAT ID FROM @UserInfoToBot */  


/* CREATE A SECURE WIFI CLIENT */ 
WiFiClientSecure secured_client;

/* CREATE A TELEGRAM BOT INSTANCE */ 
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

/* HOW OFTEN TO CHECK TELEGRAM FOR NEW MESSAGES MILLISECONDS (MS) */ 
int botRequestDelay = 100;    /* CHECK EVERY 100MS */
unsigned long lastTimeBotRan; /* STORE LAST TIME WE CHECKED */

/* LED PIN */ 
const int ledPin = 13;   /* LED IS CONNECTED TO GPIO 13 */
bool ledState = LOW;     /* CURRENT LED STATE (LOW = OFF, HIGH = ON) */

/* 
   FUNCTION: handleNewMessages()
   Called whenever new messages arrive from Telegram.
   numNewMessages = number of new messages waiting.
*/

void handleNewMessages(int numNewMessages) {
  Serial.print("Got ");
  Serial.print(numNewMessages);
  Serial.println(" new messages");

  /* LOOP THROUGH ALL NEW MESSAGES */
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);

    /* ONLY RESPOND TO YOUR CHAT ID (FOR SAFETY, PREVENTS STRANGERS CONTROLLING YOUR ESP32) */
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue; /* SKIP THIS USER */
    }

    /* GET TEXT OF THE INCOMING MESSAGE */ 
    String text = bot.messages[i].text;
    Serial.println("Message: " + text);

    /* COMMAND: TURN LED ON */
    if (text == "/ledon") {
      digitalWrite(ledPin, HIGH); /* SET GPIO HIGH (LED ON) */
      ledState = HIGH;            /* UPDATE STATE VARIABLE */
      bot.sendMessage(chat_id, "LED is ON ✅", "");
    }

    /* COMMAND: TURN LED OFF */
    if (text == "/ledoff") {
      digitalWrite(ledPin, LOW); /* SET GPIO LOW (LED OFF) */
      ledState = LOW;            /* UPDATE STATE VARIABLE */
      bot.sendMessage(chat_id, "LED is OFF ❌", "");
    }

    /* COMMAND: CHECK LED STATUS */
    if (text == "/status") {
      if (ledState)
        bot.sendMessage(chat_id, "LED is currently ON ✅", "");
      else
        bot.sendMessage(chat_id, "LED is currently OFF ❌", "");
    }
  }
}

void setup() {
  Serial.begin(115200); /* START SERIAL MONITOR FOR DEBUGGING */

  pinMode(ledPin, OUTPUT);   /* SET PIN 13 AS OUTPUT */
  digitalWrite(ledPin, LOW); /* START WITH LED OFF */ 

  /* CONNECT TO WIFI */ 
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  /* REQUIRED FOR TELEGRAM SSL */ 
  secured_client.setInsecure(); /* DISABLE SSL CERTIFICATE CHECK (SIMPLIFIES CONNECTION) */

  /* SEND WELCOME MESSAGE TO TELEGRAM */
  bot.sendMessage(CHAT_ID, "ESP32 Telegram Bot Started 🤖\nUse /ledon or /ledoff", "");
}

void loop() {
  /* CHECK FOR NEW TELEGRAM MESSAGES EVERY 'botRequestDelay' ms */ 
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    /* IF THERE ARE NEW MESSAGES, PROCESS THEM */
    while (numNewMessages) {
      handleNewMessages(numNewMessages); /* PROCESS COMMANDS */
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    /* SAVE THE LAST TIME WE CHECKED */
    lastTimeBotRan = millis(); 
  }
}
