#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "payload/magneto.h"
#include "payload/termo.h"
#include "payload/accel_gyro.h"
#include "payload/baro.h"
#include "payload/gps.h"
#include "payload/tasks.h"
#include "payload/config.h"

#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <TinyGPS++.h>

#define GPS_SERIAL Serial2

Adafruit_BMP085 bmp;

TinyGPSPlus gps;


