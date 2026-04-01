#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "NODE1";

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("TRANSMITTER STARTING...");

  if (!radio.begin()) {
    Serial.println("NRF NOT DETECTED ❌");
    while (1);
  }

  Serial.println("NRF READY ✅");

  radio.setAutoAck(false);

  radio.openWritingPipe(address);
  radio.setChannel(108);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);

  radio.stopListening();  // MUST for TX
}

void loop() {
  const char text[] = "Samrudhhi Atkare";

  Serial.println("SENDING DATA...");

  bool result = radio.write(&text, sizeof(text));

  if (result) {
    Serial.println("SEND SUCCESS ✅");
  } else {
    Serial.println("SEND FAILED ❌");
  }

  delay(1000);
}


Worked