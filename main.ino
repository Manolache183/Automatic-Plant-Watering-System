#include "LedControl.h"
#include "LowPower.h"

/*** 8x8 Matrix ***/
#define CS 10
#define DIN 12
#define CLK 11
#define EMPTY_LED 0x00 // The value to be sent to the matrix to turn off a LED

/*** Ultrasonic Sensor ***/
#define TANK_HEIGHT 12 // Distance from the sensor, down to the bottom of the tank (in cm)
#define FILL_HEIGHT 9 // Distance from the bottom of the tank, up to the maximum desired liquid height (in cm)
#define WATER_LEVEL_THRESHOLD 20 // The minimum water level at which the water tank should be refilled (percentage)
#define TRIG_PIN 9
#define ECHO_PIN 8
#define DISTANCE_SENSOR_POWER 3

/*** Soil Moisture Sensor ***/
#define MOISTURE_THRESHOLD 800 // The minimum moisture level at which the pump should be turned on (percentage)
#define MOISTURE_SENSOR_POWER 2
#define SENSOR_PIN A0

/*** Water Pump ***/
#define PUMP_POWER 5

/*** Other ***/
#define SLEEP_TIME 8000 // Sleep time in milliseconds
#define EMPTY_LED 0x00 // The value to be sent to the matrix to turn off a LED

byte heart[8] = {0x00, 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00};
byte beer1[8] = {0xAC, 0xD4, 0xAF, 0x85, 0x85, 0x85, 0x87, 0xFC};
byte beer2[8] = {0xD4, 0xAC, 0xD7, 0x85, 0x85, 0x85, 0x87, 0xFC};
byte waterDrop[8] = {0x08, 0x1C, 0x3E, 0x6E, 0xEF, 0xDF, 0X7E, 0x3C};

LedControl lc=LedControl(DIN,CLK,CS,1);

int readWaterLevel() {
  digitalWrite(DISTANCE_SENSOR_POWER, HIGH);  // Turn the sensor ON
  delay(100); // Allow power to settle

  // Measure the distance
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long t = pulseIn(ECHO_PIN, HIGH);

  int distanceMeasured = t / 58; // Calculate the distance in cm
  int distanceSensorToFill = TANK_HEIGHT - FILL_HEIGHT;

  // Map the given value between 0 and 100, to obtain a percentage
  int tankLevel = map(distanceMeasured, distanceSensorToFill, TANK_HEIGHT, 100, 0);

  if (tankLevel > 100)
    tankLevel = 100;
  if (tankLevel < 0)
    tankLevel = 0;

  digitalWrite(DISTANCE_SENSOR_POWER, LOW); // Turn the sensor OFF

  return tankLevel;
}

int readSoilMoisture() {
  digitalWrite(MOISTURE_SENSOR_POWER, HIGH); // Turn the sensor ON
  delay(10); // Allow power to settle
  
  int moisture = analogRead(SENSOR_PIN); // Measure the moisture
  
  digitalWrite(MOISTURE_SENSOR_POWER, LOW); // Turn the sensor OFF

  return moisture;
}

void setup() {
  /*** 8x8 Matrix ***/
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);

  /**** Ultrasonic Sensor ****/
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(DISTANCE_SENSOR_POWER, OUTPUT);
  digitalWrite(DISTANCE_SENSOR_POWER, LOW); // Turn the sensor OFF

  /**** Soil Moisture Sensor ****/
  pinMode(MOISTURE_SENSOR_POWER, OUTPUT);
  digitalWrite(MOISTURE_SENSOR_POWER, LOW); // Turn the sensor OFF

  /**** Water Pump ****/
  pinMode(PUMP_POWER, OUTPUT);
  digitalWrite(PUMP_POWER, LOW); // Turn the pump OFF
}

void loop() {
  int waterLevel = readWaterLevel();

  if (waterLevel < WATER_LEVEL_THRESHOLD) {
    // Display a warning on the matrix and keep the pump turned off

    digitalWrite(PUMP_POWER, LOW);

    for (int i = 0; i < 8; i++) {
      lc.setRow(0, i, waterDrop[i]);
    }

    delay(300);
  } else {
    int soilMoisture = readSoilMoisture();

    if (soilMoisture > MOISTURE_THRESHOLD) {
      // Turn on the pump, display an animation with a beer on the matrix

      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, beer1[i]);
      }

      delay(200);

      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, beer2[i]);
      }

      delay(200);

      digitalWrite(PUMP_POWER, HIGH);
    } else {
      // Turn off the pump, display a heart on the matrix, put the board to sleep

      digitalWrite(PUMP_POWER, LOW);

      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, heart[i]);
      }

      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
  }
}
