/*
   The Code is Written By Vishal Soni (Youtube: ElectroDonut)
   esp32 Board Version: 3.3.0
   TFT_espi Version : 2.5.43
*/

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include "images/soul_m.h"
#include "images/soul_f.h"
#include "images/intro.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();  // Create an instance of TFT_eSPI
TFT_eSprite gif_sprite = TFT_eSprite(&tft);  // Sprite object for displaying frames

#include <Preferences.h>
Preferences preferences;

void saveValue(const char* key, uint8_t value);
uint8_t readValue(const char* key, uint8_t defaultValue = 0);

#define SOUL_CONFIG "sf"
#define SOUL_CHANGE_FLAG "cf"
#define error_nvs 0
#define true_nvs  1
#define false_nvs 2

uint8_t frame_delay = 100;    //frame delay in milliseconds
uint8_t flag = error_nvs;
uint8_t change_soul = error_nvs;

#define SOUL_CHANGE_TIMEOUT_SEC 5
TimerHandle_t xTimer;

// Timer callback function
void timerCallback(TimerHandle_t xTimer) {
  Serial.println("Soul Change Timer Gone");
  change_soul = false_nvs;
  saveValue(SOUL_CHANGE_FLAG, false_nvs);  // save a value
}

void setup() {
  tft.begin();
  tft.setRotation(0);        // Set the display rotation
  tft.fillScreen(TFT_WHITE); // Clear the screen
  
  Serial.begin(115200);
  Serial.println("### SETUP ###");
  start_soul_change_time();

  uint8_t change_soul = readValue(SOUL_CHANGE_FLAG); // read it back
  uint8_t stored_flag = readValue(SOUL_CONFIG); // read it back
  flag = stored_flag;
  if (change_soul == error_nvs || change_soul == true_nvs) {
    Serial.println("Restarted BEFORE 20 sec | Changing Soul");
    if (stored_flag == error_nvs || stored_flag == false_nvs ) {
      saveValue(SOUL_CONFIG, true_nvs );  // save a value
      flag = true_nvs ;
    }
    if (stored_flag == true_nvs ) {
      saveValue(SOUL_CONFIG, false_nvs );  // save a value
      flag = false_nvs ;
    }
    Serial.printf("Value in NVS = %u\n", stored_flag);
  }
  if(change_soul == false_nvs) {
    Serial.println("Restarted AFTER 20 sec | Starting Previous Soul");
    change_soul = true_nvs;
    saveValue(SOUL_CHANGE_FLAG, true_nvs);  // save a value
  }

  // Initialize the sprite
  gif_sprite.createSprite(240, 240);
  gif_sprite.setSwapBytes(true);  // Ensure byte order matches the format

  //play intro on every wake
  playGif(intro_allArray, intro_allArray_LEN, frame_delay);
}

void loop() {
  //change soul
  if (flag == true_nvs) {
    playGif(soul_m_allArray, soul_m_allArray_LEN, frame_delay);  // Play the GIF
  } else if (flag == false_nvs) {
    playGif(soul_f_allArray, soul_f_allArray_LEN, frame_delay);  // Play the GIF
  }
}

//______________________________ FUNCTIONS _________________________________________

void playGif(const uint16_t* frames[], int numFrames, uint8_t frameDelay) {
  for (int i = 0; i < numFrames; i++) {
    // Clear the sprite (optional, but can prevent artifacts)
    gif_sprite.fillSprite(TFT_BLACK);

    // Push the current frame to the sprite
    gif_sprite.pushImage(0, 0, 240, 240, frames[i]);

    // Push the sprite to the display
    gif_sprite.pushSprite(0, 0);

    // Delay to control the animation speed
    delay(frameDelay);
  }
}

/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

// Function to save a uint8_t value
void saveValue(const char* key, uint8_t value) {
  if (preferences.begin("my-app", false)) {   // false = read/write
    preferences.putUChar(key, value);
    preferences.end();
    Serial.printf("Saved %u to key '%s'\n", value, key);
  } else {
    Serial.println("Failed to open NVS for saving!");
  }
}

// Function to read a uint8_t value
uint8_t readValue(const char* key, uint8_t defaultValue) {
  uint8_t value = defaultValue;
  if (preferences.begin("my-app", true)) {    // true = read-only
    value = preferences.getUChar(key, defaultValue);
    preferences.end();
    Serial.printf("Read %u from key '%s'\n", value, key);
  } else {
    Serial.println("Failed to open NVS for reading!");
  }
  return value;
}

void start_soul_change_time(){
    xTimer = xTimerCreate(
             "SecTimer",           // Timer name
             pdMS_TO_TICKS(SOUL_CHANGE_TIMEOUT_SEC * 2000),  // Period in ticks
             pdFALSE,                    // Auto-reload
             (void*)0,                  // Timer ID
             timerCallback              // Callback function
           );

  // Start the timer
  if (xTimerStart(xTimer, 0) != pdPASS) {
    Serial.println("Timer start failed!");
  }
}
