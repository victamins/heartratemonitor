# Heart Rate Monitor
MAX30102 Heart Rate Sensor with LCD Display

<img width="2302" height="2302" alt="image1" src="https://github.com/user-attachments/assets/6621b94a-4035-457d-b6aa-2cfc499bbd43" /><img width="807" height="619" alt="Screenshot 2026-10-14 at 11 10 00 PM" src="https://github.com/user-attachments/assets/eec28bf2-ee35-4737-a7e2-43c7125982a1" />


# ESP32/Arduino Heart Rate Monitor

A simple heart rate monitoring project using a **MAX30102 pulse sensor** and an **SSD1306 OLED display**. The device detects heartbeats using infrared (IR) light measurements, calculates beats per minute (BPM), and displays the readings in real time.

## Software and Libraries

The project uses the following Arduino libraries:

- `Wire` — I²C communication
- `SSD1306Ascii` — lightweight SSD1306 OLED display control
- `SparkFun MAX3010x Sensor Library` — sensor initialization and readings
- `heartRate.h` — heartbeat detection algorithm

### Calculate beats per minute

Heart rate is calculated using the time interval between consecutive detected beats.

Let:

- \( \Delta t \) = time between two consecutive beats, in milliseconds
- \(60{,}000\) = number of milliseconds in one minute
- \(BPM\) = estimated beats per minute

The equation is:

\[
\boxed{BPM = \frac{60{,}000}{\Delta t}}
\]

For example, suppose two consecutive beats are detected 800 milliseconds apart.

\[
BPM = \frac{60{,}000}{800}
= 75
\]

The estimated heart rate is **75 BPM**.

The code implements this calculation as follows:

```cpp
unsigned long delta = now - lastBeat;

if (delta > 0) {
    float bpm = 60000.0 / delta;

    if (bpm >= 40 && bpm <= 200) {
        rates[rateSpot] = (byte)bpm;
    }
}
```

The program accepts calculated values between 40 and 200 BPM to filter out some implausible readings. This range is a programming choice, not a medical diagnostic threshold.

The first detected beat only establishes a reference time. A BPM value can be calculated after the next beat is detected.

### 5. Smooth the readings with a rolling average

Individual measurements may fluctuate because of movement, sensor noise, or imperfect finger placement. To reduce these fluctuations, the program stores up to four accepted BPM values.

```cpp
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE] = {0};
```

Each new accepted value is stored in a circular buffer. Once the buffer reaches its capacity, the oldest value is replaced by the newest.

The average heart rate is calculated using:

\[
\boxed{
\overline{BPM} =
\frac{1}{N}\sum_{i=1}^{N} BPM_i
}
\]

where \(N\) is the number of valid readings currently stored.

For example, if the four stored readings are 74, 76, 75, and 77 BPM:

\[
\overline{BPM}
= \frac{74+76+75+77}{4}
= 75.5
\]

Because the code uses integer division when calculating the average, the displayed value is **75 BPM**.

This averaging method reduces short-term fluctuations but does not eliminate all measurement errors.

## OLED Display

The display presents three main pieces of information:

| Display field | Description |
|---|---|
| `IR` | Current infrared sensor reading |
| `BPM` | Average calculated heart rate |
| Finger status | Indicates whether the IR reading exceeds the detection threshold |

The display is refreshed approximately every 250 milliseconds:

```cpp
if (millis() - lastDisplay >= 250) {
    lastDisplay = millis();
}
```

The refresh frequency is approximately:

\[
f = \frac{1}{T}
= \frac{1}{0.25}
= 4 \text{ Hz}
\]

This means the display updates approximately four times per second. Meanwhile, the main loop continues sampling the sensor between display updates.

## Finger Detection

The program uses an infrared threshold to estimate whether a finger is present.

```cpp
if (irValue < 10000) {
    rateCount = 0;
    rateSpot = 0;
    averageBPM = 0;
    lastBeat = 0;
}
```

When the IR reading falls below `10000`, the program assumes that no finger is detected and resets the heart rate calculation.

When the reading exceeds the threshold, the display indicates that a finger is detected.

The threshold may need adjustment for different sensor modules, ambient lighting conditions, and finger placement. It is a simple heuristic rather than a definitive finger-detection measurement.

## Serial Monitor

Example Serial Output: 
<img width="1168" height="774" alt="Screenshot 2026-10-14 at 6 53 54 PM" src="https://github.com/user-attachments/assets/b3c0b848-b1dc-465e-9ee7-831901d07090" />
