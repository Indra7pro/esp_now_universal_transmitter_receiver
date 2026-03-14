#include <esp_now.h>
#include <WiFi.h>

#define TX_ID 12345678

#define CH1_PIN 36
#define CH2_PIN 39
#define CH3_PIN 34
#define CH4_PIN 35
#define BTN1 16
#define BTN2 17

// Lock to a specific channel (must match receiver)
#define ESPNOW_CHANNEL 1

typedef struct {
  uint16_t header;
  uint32_t txID;
  uint16_t ch[6];
} Packet;

Packet data;

// Replace with your receiver's actual MAC address for unicast
uint8_t receiverAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// Fast multi-sample ADC read to reduce noise
uint16_t readADC(int pin) {
  uint32_t sum = 0;
  for (int i = 0; i < 4; i++) sum += analogRead(pin);
  return map(sum >> 2, 0, 4095, 1000, 2000); // >>2 = divide by 4
}

void setup() {
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_now_init();

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = ESPNOW_CHANNEL;  // Fixed channel
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  esp_now_add_peer(&peerInfo);
}

unsigned long lastSend = 0;

void loop() {
  unsigned long now = micros();
  
  // Send every 10ms = 100Hz — sweet spot for RC (was unlimited or 20ms)
  if (now - lastSend >= 10000) {
    lastSend = now;

    data.header = 0xAA55;
    data.txID = TX_ID;

    data.ch[0] = readADC(CH1_PIN);
    data.ch[1] = readADC(CH2_PIN);
    data.ch[2] = readADC(CH3_PIN);
    data.ch[3] = readADC(CH4_PIN);
    data.ch[4] = digitalRead(BTN1) ? 2000 : 1000;
    data.ch[5] = digitalRead(BTN2) ? 2000 : 1000;

    esp_now_send(receiverAddress, (uint8_t*)&data, sizeof(data));
  }
}