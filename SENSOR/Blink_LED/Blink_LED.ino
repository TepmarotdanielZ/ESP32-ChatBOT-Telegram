/* ESP32 BLINK ON PIN 13 */ 

const int LED_PIN = 13; /* ADJUST LED_PIN IF YOUR BOARD USES A DIFFERENT GPIO */

void setup() {
  pinMode(LED_PIN, OUTPUT); /* INITIALIZE THE LED PIN AS AN OUTPUT */
}

void loop() {
  
  digitalWrite(LED_PIN, HIGH); /* TURN THE LED ON */
  delay(300); /* WAIT 300ms */ 

  digitalWrite(LED_PIN, LOW); /* TURN THE LED OFF */
  delay(300); /* WAIT 300ms */
}