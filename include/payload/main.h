#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "payload/magneto.h"
#include "payload/termo.h"
#include "payload/accel_gyro.h"
#include "payload/baro.h"
#include "payload/tasks.h"
#include "payload/config.h"

#include <SPI.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

