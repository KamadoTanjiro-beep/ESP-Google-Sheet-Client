
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: suwatchai@outlook.com
 *
 * Github: https://github.com/mobizt
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows how to clear the spreadsheet's values.

#include <Arduino.h>
#if defined(ESP32) || defined(PICO_RP2040)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ESP_Google_Sheet_Client.h>

#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// For how to create Service Account and how to use the library, go to https://github.com/mobizt/ESP-Google-Sheet-Client

#define PROJECT_ID "PROJECT_ID"

// Service Account's client email
#define CLIENT_EMAIL "CLIENT_EMAIL"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

bool taskComplete = false;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void tokenStatusCallback(TokenInfo info);

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    Serial.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

#if defined(ESP32) || defined(ESP8266)
    WiFi.setAutoReconnect(true);
#endif

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Set the callback for Google API access token generation status (for debug only)
    GSheet.setTokenCallback(tokenStatusCallback);

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    GSheet.clearAP();
    GSheet.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    // Begin the access token generation for Google API authentication
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop()
{
    // Call ready() repeatedly in loop for authentication checking and processing
    bool ready = GSheet.ready();

    if (ready && !taskComplete)
    {
        // For basic FirebaseJson usage example, see examples/FirebaseJson/Create_Edit_Parse/Create_Edit_Parse.ino

        // If you assign the spreadsheet id from your own spreadsheet,
        // you need to set share access to the Service Account's CLIENT_EMAIL

        FirebaseJson response;
        // Instead of using FirebaseJson for response, you can use String for response to the functions
        // especially in low memory device that deserializing large JSON response may be failed as in ESP8266

        Serial.println("\nWrite spreadsheet values in range...");
        Serial.println("--------------------------------------------------------------");

        FirebaseJson valueRange;

        valueRange.add("range", "Sheet1!A1:H10");
        valueRange.add("majorDimension", "ROWS");

        String path;
        for (size_t i = 0; i < 8; i++)
        {
            for (size_t j = 0; j < 10; j++)
            {
                path = "values/[";
                path += String(j);
                path += "]/[";
                path += String(i);
                path += "]";
                valueRange.set(path, "x");
            }
        }

        // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/update

        bool success = GSheet.values.update(&response /* returned response */, "<spreadsheetId>" /* spreadsheet Id to update */, "Sheet1!A1:H10" /* range to update */, &valueRange /* data to update */);
        response.toString(Serial, true);
        Serial.println();

        Serial.println("\nClear spreadsheet values in range...");
        Serial.println("--------------------------------------------------------------");

        // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/clear

        success = GSheet.values.clear(&response /* returned response */, "<spreadsheetId>" /* spreadsheet Id to clear */, "Sheet1!A1:A3" /* range to clear */);
        response.toString(Serial, true);
        Serial.println();

        // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/batchClear

        Serial.println("\nClear spreadsheet values in multiple ranges...");
        Serial.println("--------------------------------------------------------------");
        success = GSheet.values.batchClear(&response /* returned response */, "<spreadsheetId>" /* spreadsheet Id to clear */, "Sheet1!A5:A8,Sheet1!B1:C10" /* ranges to clear with comma separated */);
        response.toString(Serial, true);
        Serial.println();

        Serial.println("\nClear spreadsheet values from multiple ranges by data filter...");
        Serial.println("--------------------------------------------------------------");

        FirebaseJsonArray dataFiltersArr;

        FirebaseJson dataFilters1;
        dataFilters1.add("a1Range", "Sheet1!D5:E10");
        dataFiltersArr.add(dataFilters1);

        FirebaseJson dataFilters2;
        dataFilters2.add("a1Range", "Sheet1!F1:H3");
        dataFiltersArr.add(dataFilters2);

        // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/batchClearByDataFilter

        success = GSheet.values.batchClearByDataFilter(&response /* returned response */, "<spreadsheetId>" /* spreadsheet Id to clear */, &dataFiltersArr /* array of data range to read  with filter */);
        if (success)
            response.toString(Serial, true);
        else
            Serial.println(GSheet.errorReason());
        Serial.println();

#if defined(ESP32) || defined(ESP8266)
        Serial.println(ESP.getFreeHeap());
#elif defined(PICO_RP2040)
        Serial.println(rp2040.getFreeHeap());
#endif

        taskComplete = true;
    }
}

void tokenStatusCallback(TokenInfo info)
{
    if (info.status == esp_signer_token_status_error)
    {
        Serial.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        Serial.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    }
    else
    {
        Serial.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}