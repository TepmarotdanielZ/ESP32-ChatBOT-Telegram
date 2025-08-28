/* V3 */

/* REQUIRED LIBRARIES */ 
#include <WiFi.h>                 /* CONNECT ESP32 TO WIFI */ 
#include "time.h"                 /* TIME FUNCTIONS (NTP, RTC) */ 
#include "esp_sntp.h"             /* SNTP CLIENT FOR SYNCING TIME */ 
#include <WiFiClientSecure.h>     /* SECURE HTTPS CONNECTION (NEEDED FOR TELEGRAM) */ 
#include <UniversalTelegramBot.h> /* TELEGRAM BOT LIBRARY */ 

/* WIFI CREDENTIALS */
const char* ssid = "Sonin KH";        /* WIFI SSID (NETWORK NAME) */ 
const char* password = "77771112";    /* WIFI PASSWORD */ 

/* TELEGRAM BOT CREDENTIALS */
#define BOT_TOKEN "8084553369:AAHgqoTlB1nURTs69WxGDE2KLjEXDW4soAw"  /* YOUR BOT TOKEN FROM @BotFather */  
#define CHAT_ID   "843845173"                                       /* YOUR PERSONAL TELEGRAM CHAT ID FROM @UserInfoToBot */ 

WiFiClientSecure client;                       /* SECURE WIFI CLIENT FOR HTTPS */ 
UniversalTelegramBot bot(BOT_TOKEN, client);   /* CREATE BOT INSTANCE WITH TOKEN AND CLIENT */ 

/* NTP SERVERS */
const char *ntpServer1 = "pool.ntp.org";  /* FIRST NTP SERVER */ 
const char *ntpServer2 = "time.nist.gov"; /* BACKUP NTP SERVER */ 

/* CAMBODIA UTC+7 */
const char *time_zone = "ICT-7";  /* INDOCHINA TIME (UTC+7, NO DST) */ 

/* NON-BLOCKING TIMER VARIABLES */
unsigned long previousMillis = 0;          /* STORE LAST TIME A MESSAGE WAS SENT */ 
const unsigned long interval = 20;         /* INTERVAL (20 ms) */   

/* FUNCTION: GET LOCAL TIME AS STRING */
String getLocalTimeString() {
  struct tm timeinfo;  /* STRUCTURE TO HOLD TIME INFO (hours, minutes, etc.) */ 
  
  if (!getLocalTime(&timeinfo)) {   /* IF TIME IS NOT YET AVAILABLE */ 
    return "No time available";     /* RETURN ERROR MESSAGE */ 
  }

  char buffer[64];  /* CHARACTER BUFFER TO HOLD FORMATTED TIME */ 
  /* FORMAT TIME: EXAMPLE -> "WEDNESDAY, 27/08/2025 20:53:43 */ 
  strftime(buffer, sizeof(buffer), "%A, %d/%m/%Y %H:%M:%S", &timeinfo);

  /*
    NOTE:
      %A → day of week (e.g., Wednesday)
      %B → month (e.g., August)
      %d → day of month (e.g., 28)
      %Y → year (e.g., 2025)
      %H:%M:%S → hour:minute:second (24h format).
  */
  
  return String(buffer);  /* RETURN FORMATTED TIME AS STRING */ 
}

/* CALLBACK: WHEN NTP UPDATES TIME */
void timeavailable(struct timeval *t) {
  Serial.println("⏳ Got time adjustment from NTP!");
}

/*  SETUP  */
void setup() {
  Serial.begin(115200);   /* START SERIAL COMMUNICATION (DEBUGGING) */ 

  /* CONNECT TO WIFI */ 
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);

  /* WAIT UNTIL WIFI IS CONNECTED */ 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n📶 WiFi connected");

  client.setInsecure();  /* DISABLE SSL CERTIFICATE CHECKING (TELEGRAM REQUIRES HTTPS) */ 

  /* SET UP NTP */
  sntp_set_time_sync_notification_cb(timeavailable);   /* CALL 'timeavailable' WHEN TIME SYNC HAPPENS  */ 
  configTzTime(time_zone, ntpServer1, ntpServer2);     /* SET 'timezone' AND 'NTP' SERVERS  */ 

  /* SEND FIRST TEST MESSAGE TO TELEGRAM */ 
  Serial.println("📩 Sending test message to Telegram...");
  bot.sendMessage(CHAT_ID, "✅ ESP32 connected! Time sync via NTP started.", "Markdown");
}

/* LOOP */
void loop() {
  unsigned long currentMillis = millis();  /* GET CURRENT TIME (MS SINCE BOOT) */ 

  /* CHECK IF INTERVAL (20 SEC) HAS PASSED */ 
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  /* SAVE LAST TIME MESSAGE WAS SENT */ 

    /* GET CURRENT TIME STRING */ 
    String currentTime = getLocalTimeString();

    /* PREPARE TELEGRAM MESSAGE */ 
    String message = "🗓️ " + currentTime;

    /* PRINT TO SERIAL MONITOR */ 
    Serial.println(message);

    /* SEND TO TELEGRAM */ 
    bot.sendMessage(CHAT_ID, message, "Markdown");
  }
}
