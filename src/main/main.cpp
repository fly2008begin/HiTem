#include <M5Cardputer.h>
#include <SPI.h>
#include <SD.h>
#include "ChatApp.h"

#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

ChatApp app;

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    Serial.begin(115200);

    // SD card init (same as official M5Cardputer sdcard example)
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
        Serial.println("SD: init failed or no card");
    } else {
        uint8_t ct = SD.cardType();
        Serial.printf("SD: type=%d size=%lluMB\n", ct, SD.cardSize()/(1024*1024));
    }

    app.begin();
}

void loop() {
    app.update();
    delay(10);
}
