#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

typedef struct struct_message {
  int id;
  char text[32];
} struct_message;

struct_message incomingData;
String esp1_data = "@1 clear";
String esp2_data = "@2 clear";

unsigned long lastRecvTime1 = 0;
unsigned long lastRecvTime2 = 0;

void showOLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(esp1_data);

  display.setCursor(0, 30);
  display.println(esp2_data);

  display.display();
}

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataBytes, int len) {
  struct_message data;
  memcpy(&data, incomingDataBytes, sizeof(data));

  if (data.id == 1) {
    esp1_data = data.text;
    lastRecvTime1 = millis();
  } else if (data.id == 2) {
    esp2_data = data.text;
    lastRecvTime2 = millis();
  }

  showOLED();
  Serial.print("ESP");
  Serial.print(data.id);
  Serial.print(": ");
  Serial.println(data.text);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed!");
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Waiting for sensors...");
  display.display();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Agar sensor inactive → 5 sec se data nahi aaya → clear dikhao
  if (millis() - lastRecvTime1 > 5000) esp1_data = "@1 clear";
  if (millis() - lastRecvTime2 > 5000) esp2_data = "@2 clear";

  showOLED();
}
