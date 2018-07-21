#include <Wire.h>

uint8_t receiver_addr = 0x42;  // address of GPS
uint16_t data_reg = 0xFF;  // register for GPS data stream

void setup() {
  Serial.begin(9600); // initialize Serial communication
  while (!Serial);    // wait for the serial port to open
  Serial.println("on");
  Wire.begin();

}

void loop() {
  Wire.requestFrom(receiver_addr, data_reg);
  Serial.println(Wire.available());
  while(Wire.available()) {
    char c = Wire.read();
    Serial.print(c);
  }

  delay(100);
}
