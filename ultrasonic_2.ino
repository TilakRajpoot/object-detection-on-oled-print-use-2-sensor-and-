// Sender_ESP32_fixed.ino
#include <esp_now.h>
#include <WiFi.h>

#define TRIG_PIN 5
#define ECHO_PIN 18

// Change to 1 or 2 depending on this device
#define SENSOR_ID 2

typedef struct struct_message {
  int id;
  char text[32];
} struct_message;

struct_message myData;

// Put the receiver (OLED ESP) MAC here (6 bytes). Replace with your receiver's MAC.
uint8_t receiverAddress[] = {0xf8, 0xb3, 0xb7, 0x30, 0x3b, 0x10};

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  // Note: don't rely on info internals across ESP core versions; we only print status.
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  if (duration == 0) return -1; // no echo
  long distance = duration * 0.034 / 2; // cm
  if (distance <= 0 || distance > 400) return -1;
  return distance;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // WiFi must be in STA mode for ESP-NOW
  WiFi.mode(WIFI_STA);
  delay(100);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }

  // register send callback (new signature)
  esp_err_t cbres = esp_now_register_send_cb(OnDataSent);
  if (cbres != ESP_OK) {
    Serial.printf("Failed to register send cb: %d\n", cbres);
  }

  // add peer (unicast to receiver)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer (may already exist)");
    // continue anyway
  }

  Serial.printf("Sender ready. SENSOR_ID=%d\n", SENSOR_ID);
}

void loop() {
  long distance = getDistance();
  myData.id = SENSOR_ID;
  if (distance > 0) {
    snprintf(myData.text, sizeof(myData.text), "@%d, %ld cm", SENSOR_ID, distance);
  } else {
    snprintf(myData.text, sizeof(myData.text), "@%d clear", SENSOR_ID);
  }

  // send to receiver (unicast)
  esp_err_t res = esp_now_send(receiverAddress, (uint8_t *)&myData, sizeof(myData));
  if (res != ESP_OK) {
    Serial.printf("esp_now_send error: %d\n", res);
    // optional fallback: broadcast
    // esp_now_send(NULL, (uint8_t *)&myData, sizeof(myData));
  } else {
    Serial.printf("Sent: id=%d text=%s\n", myData.id, myData.text);
  }

  delay(1000); // send every 1 second
}
