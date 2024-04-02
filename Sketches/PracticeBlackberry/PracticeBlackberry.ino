#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <SD.h>
#include "es7210.h"
#include <Audio.h>
#include <driver/i2s.h>

#define TOUCH_MODULES_GT911
#include "TouchLib.h"
#include "utilities.h"

#ifndef RADIO_FREQ
#define RADIO_FREQ                  868.0
#endif

#define MIC_I2S_SAMPLE_RATE         16000
#define MIC_I2S_PORT                I2S_NUM_1
#define SPK_I2S_PORT                I2S_NUM_0
#define VAD_SAMPLE_RATE_HZ          16000
#define VAD_FRAME_LENGTH_MS         30
#define VAD_BUFFER_LENGTH           (VAD_FRAME_LENGTH_MS * VAD_SAMPLE_RATE_HZ / 1000)

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
