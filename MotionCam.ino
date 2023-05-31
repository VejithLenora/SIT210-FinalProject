#include <WiFi.h>
#include <Wire.h>
#include <FS.h>
#include "esp_camera.h"
#include "SD_MMC.h"
#include "Arduino.h"
#include "soc/soc.h" 
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <EEPROM.h>  

const char* ssid = "MiddleEarth";
const char* password = "vejith666";

void setup() {
  pinMode(13, INPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5; 
  config.pin_d1 = 18; 
  config.pin_d2 = 19; 
  config.pin_d3 = 21; 
  config.pin_d4 = 36;
  config.pin_d5 = 39; 
  config.pin_d6 = 34; 
  config.pin_d7 = 35; 
  config.pin_xclk = 0; 
  config.pin_pclk = 22; 
  config.pin_vsync = 25; 
  config.pin_href = 23; 
  config.pin_sscb_sda = 26; 
  config.pin_sscb_scl = 27; 
  config.pin_pwdn = 32; 
  config.pin_reset = -1; 
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Init SD card
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  Serial.println("SD Card Initialized");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (digitalRead(13) == HIGH) {
      capturePhoto();
      delay(5000); 
    }
    else
    {
      Serial.println("Motion Not Detected");
      }
  } else {
    Serial.println("WiFi Disconnected");
    delay(1000);
  }
}

void capturePhoto() {
  String filename = "/sdcard/capture_" + String(millis()) + ".jpg";

  fs::FS &fs = SD_MMC;

  File file = fs.open(filename.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in write mode");
    return;
  }

  camera_fb_t *fb = NULL;
  bool ok = false;

  try {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // Write the photo data to the SD card
    if (file.write(fb->buf, fb->len)) {
      Serial.println("Capture saved to file: " + filename);
      ok = true;
    } else {
      Serial.println("Failed to write photo to file");
    }
  } catch (...) {
    Serial.println("Exception occurred during photo capture");
  }

  file.close();
  esp_camera_fb_return(fb);

  if (!ok) {
    fs.remove(filename.c_str());
    Serial.println("Capture removed");
  }
}
