#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

#define BOUND_TX_ID 12345678
#define ESPNOW_CHANNEL 1  // Must match transmitter

#define SERVO1_PIN 15
#define SERVO2_PIN 4
#define SERVO3_PIN 16
#define SERVO4_PIN 17
#define SERVO5_PIN 5

Servo servo1, servo2, servo3, servo4, servo5;

typedef struct {
  uint16_t header;
  uint32_t txID;
  uint16_t ch[6];
} Packet;

// Double-buffer: ISR writes to incoming, loop reads from active
volatile bool newData = false;
Packet incoming, active;

unsigned long lastPacket = 0;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  if (len != sizeof(Packet)) return;
  memcpy(&incoming, incomingData, sizeof(Packet));
  if (incoming.header != 0xAA55) return;
  if (incoming.txID != BOUND_TX_ID) return;
  lastPacket = millis();
  newData = true;  // Signal main loop — don't call servo from ISR
}

void setup() {
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);

  servo1.attach(SERVO1_PIN, 500, 2500);
  servo2.attach(SERVO2_PIN, 500, 2500);
  servo3.attach(SERVO3_PIN, 500, 2500);
  servo4.attach(SERVO4_PIN, 500, 2500);
  servo5.attach(SERVO5_PIN, 500, 2500);
  
  servo1.writeMicroseconds(1500);
  servo2.writeMicroseconds(1500);
  servo3.writeMicroseconds(1500);
  servo4.writeMicroseconds(1500);
  servo5.writeMicroseconds(1500);
}

void loop() {
  if (newData) {
    newData = false;
    memcpy(&active, &incoming, sizeof(Packet));


    servo1.writeMicroseconds(3000-active.ch[0]);
    servo2.writeMicroseconds(3000-active.ch[0]);

    servo3.writeMicroseconds(active.ch[2]);
    servo4.writeMicroseconds(active.ch[3]);
    servo5.writeMicroseconds(active.ch[1]);
  }

  // Failsafe: signal lost > 250ms
  if (millis() - lastPacket > 250) {
    servo1.writeMicroseconds(1500);
    servo2.writeMicroseconds(1500);
    servo3.writeMicroseconds(1500);
    servo4.writeMicroseconds(1500);
    servo5.writeMicroseconds(1500);
  }
}