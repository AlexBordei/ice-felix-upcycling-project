#include <SoftwareSerial.h>

SoftwareSerial SerialOne(D1, D2); // RX, TX

void setup() {
  Serial.begin(115200);
  SerialOne.begin(9600);

  delay(1000);

  SerialOne.println();
  SerialOne.println("=== WIDTH TEST ===");
  SerialOne.println();

  // Test diferite lungimi
  SerialOne.println("10 chars:");
  SerialOne.println("1234567890");
  SerialOne.println();

  SerialOne.println("20 chars:");
  SerialOne.println("12345678901234567890");
  SerialOne.println();

  SerialOne.println("40 chars:");
  SerialOne.println("1234567890123456789012345678901234567890");
  SerialOne.println();

  SerialOne.println("60 chars:");
  SerialOne.println("123456789012345678901234567890123456789012345678901234567890");
  SerialOne.println();

  SerialOne.println("80 chars:");
  SerialOne.println("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
  SerialOne.println();

  SerialOne.println("100 chars:");
  SerialOne.println("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
  SerialOne.println();

  // Rigla cu numere
  SerialOne.println("Rigla (fiecare 10):");
  SerialOne.println("----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|");
  SerialOne.println("         1         2         3         4         5         6         7         8         9        10");
  SerialOne.println();

  SerialOne.println("Spune-mi unde se taie textul!");
}

void loop() {
}
