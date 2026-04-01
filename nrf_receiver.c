#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "NODE1";

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("RECEIVER STARTING...");

  if (!radio.begin()) {
    Serial.println("NRF NOT DETECTED ❌");
    while (1);
  }

  Serial.println("NRF READY ✅");

  radio.openReadingPipe(0, address);
  radio.setChannel(108);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);

  radio.startListening();  // MUST for RX
}

void loop() {
  if (radio.available()) {
    char text[32] = "";

    radio.read(&text, sizeof(text));

    Serial.print("Received: ");
    Serial.println(text);
  }
}