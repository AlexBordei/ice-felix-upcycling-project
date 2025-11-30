#include <SoftwareSerial.h>

SoftwareSerial SerialOne(D1, D2); // RX, TX

void setup() {
  Serial.begin(115200);
  SerialOne.begin(9600);

  delay(1000);

  SerialOne.println();
  SerialOne.println();
  SerialOne.println("=== ASCII DEBUG ===");
  SerialOne.println("Press any key...");
  SerialOne.println();
}

void loop() {
  while (SerialOne.available()) {
    int c = SerialOne.read();

    // Show ASCII code prominently
    SerialOne.print(">>> ");
    SerialOne.print(c);
    SerialOne.print(" <<<");
    SerialOne.println();
  }
}
