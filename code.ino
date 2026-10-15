#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define OLED_ADDRESS 0x3C

SSD1306AsciiWire oled;
MAX30105 sensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE] = {0};
byte rateSpot = 0;
byte rateCount = 0;

unsigned long lastBeat = 0;
unsigned long lastDisplay = 0;
int averageBPM = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
  delay(1000);

  Serial.println(F("Initializing OLED..."));

  Wire.beginTransmission(OLED_ADDRESS);
  if (Wire.endTransmission() != 0) {
    Serial.println(F("OLED not found on I2C"));
    return;
  }

  oled.begin(&Adafruit128x64, OLED_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();

  Serial.println(F("OLED initialized!"));

  Serial.println(F("Initializing MAX3010x..."));

  if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println(F("MAX3010x initialization failed!"));
    oled.println(F("Sensor failed"));
    return;
  }

  sensor.setup();
  sensor.setPulseAmplitudeRed(0x1F);
  sensor.setPulseAmplitudeIR(0x1F);

  Serial.println(F("MAX3010x initialized!"));

  oled.println(F("Heart Rate Monitor"));
  oled.println(F("------------------"));
}

void loop() {
  long irValue = sensor.getIR();

  if (irValue < 10000) {
    rateCount = 0;
    rateSpot = 0;
    averageBPM = 0;
    lastBeat = 0;
  }
  else if (checkForBeat(irValue)) {
    unsigned long now = millis();

    if (lastBeat != 0) {
      unsigned long delta = now - lastBeat;

      if (delta > 0) {
        float bpm = 60000.0 / delta;

        if (bpm >= 40 && bpm <= 200) {
          rates[rateSpot] = (byte)bpm;
          rateSpot = (rateSpot + 1) % RATE_SIZE;

          if (rateCount < RATE_SIZE) {
            rateCount++;
          }

          int total = 0;

          for (byte i = 0; i < rateCount; i++) {
            total += rates[i];
          }

          averageBPM = total / rateCount;

          Serial.print(F("BPM: "));
          Serial.println(averageBPM);
        }
      }
    }

    lastBeat = now;
  }

  // Update display every 250 ms
  if (millis() - lastDisplay >= 250) {
    lastDisplay = millis();

    oled.setCursor(0, 2);
    oled.print(F("IR: "));
    oled.print(irValue);
    oled.print(F("          "));

    oled.setCursor(0, 4);
    oled.print(F("BPM: "));

    if (averageBPM > 0) {
      oled.print(averageBPM);
    } else {
      oled.print(F("--"));
    }

    oled.print(F("          "));

    oled.setCursor(0, 6);

    if (irValue < 10000) {
      oled.print(F("No finger detected "));
    } else {
      oled.print(F("Finger detected    "));
    }
  }
}
