#include <Arduino.h>
#include "RelayApp.h"

RelayApp app;

void setup() {
    Serial.begin(115200);
    app.begin();
}

void loop() {
    app.update();
}
