#include <esp_now.h>
#include <WiFi.h>

#define TRIG_PIN 5
#define ECHO_PIN 18

typedef struct struct_message {
  int id;
  char text[32];
} struct_message;

struct_message myData;

// ESP3 (OLED) MAC address
uint8_t broadcastAddress[] = {0xf8, 0xb3, 0xb7, 0x30, 0x3b, 0x10};  

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
  if (distance <= 0 || distance > 400) return -1;
  return distance;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  long distance = getDistance();
  myData.id = 2;  // ESP1 ID
  if (distance > 0) {
    sprintf(myData.text, "@1, %ld cm", distance);
  } else {
    sprintf(myData.text, "@1 clear");
  }

  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  delay(1000);
}
