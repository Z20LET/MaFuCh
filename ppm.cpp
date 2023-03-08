#pragma once

#include <MQ135.h>

MQ135 mq135(39);

String readPPM() {
  ppm = mq135.getPPM();
  if (isnan(ppm)) {
    Serial.println("Failed to read from MQ135 sensor!");
    return "--";
  }
  else {
    Serial.println(ppm*10);
    return String(ppm*10);
  }
}
