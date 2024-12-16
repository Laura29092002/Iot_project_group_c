
# **LoRaWAN Project with Feather M0 and TTN**

## **Description**
This project uses an **Adafruit Feather M0 LoRa** board to collect data from multiple sensors (temperature, humidity, heart rate, and GPS), encode the data into a compact format, and transmit it to **The Things Network (TTN)** using LoRaWAN.

---

## **Required Hardware**
1. **Microcontroller**: Adafruit Feather M0 LoRa.
2. **Sensors**:
   - **DHT11**: Temperature and humidity sensor.
   - **GT-U7 GPS Module**: For location coordinates.
   - **Grove Heart Rate Sensor**.
3. **Required Libraries**:
   - LMIC (LoRaWAN): For LoRa communication.
   - TinyGPS++: For GPS data handling.
   - DHT: For reading the DHT11 sensor.
   - Wire (I2C): For heart rate sensor data.

---

## **Architecture**
The system follows this flow:
1. **Data Acquisition**:
   - Temperature and humidity via the **DHT11** sensor.
   - GPS position (latitude, longitude) via the **GT-U7 module**.
   - Heart rate via the **Grove heart rate sensor**.
2. **Data Encoding**:
   - The data is scaled and converted into a **16-bit format** to minimize payload size.
3. **Data Transmission**:
   - Encoded data is sent using **LoRaWAN** via the **LMIC** library.
   - The Feather M0 connects to **TTN**.
4. **Processing on TTN**:
   - TTN receives, decodes, and makes the data available for cloud applications.

---

## **Payload Structure**
The payload contains the following information, encoded in **10 bytes**:

| **Index** | **Data**              | **Size**   | **Description**                |
|-----------|-----------------------|------------|--------------------------------|
| 0-1       | Temperature           | 2 bytes    | Scale: 0°C - 50°C              |
| 2-3       | Humidity              | 2 bytes    | Scale: 20% - 90%               |
| 4         | Heart Rate            | 1 byte     | In BPM (0-255)                 |
| 5-6       | Latitude              | 2 bytes    | Scale: -90° to 90°             |
| 7-8       | Longitude             | 2 bytes    | Scale: -180° to 180°           |
| 9         | GPS Validity          | 1 byte     | 1 = Valid, 0 = Invalid         |

---

## **Project Setup**

### **1. Install Required Libraries**
Make sure to install the following libraries via the **Library Manager** in Arduino IDE:
- **LMIC** (MCCI LoRaWAN Library)
- **TinyGPS++**
- **DHT**
- **Wire** (included by default).

### **2. Configure LoRaWAN Keys**
Modify the **DevEUI**, **AppEUI**, and **AppKey** to match those provided by **The Things Network**:
```cpp
static const u1_t PROGMEM DEVEUI[8] = { DevEUI };
static const u1_t PROGMEM APPEUI[8] = { AppEUI };
static const u1_t PROGMEM APPKEY[16] = { AppKey };
```

### **3. Sensor Connections**
| **Component**              | **Pin**                   |
|----------------------------|---------------------------|
| DHT11 (Data)               | Pin 12                   |
| GPS (Serial)               | UART (Serial1) at 9600 bps |
| Heart Rate Sensor (I2C)    | SDA/SCL                  |

