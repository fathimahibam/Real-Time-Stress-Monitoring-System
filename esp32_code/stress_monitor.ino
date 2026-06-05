#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000
#define FIREBASE_UPDATE_INTERVAL 8000

// WiFi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Firebase Credentials
#define DATABASE_URL "YOUR_FIREBASE_DATABASE_URL"
#define API_KEY "YOUR_FIREBASE_API_KEY"

#define GSR_Pin 34

FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

PulseOximeter pox;

uint32_t tsLastReport = 0;

#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress;

int resolution = 12;
unsigned long lastTempRequest = 0;
int delayInMillis = 0;

float temperature = 0.0;
float heartRate = 0.0;
float spo2 = 0.0;
float gsr = 0.0;

unsigned long lastFirebaseUpdate = 0;

bool signupOK = false;

void onBeatDetected()
{
    Serial.print("Beat! : ");
    Serial.println(heartRate);
}

void configureMax30100()
{
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void setup()
{
    Serial.begin(9600);

    Serial.println("Stress Monitoring System");

    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();
    Serial.print("Connected IP: ");
    Serial.println(WiFi.localIP());

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    if (Firebase.signUp(&config, &auth, "", ""))
    {
        signupOK = true;
        Serial.println("Firebase Signup OK");
    }
    else
    {
        Serial.printf("%s\n",
                      config.signer.signupError.message.c_str());
    }

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    sensors.begin();

    if (!sensors.getAddress(tempDeviceAddress, 0))
    {
        Serial.println("DS18B20 Not Found");
    }

    sensors.setResolution(tempDeviceAddress, resolution);
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();

    delayInMillis = 750 / (1 << (12 - resolution));
    lastTempRequest = millis();

    Wire.setClock(400000UL);

    Serial.print("Initializing MAX30100...");

    if (!pox.begin())
    {
        Serial.println("FAILED");
        while (1);
    }
    else
    {
        Serial.println("SUCCESS");
    }

    configureMax30100();
}

void loop()
{
    pox.update();

    if (millis() - lastTempRequest >= 2000)
    {
        temperature = sensors.getTempCByIndex(0);

        gsr = analogRead(GSR_Pin);

        resolution++;

        if (resolution > 12)
            resolution = 9;

        sensors.setResolution(tempDeviceAddress, resolution);

        sensors.requestTemperatures();

        delayInMillis = 750 / (1 << (12 - resolution));

        lastTempRequest = millis();
    }

    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    {
        heartRate = pox.getHeartRate();
        spo2 = pox.getSpO2();

        tsLastReport = millis();

        Serial.print("Heart Rate: ");
        Serial.println(heartRate);

        Serial.print("SpO2: ");
        Serial.println(spo2);

        Serial.print("Temperature: ");
        Serial.println(temperature);

        Serial.print("GSR: ");
        Serial.println(gsr);
    }

    if (millis() - lastFirebaseUpdate >= FIREBASE_UPDATE_INTERVAL)
    {
        pox.shutdown();

        if (heartRate > 40 && heartRate < 240)
        {
            Firebase.RTDB.setFloat(
                &fbdo,
                "/health-monitor/heart_rate",
                heartRate);
        }

        if (spo2 > 70)
        {
            Firebase.RTDB.setFloat(
                &fbdo,
                "/health-monitor/spo2",
                spo2);
        }

        Firebase.RTDB.setFloat(
            &fbdo,
            "/health-monitor/temperature",
            temperature);

        Firebase.RTDB.setFloat(
            &fbdo,
            "/health-monitor/gsr",
            gsr);

        pox.resume();

        lastFirebaseUpdate = millis();
    }

    delay(1);
}