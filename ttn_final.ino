#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "DHT.h"

// Initialisation GPS
TinyGPSPlus gps;

// DHT11 configuration
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Clés pour LoRaWAN
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }

static const u1_t PROGMEM DEVEUI[8] = { 0xDC, 0xB3, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }

static const u1_t PROGMEM APPKEY[16] = { 0x5C, 0x02, 0xBD, 0xF7, 0x6A, 0x62, 0xF7, 0x25, 0x8E, 0x06, 0xE8, 0x29, 0xEF, 0xF0, 0x95, 0x41 };
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

// Pin mapping pour Feather M0 LoRa
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,
    .spi_freq = 8000000,
};

static uint8_t payload[10]; // Payload étendu pour inclure les coordonnées GPS
static osjob_t sendjob;
const unsigned TX_INTERVAL = 10000; // Intervalle d'envoi (30 secondes)

// Fonction pour lire la fréquence cardiaque
uint8_t readBPM() {
    Wire.requestFrom(0xA0 >> 1, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}

// Gestion des événements LoRaWAN
void onEvent(ev_t ev) {
    switch (ev) {
        case EV_JOINING:
            Serial.println(F("Joining..."));
            break;
        case EV_JOINED:
            Serial.println(F("Joined!"));
            LMIC_setLinkCheckMode(0);
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("Transmission complete!"));
            break;
        default:
            Serial.print(F("Event: "));
            Serial.println((unsigned)ev);
            break;
    }
}

// Fonction principale pour l'envoi des données
void do_send() {
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
        return;
    }

    // Lecture des capteurs
    float temperature = dht.readTemperature();
    float rHumidity = dht.readHumidity();
    uint8_t bpm = readBPM();

    // Gestion du GPS
    float latitude = 0.0;
    float longitude = 0.0;
    bool valid_gps;
    while (Serial1.available() > 0) {
        char c = Serial1.read();
        gps.encode(c);

        if (gps.location.isValid()) {
          latitude = gps.location.lat();
          longitude = gps.location.lng();
          valid_gps=true;
        }
        else{
          valid_gps=false;
        }
    }

    if (isnan(temperature) || isnan(rHumidity)) {
        Serial.println("Erreur lors de la lecture des capteurs !");
        return;
    }

    Serial.print("Température : "); Serial.println(temperature);
    Serial.print("Humidité : "); Serial.println(rHumidity);
    Serial.print("BPM : "); Serial.println(uint16_t(bpm));
    Serial.print("Latitude : "); Serial.println(latitude, 6);
    Serial.print("Longitude : "); Serial.println(longitude, 6);

  temperature = constrain(temperature, 0.0, 50.0);
  temperature = (temperature / 50.0) * 65535;
  uint16_t payloadTemp = uint16_t(temperature);
  rHumidity = constrain(rHumidity, 20.0, 90.0);

// Conversion de l'humidité dans la plage 0-65535
  rHumidity = ((rHumidity - 20.0) / 70.0 * 65535);
  uint16_t payloadHumid= uint16_t(rHumidity);
  // Latitude et longitude
  uint16_t payloadLat = uint16_t(((latitude + 90.0) / 180.0) * 65535);
  uint16_t payloadLng = uint16_t(((longitude + 180.0) / 360.0) * 65535);
  // Construction du payload
  payload[0] = lowByte(payloadTemp);
  payload[1] = highByte(payloadTemp);
  payload[2] = lowByte(payloadHumid);
  payload[3] = highByte(payloadHumid);
  payload[4] = uint16_t(bpm);
  payload[5] = lowByte(payloadLat);
  payload[6] = highByte(payloadLat);
  payload[7] = lowByte(payloadLng);
  payload[8] = highByte(payloadLng);
  payload[9] = uint16_t(valid_gps);


    Serial.print("Payload : ");
    for (int i = 0; i < sizeof(payload); i++) {
        Serial.print(payload[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Envoi des données
    LMIC_setTxData2(1, payload, sizeof(payload), 0);
    Serial.println(F("Données envoyées !"));
}

// Configuration initiale
void setup() {
    Serial.begin(115200);
    Serial.println(F("Démarrage..."));
    Serial1.begin(9600); // Initialisation du GPS
    dht.begin(); // Initialisation DHT11
    Wire.begin();
    // Initialisation LMIC
    os_init();
    LMIC_reset();
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
    LMIC_startJoining();
}

// Boucle principale
void loop() {
    os_runloop_once();

    static unsigned long previousMillis = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= TX_INTERVAL) {
        previousMillis = currentMillis;
        LMIC_reset();
        do_send();
    }
}
