
/*
  HOW IT WORKS: 

    LED automation modes:

      Turns ON if temperature is HOT (≥35 °C).
      Turns OFF if NORMAL (33–34 °C).
      Uses schedule (set with /setime) if no manual override.
      User can manually control with /ledon or /ledoff.

    Telegram bot commands:

      SEND:  /setime HH:MM to HH:MM → set schedule.
      SEND:  /status → report LED, schedule, sensor values.
      SEND:  /ledon → manual ON.
      SEND:  /ledoff → manual OFF.

*/


/* LIBRARY */
#include <WiFi.h>                 /* WIFI CONNECTION */
#include "time.h"                 /* TIME FUNCTIONS (NTP, RTC) */
#include "esp_sntp.h"             /* SNTP TIME SYNCHRONIZATION */
#include <WiFiClientSecure.h>     /* SECURE HTTPS CLIENT */
#include <UniversalTelegramBot.h> /* TELEGRAM BOT LIBRARY */
#include "DHT.h"                  /* DHT11 SENSOR LIBRARY */

/* WIFI CREDENTIALS */
const char* ssid = "YOUR_WIFI_SSID";         /* YOUR WIFI SSID (NAME) */ 
const char* password = "YOUR_WIFI_PASSWORD"; /* YOUR WIFI PASSWORD */ 

/* TELEGRAM BOT CREDENTIALS */
#define BOT_TOKEN "YOUR_BOT_TOKEN"   /* TOKEN YOU GET FROM  @BotFather */ 
#define CHAT_ID   "YOUR_CHAT_ID"     /* YOUR PERSONAL CHAT ID FROM @UserInfoToBot */ 

/* LED PIN */
#define LED_PIN 13 /* LED CONNECTED TO GPIO 13 */

/* DHT11 CONFIG */
#define DHTPIN 4          /* DHT11 DATA PIN */
#define DHTTYPE DHT11     /* SENSOR TYPE = DHT11 */
DHT dht(DHTPIN, DHTTYPE); /* CREATE DHT OBJECT */

/* OBJECTS */
WiFiClientSecure client;  /* SECURE CLIENT FOR TELEGRAM HTTPS */
UniversalTelegramBot bot(BOT_TOKEN, client); /* BOT INSTANCE */

/* NTP CONFIG */
const char *ntpServer1 = "pool.ntp.org";  /* PRIMARY NTP SERVER */
const char *ntpServer2 = "time.nist.gov"; /* BACKUP NTP SERVER */
const char *time_zone  = "ICT-7";         /* CAMBODIA UTC+7 */ 

/* TIMER VARIABLES */
unsigned long previousMillis = 0;
const unsigned long interval = 20;  /* INTERVAL BETWEEN UPDATES (MS) -> ACTUALLY 20 MS */ 

/* SCHEDULE VARIABLES */
int onHour = -1, onMinute = -1;   /*  LED ON SCHEDULE TIME */
int offHour = -1, offMinute = -1; /* LED OFF SCHEDULE TIME */

/* MANUAL OVERRIDE */
bool manualOverride = false; /* TRUE IF USER OVERRIDES SCHEDULE */
bool manualOn = false;       /* TRUE IF USER FORCES LED ON */

/* LED STATE */
bool ledState = false;       /* TRUE IF LED IS CURRENTLY ON */

/* GLOBAL SENSOR VARIABLES */
float t = 0;            /* TEMPERATURE */ 
float h = 0;            /* HUMIDITY */ 
String tempStatus = ""; /* TEMPERATURE STATUS */ 

/* HELPER: GET CURRENT LOCAL TIME */
bool getLocalTimeNow(struct tm *timeinfo) {
  if (!getLocalTime(timeinfo)) { /* TRY TO GET SYSTEM TIME */
    Serial.println("⚠️ Failed to obtain time");
    return false;
  }
  return true;
}

