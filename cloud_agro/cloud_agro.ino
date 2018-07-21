/*
  ===============================================
  Adapted from RawSerialImuData example from CurieImu library
  ===============================================
  Script for execution by Cloud Agronomics Udoo x86 onboard Arduino 101.
  Delivers readings from onboard IMU and external I2C GPS sensor via
  serial to Udoo x86. Use with "read_gps.py" script.

  Author: David Schurman
  Last Modified: 7/16/18
  ===============================================
*/

#include "CurieIMU.h"
#include <Wire.h>
#include "CurieTime.h"

int ax, ay, az;         // accelerometer values
int gx, gy, gz;         // gyrometer values

int count = 0;

int gpio_pin = 3;
uint8_t receiver_addr = 0x42;  // I2C address of GPS
uint16_t data_reg = 0xFF;  // register for GPS data stream

int calibrateOffsets = 1; // int to determine whether calibration takes place or not
//int buflen = 512;
char gps_buf[512];

void setup() {
  pinMode(gpio_pin, INPUT);
  Serial.begin(115200); // initialize Serial communication
  while (!Serial);    // wait for the serial port to open

  // bool start_flag = false;
  while(!digitalRead(gpio_pin)) {  // wait for HIGH signal from pixhawk
    delay(100);
  }
  Serial.println("GPIO SIGNALED");
  Serial.println("Initializing I2C device...");
  Wire.begin(receiver_addr);
  Wire.setClock(400000L);
  // initialize device
  Serial.println("Initializing IMU device...");
  CurieIMU.begin();

  // verify connection
  Serial.println("Testing device connections...");
  if (CurieIMU.begin()) {
    Serial.println("CurieIMU connection successful");
  } else {
    Serial.println("CurieIMU connection failed");
  }

  // sync time 
//  Serial.println("SYNC");
//  while (!Serial.available());  // wait for incoming message
//  String t_sync = Serial.readString();  // store Udoo time
//  Serial.print(micros());  // time since start of program in microseconds
//  Serial.println(" microseconds now");
//  Serial.print("SYNCED, start time ");
//  Serial.println(t_sync);

  while (!Serial.available());
  //String go = Serial.readString();
  /* To determine time for each datapoint, add millisecond timestamp
     to start sync time. */
  for (int i = 0; i < 100; i++) {
    Serial.print(micros());
    Serial.println(",a/g,###,###,#####,###,###,###");
    delay(10);
  }
  Serial.println("DONE SYNC");
  
  

  // use the code below to calibrate accel/gyro offset values
  if (calibrateOffsets == 1) {
    Serial.println("Internal sensor offsets BEFORE calibration...");
    Serial.print(CurieIMU.getAccelerometerOffset(X_AXIS));
    Serial.print("\t"); // -76
    Serial.print(CurieIMU.getAccelerometerOffset(Y_AXIS));
    Serial.print("\t"); // -235
    Serial.print(CurieIMU.getAccelerometerOffset(Z_AXIS));
    Serial.print("\t"); // 168
    Serial.print(CurieIMU.getGyroOffset(X_AXIS));
    Serial.print("\t"); // 0
    Serial.print(CurieIMU.getGyroOffset(Y_AXIS));
    Serial.print("\t"); // 0
    Serial.println(CurieIMU.getGyroOffset(Z_AXIS));

    // To manually configure offset compensation values,
    // use the following methods instead of the autoCalibrate...() methods below
    //CurieIMU.setAccelerometerOffset(X_AXIS,495.3);
    //CurieIMU.setAccelerometerOffset(Y_AXIS,-15.6);
    //CurieIMU.setAccelerometerOffset(Z_AXIS,491.4);
    //CurieIMU.setGyroOffset(X_AXIS,7.869);
    //CurieIMU.setGyroOffset(Y_AXIS,-0.061);
    //CurieIMU.setGyroOffset(Z_AXIS,15.494);

    Serial.println("About to calibrate. Make sure your board is stable and upright");
    delay(5000);

    // The board must be resting in a horizontal position for
    // the following calibration procedure to work correctly!
    Serial.print("Starting Gyroscope calibration and enabling offset compensation...");
    CurieIMU.autoCalibrateGyroOffset();
    Serial.println(" Done");

    Serial.print("Starting Acceleration calibration and enabling offset compensation...");
    CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 1);
    Serial.println(" Done");

    Serial.println("Internal sensor offsets AFTER calibration...");
    Serial.print(CurieIMU.getAccelerometerOffset(X_AXIS));
    Serial.print("\t"); // -76
    Serial.print(CurieIMU.getAccelerometerOffset(Y_AXIS));
    Serial.print("\t"); // -2359
    Serial.print(CurieIMU.getAccelerometerOffset(Z_AXIS));
    Serial.print("\t"); // 1688
    Serial.print(CurieIMU.getGyroOffset(X_AXIS));
    Serial.print("\t"); // 0
    Serial.print(CurieIMU.getGyroOffset(Y_AXIS));
    Serial.print("\t"); // 0
    Serial.println(CurieIMU.getGyroOffset(Z_AXIS));
  }

  Serial.println("IMU data format: microseconds,a/g (tag),ax,ay,az,gx,gy,gz");
}

void loop() {
  // read raw accel/gyro measurements from device
  CurieIMU.readMotionSensor(ax, ay, az, gx, gy, gz);
  
  // timestamp
  Serial.print(micros());
  // display comma-separated accel/gyro x/y/z values
  Serial.print(",a/g,");
  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.print(az);
  Serial.print(",");
  Serial.print(gx);
  Serial.print(",");
  Serial.print(gy);
  Serial.print(",");
  Serial.println(gz);
  
  delay(10);
  
  // read GPS at every 2 iterations
  if (!(count % 2)) {
    int gps_count = 0;
    memset(gps_buf, 0, 512);
//    Serial.print(micros());
//    Serial.println(" micros (GPS)");
    Wire.requestFrom(receiver_addr, data_reg);
    while(Wire.available()) {
      char c = Wire.read();
      // Serial.print(c);
//      if (c == 'M') {
//        Serial.print('#');
//        Serial.println(micros());
//      }
      gps_buf[gps_count] = c;
      gps_count ++;
    }
    Serial.write(gps_buf, gps_count);
    Serial.println();
  }
  count ++;
  delay(40);  // read every 50 milliseconds (20 Hz)
}