/* HANDLE TELEGRAM COMMANDS */
void handleTelegramMessages() { 
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) { /* LOOP UNTIL NO NEW MESSAGES */
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;       /* MESSAGE TEXT */
      String chat_id = bot.messages[i].chat_id; /* SENDER CHAT ID */

      /*  COMMAND: SET SCHEDULE -> /setime HH:MM to HH:MM */  
      if (text.startsWith("/setime ")) {
        int sep1 = text.indexOf(":");  /* FIND FIRST COLON */
        int sep2 = text.indexOf("to"); /* FIND "to" */

        /* VALIDATE FORMAT */
        if (sep1 > 8 && sep2 > sep1 && text.length() > sep2 + 6) {
          /* EXTRACT ON TIME */
          String onStr = text.substring(8, sep2 - 1);
          String offStr = text.substring(sep2 + 3);

          int onH = onStr.substring(0, onStr.indexOf(":")).toInt();
          int onM = onStr.substring(onStr.indexOf(":") + 1).toInt();

          int offH = offStr.substring(0, offStr.indexOf(":")).toInt();
          int offM = offStr.substring(offStr.indexOf(":") + 1).toInt();

          /* SAVE SCHEDULE */
          onHour = onH;
          onMinute = onM;
          offHour = offH;
          offMinute = offM;

          manualOverride = false; /* RESET OVERRIDE */ 
          String msg = "✅ Schedule set:\n LED ON at " + String(onHour) + ":" + String(onMinute) +
                       "\n LED OFF at " + String(offHour) + ":" + String(offMinute);
          bot.sendMessage(chat_id, msg, "");
        } else {
          bot.sendMessage(chat_id, "❌ Invalid format. Use /setime HH:MM to HH:MM", "");
        }
      }

      /* COMMAND: (/status) */ 
      else if (text == "/status") {
        String msg = "🔧 LED Schedule:\n";
        msg += "🟢 LED ON : " + (onHour >= 0 ? (String(onHour) + ":" + String(onMinute)) : "Not Set") + "\n";
        msg += "🔴 LED OFF: " + (offHour >= 0 ? (String(offHour) + ":" + String(offMinute)) : "Not Set") + "\n";
        msg += String("🛑 Manual Override: ") + (manualOverride ? (manualOn ? "LED ON" : "LED OFF") : "Inactive") + "\n";
        msg += String("💡 Current LED: ") + (ledState ? "ON" : "OFF") + "\n";
        msg += "🌡 Temp: " + String(t) + " °C | (" + tempStatus + ")\n";
        msg += "💧 Humidity: " + String(h) + " %";
        bot.sendMessage(chat_id, msg, "");
      }

      /*    COMMAND: (/ledon) TURN LED ON MANUALLY */ 
      else if (text == "/ledon") {
        manualOverride = true;
        manualOn = true;
        ledState = true;
        digitalWrite(LED_PIN, HIGH);
        bot.sendMessage(chat_id, "💡 LED turned ON ", "");
      }

      /* COMMAND: (/ledoff) TURN LED OFF MANUALLY */ 
      else if (text == "/ledoff") {
        manualOverride = true;
        manualOn = false;
        ledState = false;
        digitalWrite(LED_PIN, LOW);
        bot.sendMessage(chat_id, "💡 LED turned OFF ", "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1); /* CHECK AGAIN */
  }
}

/* SETUP */
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin(); /* START DHT11 */

  /* WIFI CONNECT */ 
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n📶 WiFi connected");

  client.setInsecure();  /* IGNORE SSL CERTIFICATE (TELEGRAM NEEDS HTTPS) */ 

  /* SETUP NTP */  
  sntp_set_time_sync_notification_cb([](struct timeval *tv) {
    Serial.println("⏳ NTP TIME SYNCHRONIZED!"); 
  });
  configTzTime(time_zone, ntpServer1, ntpServer2); 

  /* NOTIFY TELEGRAM BOT IS READY */
  bot.sendMessage(CHAT_ID, "🤖 ESP32 READY! SEND /setime, /status, /ledon, /ledoff", "");
}

/* LOOP */
void loop() {
  handleTelegramMessages(); /* CHECK FOR TELEGRAM COMMANDS */

  unsigned long currentMillis = millis(); /* TIMER CHECK */
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    struct tm timeinfo;
    if (getLocalTimeNow(&timeinfo)) { /* GET CURRENT TIME */

      /* READ SENSOR DHT11 */ 
      h = dht.readHumidity();
      t = dht.readTemperature();

      /* DETERMINE tempStatus */  
      if (t >= 29 && t <= 33) tempStatus = "COLD";
      else if (t >= 33 && t <= 34) tempStatus = "NORMAL";
      else if (t >= 35 && t <= 40) tempStatus = "HOT";

      /* TEMPERATURE-BASED LED CONTROL  */ 
      if (t >= 35 && t <= 40) {
        ledState = true;
        digitalWrite(LED_PIN, HIGH);
      } 
      else if (t >= 33 && t <= 34) {
        ledState = false;
        digitalWrite(LED_PIN, LOW);
      } 
      else if (!manualOverride && onHour >= 0 && offHour >= 0) {
        /* APPLY SCHEDULE ONLY IF NO MANUAL OVERRIDE AND TEMP NOT OVERRIDING */ 
        int hourNow = timeinfo.tm_hour;
        int minuteNow = timeinfo.tm_min;
        if (hourNow == onHour && minuteNow == onMinute) ledState = true;
        else if (hourNow == offHour && minuteNow == offMinute) ledState = false;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      }

      /* APPLY MANUAL OVERRIDE */ 
      if (manualOverride) {
        digitalWrite(LED_PIN, manualOn ? HIGH : LOW);
        ledState = manualOn;
      }

      /* FORMAT DATE/TIME */ 
      char dayBuffer[32];
      char dateBuffer[32];
      char timeBuffer[32];
      strftime(dayBuffer, sizeof(dayBuffer), "%A", &timeinfo);
      strftime(dateBuffer, sizeof(dateBuffer), "%d/%m/%Y", &timeinfo);
      strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeinfo);

    /*
      NOTE:
        %A → day of week (e.g., Wednesday)
        %B → month (e.g., August)
        %d → day of month (e.g., 28)
        %Y → year (e.g., 2025)
        %H:%M:%S → hour:minute:second (24h format).
    */

      /* BUILD MESSAGE */ 
      String msg = String(dayBuffer) + "\n" +
                   String(dateBuffer) + "\n" +
                   String(timeBuffer) + "\n" +
                   "LED: " + (ledState ? "ON" : "OFF") + "\n" +
                   "Temp: " + String(t) + " °C | (" + tempStatus + ")\n" +
                   "Humidity: " + String(h) + " %";

      /* SERIAL MONITOR */
      Serial.println(msg);
      Serial.println("");

      /* SSND DATA TO TELEGRAM */
      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}
