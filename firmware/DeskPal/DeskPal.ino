// DeskPal application by Zhe Chen, originally developed in 2024.
// Public packaging: 2026. See LICENSE and NOTICE for attribution.
#include "config.h"

/* Includes ---------------------------------------------------------------- */
#include <PoseDetection_inferencing.h>
#include "LIS3DHTR.h"
#include "TFT_eSPI.h"
#include "rpcWiFi.h"
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>  
#include <SensirionI2cSht4x.h>
#include <Wire.h>
#include <WiFi.h>         // Used for Wi-Fi connection
#include <WiFiUdp.h>      // Used for UDP support
#include <NTPClient.h>    // Used for NTP client


SensirionI2cSht4x sht4x;
LIS3DHTR<TwoWire> lis;
TFT_eSPI tft;
WiFiClient wioClient;
PubSubClient client(wioClient);

//receie current time
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 8 * 3600, 3600000); // Set the time zone to UTC+8 (Shanghai Time, China)

/* Constant defines -------------------------------------------------------- */
#define CONVERT_G_TO_MS2    9.80665f
#define MAX_ACCEPTED_RANGE  2.0f        

#define BUTTON_1 WIO_KEY_A // The rightmost button starts the system
#define BUTTON_2 WIO_KEY_B // Middle button for testing
#define BUTTON_3 WIO_KEY_C // Leftmost button for testing



// Update these variables accordingly
const char* ssid = DESKPAL_WIFI_SSID;
const char* password = DESKPAL_WIFI_PASSWORD;
const char* mqtt_server = DESKPAL_MQTT_HOST;
const char *ID = DESKPAL_MQTT_CLIENT_ID;
const char *ENV_topic = "Wio-ENV";
const char *facial_topic = "facial";
const char *notify_topic = "reminders";

//Modified to use a different theme
const char *weather_request_topic = "weather/request";   // The topic of the weather request issued by the device
const char *weather_response_topic = "weather/response"; // Devices subscribe to weather response topics
// Add global variable
String currentCity = "";
String currentWeather = "";

float currentTemperature = 0.0;
float currentHumidity = 0.0;
const unsigned long TEMP_HUMIDITY_INTERVAL = 3000; // Temperature and humidity sampling interval, 3 seconds
unsigned long lastTempHumidityTime = 0; // Last sampling time

const unsigned TIME_INTERVAL = 2000;//Get the current time interval
unsigned long lastTime = 0;

// Facial expression variables
int currentEmotion = 0; // 0 = sleep, 1 = smile, 2 = sad, 3 = dizzy
unsigned long lastInputTime = 0;
const unsigned long TIMEOUT = 10000;  // 10 seconds timeout
bool classificationEnabled = false; // Whether to enable classification
bool systemStarted = false; //Whether the system has started
bool weatherReceived = false; // Whether weather information has been received

bool isSunnyDay = false;    // Flag whether it is a sunny day
const int LIGHT_THRESHOLD = 500;  // Light intensity threshold
//const unsigned long SUNBATH_TIME = 60000;  // The sun progress bar time is 1 minute
const unsigned long SUNBATH_TIME = 30000;
unsigned long sunbathStartTime = 0;  // Start time of sun exposure
bool sunbathing = false;    // Mark whether you are basking in the sun
bool needSunReminder = false;

// Affection variables
int affectionLevel = 0;  // Current affection level
const int MAX_AFFECTION = 10; // Max affection level
bool maxAffectionReached = false;

// Idle and standing tracking variables
int idleCount = 0;
const int IDLE_THRESHOLD = 20;  // The threshold is adjusted to 20 to indicate that the idle state needs to be detected 20 times

// Program state variables
int programState = 0; // 0=normal, 1=need to stand up
bool affectionDecreased = false; // Used to mark whether favorability has decreased

// Wave detection variables
int waveCount = 0; // 'wave' The state counter
unsigned long lastWaveTime = 0; // The last time a 'wave' was detected
const unsigned long WAVE_TIME_WINDOW = 5000; // Time window (ms) to detect consecutive 'waves'

// Expression timing variables
unsigned long expressionStartTime = 0; // The time when a particular expression begins
const unsigned long EXPRESSION_DURATION = 2000; // Expression Duration (ms)
bool isSpecialExpression = false; // Whether a special expression is being displayed

bool hasDisplayedFunnyMessage = false; // The sign "That's funny!" Is it already displayed?

bool hasDisplayedBedtimeReminder = false; 
bool isSleeping = false;

// Weather request variables
bool waitingForWeatherResponse = false; // Whether you are waiting for a weather response
String receivedWeatherInfo = ""; // Stores received weather information

// Modify the progress bar variable and adjust the position to the side vertical display
const int PROGRESS_BAR_X = 300;   // Progress bar top-left x-coordinate (right side of the screen)
const int PROGRESS_BAR_Y = 20;    // The y-coordinate of the upper-left corner of the progress bar
const int PROGRESS_BAR_WIDTH = 10;  // Progress bar width
const int PROGRESS_BAR_HEIGHT = 200;  // Progress bar height


/* Arduino setup function */
void setup() {

    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_WHITE);
    
    // Initialize the SHT4x sensor
    sht4x.begin(Wire, SHT40_I2C_ADDR_44);

        // Initialize the NTP client
    timeClient.begin();

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqttCallback); // Set up the MQTT callback function


    // Initialize the I2C communication
    Wire.begin();

    lis.begin(Wire1);
    if (!lis.available()) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    }
    else {
        Serial.println("IMU initialized");
    }
    lis.setOutputDataRate(LIS3DHTR_DATARATE_100HZ);
    lis.setFullScaleRange(LIS3DHTR_RANGE_16G);
  
    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 3) {
        ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to 3 (the 3 sensor axes)\n");
        return;
    }

    pinMode(BUTTON_1, INPUT_PULLUP); // Initialize BUTTON_1
    pinMode(BUTTON_2, INPUT_PULLUP); // Initialize BUTTON_2
    pinMode(BUTTON_3, INPUT_PULLUP); // Initialize BUTTON_3
       
    tft.fillRect(0, 0, 320, 200, TFT_WHITE);

    drawSleepingFace(); // Display the initial sleep expression

    Serial.begin(115200);
    //while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");

}

/**
 * @brief Return the sign of the number
 * 
 * @param number 
 * @return int 1 if positive (or 0) -1 if negative
 */
float ei_get_sign(float number) {
    return (number >= 0.0) ? 1.0 : -1.0;
}

/**
* @brief      Get data and run inferencing
*
* @param[in]  debug  Get debug info if true
*/


void loop() {

    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    
    unsigned long currentMillis = millis();

    // Temperature and humidity sampling timer
    if (currentMillis - lastTempHumidityTime >= TEMP_HUMIDITY_INTERVAL) {
        lastTempHumidityTime = currentMillis;
        readAndDisplayTemperatureHumidity(); // Read and display temperature and humidity
    }

    // Gets and displays the time
    if (currentMillis - lastTime >= TIME_INTERVAL) { 
        lastTime = currentMillis;   
        timeClient.update();
        String formattedTime = timeClient.getFormattedTime();
        displayTime(formattedTime); // Display the time on the screen
        checkTimeForBed(); // Check to see if it's time to sleep
    }

    // Check if the button is pressed to start the system
    if (!systemStarted && digitalRead(BUTTON_1) == LOW) {
        delay(200); // Debounce
        classificationEnabled = true;
        systemStarted = true; // System started
        askForWeather(); 
    }

        // If the system does not start, wait for the button to be pressed
    if (!systemStarted) {
        return;
    }

        // If the weather information has not been received, skip the other processing
    if (!weatherReceived) {
        return;
    }

    if (needSunReminder && !sunbathing){
      displaySunReminder(); 
    }
    if (sunbathing){
       classificationEnabled = false;
    }else {
          // Now  can start sorting and interacting
        classificationEnabled = true;
    }

    if (isSunnyDay) {
      float light = analogRead(WIO_LIGHT);  // Reading light values
      //classificationEnabled = false;
      if (light > LIGHT_THRESHOLD) {
          if (!sunbathing) {
              sunbathing = true;
              sunbathStartTime = millis();  // 开始计时
              drawCozyFace();  // Display the "cozy" emoji
              drawSunbathProgressBar(0);  // Initialize the progress bar
          } else {
              // Schedule of calculation
              unsigned long elapsed = millis() - sunbathStartTime;
              if (elapsed >= SUNBATH_TIME) {
                  increaseAffectionLevel(); // Increase the favorability
                  sunbathing = false;   // Reset the sunbathing flag
                  needSunReminder = false;
                  clearProgressBar();  // Clear the progress bar
                  drawSmilingFace();  // back to the smiley face
                  tft.setCursor(220, 60);
                  tft.setTextColor(TFT_BLACK);
                  tft.setTextSize(1);
                  tft.println("good job!");
                  delay(1500);
              } else {
                  int progress = (elapsed * 100) / SUNBATH_TIME;  // Calculate the progress
                  drawSunbathProgressBar(progress);  
              }
          }
      } else if (sunbathing) {
          // If there is not enough light, cut off the sunIf there is not enough light, cut off the sun bathing
          sunbathing = false;
          clearProgressBar();  
          drawSmilingFace();  
      }
    }

        // Check if BUTTON_2 is pressed to increase favorability at test time
    if (digitalRead(BUTTON_2) == LOW) {
        delay(100); // Debounce
        increaseAffectionLevel();
    }

    // The duration of facial expressions was processed
    if (isSpecialExpression && millis() - expressionStartTime >= EXPRESSION_DURATION) {
        isSpecialExpression = false;
        drawSmilingFace(); // Return the smile expression
    }

    // If a special expression is being displayed, skip subsequent processing
    if (isSpecialExpression) {
        return;
    }

    // If classification is not enabled, skip the rest
    if (!classificationEnabled) {
        return;
    }

    Serial.println("\nStarting inferencing in 2 seconds...");
    delay(2000);
    Serial.println("Sampling...");

    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };
    uint32_t sample_time_ms = 3500;
    uint32_t start_time = millis();
    size_t ix = 0;

    while (millis() - start_time < sample_time_ms && ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
        lis.getAcceleration(&buffer[ix], &buffer[ix + 1], &buffer[ix + 2]);

        for (int i = 0; i < 3; i++) {
            if (fabs(buffer[ix + i]) > MAX_ACCEPTED_RANGE) {
                buffer[ix + i] = ei_get_sign(buffer[ix + i]) * MAX_ACCEPTED_RANGE;
            }
        }

        buffer[ix + 0] *= CONVERT_G_TO_MS2;
        buffer[ix + 1] *= CONVERT_G_TO_MS2;
        buffer[ix + 2] *= CONVERT_G_TO_MS2;

        ix += 3;
        delayMicroseconds(next_tick - micros());
    }

    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        Serial.printf("Failed to create signal from buffer (%d)\n", err);
        return;
    }

    ei_impulse_result_t result = { 0 };
    unsigned long classification_start = millis();
    err = run_classifier(&signal, &result, false);
    unsigned long classification_end = millis();
    if (err != EI_IMPULSE_OK) {
        Serial.printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    Serial.printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.):\n",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        Serial.printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }

    drawClassificationResult(result);  // The classification results were displayed on the LCD

    // Wave state handling
    if (result.classification[2].value > 0.8) { 
        // Idle count Still increase
        idleCount++;

        unsigned long currentTime = millis();
        if (currentTime - lastWaveTime <= WAVE_TIME_WINDOW) {
            waveCount++;
        } else {
            waveCount = 1; // Reset the counter
        }
        lastWaveTime = currentTime;

        // When waveCount is 2 and "That's funny!" When not shown
        if (waveCount == 2 && !hasDisplayedFunnyMessage) {
            tft.setCursor(220, 120);
            tft.setTextColor(TFT_BLACK);
            tft.setTextSize(1);
            tft.println("That's funny!");
            hasDisplayedFunnyMessage = true; // Tag "That's funny!" It has been shown
            increaseAffectionLevel();
            expressionStartTime = millis(); // The time to start the display was recorded
        }

        // The dizzy expression was displayed when waveCount reached 3
        if (waveCount == 3) {
            // Reset "That's funny!" Sign 
            hasDisplayedFunnyMessage = false;

            // dizzy state
            if (affectionLevel > 0) {
                affectionLevel--;
                drawAffectionLevel();
            }
            drawDizzyFace(); // Display a dizzy expression
            isSpecialExpression = true; // The marker is displaying a special expression
            expressionStartTime = millis(); // The time of the onset of the Special expression was recorded
            waveCount = 0; // Reset the counter
        }
    }

    // 处理特殊表情的显示时长（包括 "That's funny!" 和眩晕表情）
    if (isSpecialExpression && millis() - expressionStartTime >= EXPRESSION_DURATION) {
        isSpecialExpression = false;
        drawSmilingFace(); // 返回到微笑表情
    }

    // Idle and rise state handling
    if (result.classification[1].value > 0.8) { // The rise state has been detected
        if (programState == 1) {
            // Favorability increases only in the red exclamation point state
            programState = 0; // Return to normal
            drawSmilingFace();

            increaseAffectionLevel(); // Favorability increased
            drawAffectionLevel();
            idleCount = 0; // Reset idle count
            affectionDecreased = false; // Reset favorability decrease flag
            clearProgressBar(); // Clear the progress bar
        } else {
            // Do not enter the red exclamation point state, only reset idle count and progress bar, do not increase liking
            idleCount = 0;
            clearProgressBar();
            drawSmilingFace(); // Return to the normal state with a smile expression
        }
    } else {
        // No rise state was detected and idleCount was continued to accumulate
        idleCount++;

        if (idleCount >= IDLE_THRESHOLD) {
            if (programState != 1) {
                programState = 1; // Get to the point where need to stand up
                drawSleepingFaceWithExclamation(); // Displays a sleep expression with an exclamation point
                Serial.println("You need to stand up!");
                client.publish(notify_topic, "You need to stand up!");
                affectionDecreased = false; // Reset the favorability decrease flag
            } else if (!affectionDecreased) {
                drawSadFace();  
                client.publish(notify_topic, "Don't be lazy! You need to stand up!");
                affectionLevel = max(0, affectionLevel - 1);  // Favorability decreased by 1
                drawAffectionLevel();
                affectionDecreased = true; // Prevent further reduction
            }
        }
        updateProgressBar(); // Update progress bar
    }

    // Add timeout to avoid getting stuck in loop if inferencing takes too long
    if ((classification_end - classification_start) > 5000) {
        Serial.println("Classification took too long, skipping...");
        return;
    }
}

void increaseAffectionLevel() {
    if (!maxAffectionReached) {
        affectionLevel++;
        if (affectionLevel >= MAX_AFFECTION) {
            affectionLevel = MAX_AFFECTION; // Make sure don't exceed MAX_AFFECTION
            maxAffectionReached = true;     // Set the flag

            // Display special messages
            displayMaxAffectionMessage();

            // Return the smile expression
            drawSmilingFace();
        }
        drawAffectionLevel(); // Update the displayed favorability
    }
}

void displayMaxAffectionMessage() {
    // Clear the screen
    tft.fillScreen(TFT_WHITE);

    // Set text properties
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);

    // Defines the message to display
    String message1 = "You've completed all";
    String message2 = "achievements today!";
    String message3 = "You must have had";
    String message4 = "a good day!";

    int y = 60;

    tft.setCursor((320 - tft.textWidth(message1)) / 2, y);
    tft.println(message1);
    y += 30; // Adjust the y position to show the next row

    tft.setCursor((320 - tft.textWidth(message2)) / 2, y);
    tft.println(message2);
    y += 30;

    tft.setCursor((320 - tft.textWidth(message3)) / 2, y);
    tft.println(message3);
    y += 30;

    tft.setCursor((320 - tft.textWidth(message4)) / 2, y);
    tft.println(message4);

    delay(3000); // The message is displayed for 3 seconds

    // After displaying the message, return the smile emoji
    drawSmilingFace();

    // Make sure favorability stays on display
    drawAffectionLevel();
}

// A function of asking about the weather
void askForWeather() {
    tft.fillRect(0, 200, 320, 40, TFT_WHITE);  // Clear the area
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 200);
    tft.print("What is the weather now?");
    
    client.publish(weather_request_topic, "request_weather");  // Post a request for weather
    
    waitingForWeatherResponse = true; // Set the wait response flag
    weatherReceived = false; // Reset weather received flag
}


// WiFi Set up function
void setup_wifi() {
    delay(10); // Small delay to prevent errors

    tft.setTextSize(2);
    tft.setCursor((320 - tft.textWidth("Connecting to Wi-Fi..")) / 2, 120);
    tft.print("Connecting to Wi-Fi..");

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password); // Connect to Wi-Fi

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");

    tft.fillScreen(TFT_BLACK);
    tft.setCursor((320 - tft.textWidth("Connected!")) / 2, 120);
    tft.print("Connected!");

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); // Displays the local IP address
}

// MQTT callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String incomingMessage = "";
    for (unsigned int i = 0; i < length; i++) {
        incomingMessage += (char)payload[i];
    }
    Serial.print("Message received on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    Serial.println(incomingMessage);

    //Only messages from weather_response_topic are processed 
    if (String(topic) == weather_response_topic && waitingForWeatherResponse) {
        // The received JSON message is parsed using ArduinoJson
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, incomingMessage);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.f_str());
            return;
        }

        String city = doc["city"];
        String weather = doc["weather"];
        currentCity = city;       // Save to a global variable
        currentWeather = weather; 


        Serial.print("Parsed city: ");
        Serial.println(city);
        Serial.print("Parsed weather: ");
        Serial.println(weather);

        displayWeatherInfo(city, weather);
        waitingForWeatherResponse = false; // Reset the wait flag
        weatherReceived = true; // The flag has received weather information
        // Reset the timeout timer
        drawSmilingFace(); // Show a smile expression
        
        if (weather == "sunny") {
        isSunnyDay = true;  // Set up sunny signs
        client.publish(notify_topic, "Let's go sunbathing!");  // Send a reminder
        needSunReminder = true; // Display reminders on the screen
    } else {
        needSunReminder = false;
        isSunnyDay = false;  // Cancel the sunny day sign
    }  
    }
}

// MQTT Reconnection function
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(ID)) {
            Serial.println("connected");
            client.publish(notify_topic, "hello from Wio Terminal");
            // Subscribe to the Weather Response topic
            client.subscribe(weather_response_topic); 
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void publishFacialExpression(const char* expression) {
    client.publish(facial_topic, expression);
}

// Expression drawing function

// Draw the "cozy" emoji
void drawCozyFace() {
    tft.fillRect(100, 60, 120, 120, TFT_WHITE);
    tft.fillCircle(160, 120, 60, TFT_YELLOW);  // Yellow face
    tft.fillCircle(140, 100, 10, TFT_BLACK);  // Left eye
    tft.fillCircle(180, 100, 10, TFT_BLACK);  // Right eye
    tft.fillRect(130, 90, 20, 10, TFT_BLACK); // Sunglasses left lens
    tft.fillRect(170, 90, 20, 10, TFT_BLACK); // Sunglasses right lens
    tft.drawLine(150, 95, 170, 95, TFT_BLACK); // Sunglasses connect the wire
    drawSmile(160, 140, 20, TFT_BLACK);  // Smiling mouth
     publishFacialExpression("cozy");  // Post sleep emojis
    drawAffectionLevel();
    tft.fillRect(220, 60, 70, 50, TFT_WHITE);
    tft.setCursor(220, 60);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(1);
    tft.println("niceeee!");

}
void drawSleepingFace() {
    tft.fillRect(100, 60, 120, 120, TFT_WHITE);
    tft.fillCircle(160, 120, 60, TFT_YELLOW);  // Yellow face
    drawClosedEye(140, 100, 20, TFT_BLACK);    // Left eye
    drawClosedEye(180, 100, 20, TFT_BLACK);    // Right eye
    tft.drawLine(140, 140, 180, 140, TFT_BLACK);  // Straight mouth
    drawZLetter(220, 60, 3, TFT_BLACK);  // The Z letter
    publishFacialExpression("sleep");  // Post sleep emojis
    drawAffectionLevel();
    // No progress bar is drawn
    displayWeatherInfo(currentCity, currentWeather);
}

void drawSleepingFaceWithExclamation() {
    tft.fillRect(100, 60, 120, 120, TFT_WHITE);
    tft.fillRect(220, 60, 70, 50, TFT_WHITE);
    tft.fillCircle(160, 120, 60, TFT_YELLOW);  // Yellow face
    drawClosedEye(140, 100, 20, TFT_BLACK);    // Left eye
    drawClosedEye(180, 100, 20, TFT_BLACK);    //  Right eye
    tft.drawLine(140, 140, 180, 140, TFT_BLACK);  // Straight mouth
    drawExclamationMark(220, 60, 3, TFT_RED);  // Red exclamation mark
    publishFacialExpression("need_to_stand_up");  // Post emojis
    drawAffectionLevel();
    // The progress bar continues to display when you enter a state where you need to stand
    displayWeatherInfo(currentCity, currentWeather);
}

void drawSmilingFace() {
    tft.fillRect(0, 0, 320, 200, TFT_WHITE);
    tft.fillCircle(160, 120, 60, TFT_YELLOW);
    tft.fillCircle(140, 100, 10, TFT_BLACK);  // Left eye
    tft.fillCircle(180, 100, 10, TFT_BLACK);  // Right eye
    drawSmile(160, 140, 20, TFT_BLACK);  // Smiling mouth
    publishFacialExpression("smile");  // Post a smile emoji
    drawAffectionLevel();
    clearProgressBar(); // Clear the progress bar
    displayWeatherInfo(currentCity, currentWeather);
}

void drawDizzyFace() {
  tft.fillRect(220, 120, 80, 50, TFT_WHITE); // Remove "that's funny messages"
    tft.fillRect(100, 60, 120, 120, TFT_WHITE);
    tft.fillCircle(160, 120, 60, TFT_YELLOW);
    drawCrossEye(140, 100, 10, TFT_BLACK);  // Left eye
    drawCrossEye(180, 100, 10, TFT_BLACK);  // Right eye
    drawWavyMouth(120, 140, 80, 10, TFT_BLACK);  // Mouth of wave
    publishFacialExpression("dizzy");  // Post the dizzy emoji
    drawAffectionLevel();
    displayWeatherInfo(currentCity, currentWeather);
}

void drawSadFace() {
    tft.fillRect(220, 60, 70, 50, TFT_WHITE);
    tft.fillRect(100, 60, 120, 120, TFT_WHITE);
    tft.fillCircle(160, 120, 60, TFT_YELLOW);
    drawClosedEye(140, 100, 20, TFT_BLACK);  // Left eye
    drawClosedEye(180, 100, 20, TFT_BLACK);  // Right eye
    tft.drawLine(140, 160, 180, 160, TFT_BLACK);  // Sad mouth
    publishFacialExpression("sad");  // Post sad emojis
    drawAffectionLevel();
    displayWeatherInfo(currentCity, currentWeather);
}

void drawAffectionLevel() {
    tft.fillRect(220, 0, 80, 40, TFT_WHITE);  // Clear previous favorability display
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(110, 10);
    tft.print("Affection: ");
    tft.print(affectionLevel);
    tft.print("/10 ");
    drawHeart(105, 10, 5, TFT_RED);
}

void drawClassificationResult(ei_impulse_result_t result) {
    tft.fillRect(0, 200, 320, 40, TFT_WHITE);  // Clear the previous classification results
    tft.setTextColor(TFT_BLUE);
    tft.setTextSize(2);
    tft.setCursor(10, 200);
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        tft.print(result.classification[ix].label);
        tft.print(": ");
        tft.print(result.classification[ix].value, 3);
        tft.print("  ");
    }
}

void displayWeatherInfo(String city, String weather) {
    Serial.print("Displaying weather info: ");
    Serial.print("City: ");
    Serial.print(city);
    Serial.print(", Weather: ");
    Serial.println(weather);

    // Defines the location of the display area
    int x = 10; // 10 pixels from the left edge
    int y = 100; // Middle of the screen

    // Clear the previous weather information display area
    tft.fillRect(0, y, 80, 40, TFT_WHITE);  // Clear the area

    tft.setTextColor(TFT_BLUE);
    tft.setTextSize(1); // The font size is set to 1

    tft.setCursor(x, y);  // Sets the cursor to the specified location
    tft.print("City: ");
    tft.println(city);    // Line break display

    tft.print("Weather: ");
    tft.println(weather);
}

// Display "Let's go sunbathing! Reminders
void displaySunReminder() {
    /*tft.fillRect(0, 200, 320, 40, TFT_WHITE);
    tft.setTextColor(TFT_BLUE);
    tft.setTextSize(2);
    tft.setCursor((320 - tft.textWidth("Let's go sunbathing!")) / 2, 200);
    tft.println("Let's go sunbathing!");*/
    tft.fillRect(220, 60, 70, 50, TFT_WHITE);
    tft.setCursor(220, 60);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(1);
    tft.println("Let's go\n");
    tft.setCursor(220, 70);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(1);
    tft.println("sunbathing!");

}


// Draw the sun progress bar
void drawSunbathProgressBar(int progress) {
    const int PROGRESS_BAR_X = 60;
    const int PROGRESS_BAR_Y = 180;
    const int PROGRESS_BAR_WIDTH = 200;
    const int PROGRESS_BAR_HEIGHT = 20;

    // Draw a border
    tft.drawRect(PROGRESS_BAR_X, PROGRESS_BAR_Y, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, TFT_BLACK);

    // Fill the progress bar
    int filledWidth = (progress * PROGRESS_BAR_WIDTH) / 100;
    tft.fillRect(PROGRESS_BAR_X + 1, PROGRESS_BAR_Y + 1, filledWidth, PROGRESS_BAR_HEIGHT - 2, TFT_GREEN);

    // Unfilled sections were removed
    tft.fillRect(PROGRESS_BAR_X + 1 + filledWidth, PROGRESS_BAR_Y + 1, PROGRESS_BAR_WIDTH - filledWidth - 2, PROGRESS_BAR_HEIGHT - 2, TFT_WHITE);
}


void readAndDisplayTemperatureHumidity() {
    // Temperature and humidity data were read
    float temperature;
    float humidity;
    uint16_t errorCode = sht4x.measureHighPrecision(temperature, humidity);
    if (errorCode) {
        Serial.print("Error trying to execute measureHighPrecision(): ");
        Serial.println(errorCode);
    } else {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C, Humidity: ");
        Serial.print(humidity);
        Serial.println(" %RH");

        // Save to a global variable for display time
        currentTemperature = temperature;
        currentHumidity = humidity;

        // If the temperature exceeds 30 degrees, display a warning and send an MQTT alert
        if (temperature > 30.0) {
            drawHotWarning(); // Draw "It's too hot! Warnings
            client.publish(ENV_topic, "Warning: Temperature above 30°C, too hot!");  // Send MQTT alerts
        }

        // Display temperature and humidity data
        displayTemperatureHumidity();

        // Send an MQTT message
        char msg[50];
        snprintf(msg, 50, "Temperature: %.2f C, Humidity: %.2f %%", temperature, humidity);
        client.publish(ENV_topic, msg);
    }
}

void drawHotWarning() {
    // Clear the area and display "It's too hot! Warnings
    tft.fillRect(0, 200, 320, 40, TFT_WHITE);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor((320 - tft.textWidth("It's too hot!")) / 2, 200);
    tft.println("It's too hot!");
    publishFacialExpression("hot_warning");  //Post a warning emoji
}

// Display real time
void displayTime(String time) {
    // Erases the previously displayed time region
    tft.fillRect(0, 25, 100, 20, TFT_WHITE);  // Erase region, width 320 height 40, starting at Y=10

    // Set the font size and color
    tft.setTextSize(1);   // Set a large font
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(10, 25);  // Set cursor position (top left corner)

    tft.print("Time: ");
    tft.println(time);  // Print formatting time
}

void checkTimeForBed() {
    // Check if it is 11 p.m. each time you update the time
    String currentTime = timeClient.getFormattedTime();
    int currentHour = currentTime.substring(0, 2).toInt();
    int currentMinute = currentTime.substring(3, 5).toInt();
    // Serial.print(currentHour);
    

    if (currentHour >= 23||currentHour <=20) {  // Determine if it is 11 o 'clock or later
      if (!isSleeping){
        // If a reminder has not already been displayed, it will display "Time to sleep!" Reminders
            drawBedtimeReminder();
            client.publish(notify_topic, "It's past 16 PM, time to go to bed!");
          } 

            // Wait for user confirmation
        if (digitalRead(BUTTON_3 ) == LOW) {  // Wait for the user to press the button
            delay(100);  // Debounce
            // When the user presses the button, the system is turned off
          systemStarted = false;  // Stop classifying
          tft.fillScreen(TFT_WHITE);
          drawSleepingFace();  // Show a sleep expression
          client.publish(notify_topic, "System shut down, going to sleep.");  // Send a system shutdown message
          isSleeping = true;
        }

    } 
}

void drawBedtimeReminder() {
    // Clear the area and display a sleep reminder
    tft.setTextColor(TFT_RED);
    tft.setTextSize(1);
    tft.setCursor(110, 30);
    //tft.setCursor((320 - tft.textWidth("It's time to go to bed!")) / 2, 200);
    tft.println("It's time to go to bed!");
    publishFacialExpression("bedtime_reminder");  // Post a sleep reminder emoji
}


// Displays a function of temperature and humidity data
void displayTemperatureHumidity() {
    // Defines the location of the display area
    int x = 10; 
    int y = 120; 

    // 清除之前的温湿度显示区域
    tft.fillRect(0, y, 85, 30, TFT_WHITE);  // Clear the area

    tft.setTextColor(TFT_BLUE);
    tft.setTextSize(1); // The font size is set to 1

    tft.setCursor(x, y);  // Sets the cursor to the specified location
    tft.print("Temp: ");
    tft.print(currentTemperature, 1); // Displays one decimal
    tft.print(" C");
    tft.println();

    tft.print("Humidity: ");
    tft.print(currentHumidity, 1); // Displays one decimal
    tft.print(" %");
}

/* Draw the progress bar function */
void drawProgressBar() {
    // Draws the progress bar border
    tft.drawRect(PROGRESS_BAR_X, PROGRESS_BAR_Y, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, TFT_BLACK);
    // Calculate the height of the fill section, ensuring that it does not exceed the maximum height of the progress bar
    int filledHeight = (idleCount * PROGRESS_BAR_HEIGHT) / IDLE_THRESHOLD;
    if (filledHeight > PROGRESS_BAR_HEIGHT) {
        filledHeight = PROGRESS_BAR_HEIGHT;  // Ensure that the maximum height is not exceeded
    }
    // The filling part is increased from the bottom to the top
    int yStart = PROGRESS_BAR_Y + PROGRESS_BAR_HEIGHT - filledHeight;
    tft.fillRect(PROGRESS_BAR_X + 1, yStart + 1, PROGRESS_BAR_WIDTH - 2, filledHeight - 2, TFT_GREEN);
    // Unfilled sections were removed
    tft.fillRect(PROGRESS_BAR_X + 1, PROGRESS_BAR_Y + 1, PROGRESS_BAR_WIDTH - 2, PROGRESS_BAR_HEIGHT - filledHeight - 2, TFT_WHITE);
}

/* Clear the progress bar function */
void clearProgressBar() {
    tft.fillRect(PROGRESS_BAR_X, PROGRESS_BAR_Y, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, TFT_WHITE);
}

void updateProgressBar() {
    drawProgressBar();
}

// Auxiliary function, used to draw facial expression features
void drawCrossEye(int x, int y, int size, uint16_t color) {
    tft.drawLine(x - size, y - size, x + size, y + size, color);
    tft.drawLine(x - size, y + size, x + size, y - size, color);
}

void drawWavyMouth(int x, int y, int width, int amplitude, uint16_t color) {
    int halfWidth = width / 2;
    for (int i = -halfWidth; i <= halfWidth; i++) {
        int yOffset = amplitude * sin((float)i / halfWidth * PI);
        tft.drawPixel(x + i + halfWidth, y + yOffset, color);
    }
}

void drawClosedEye(int x, int y, int r, uint16_t color) {
    for (int i = 0; i <= 180; i += 5) {
        int x1 = x + r * cos(radians(i));
        int y1 = y + r * sin(radians(i));
        int x2 = x + r * cos(radians(i + 5));
        int y2 = y + r * sin(radians(i + 5));
        tft.drawLine(x1, y1, x2, y2, color);
    }
}

void drawZLetter(int x, int y, int scale, uint16_t color) {
    tft.drawLine(x, y, x + 10 * scale, y, color);
    tft.drawLine(x + 10 * scale, y, x, y + 10 * scale, color);
    tft.drawLine(x, y + 10 * scale, x + 10 * scale, y + 10 * scale, color);
}

void drawExclamationMark(int x, int y, int scale, uint16_t color) {
    // Draw the exclamation mark
    tft.fillRect(x, y, scale * 2, scale * 6, color); // Vertical line
    tft.fillRect(x, y + scale * 7, scale * 2, scale * 2, color); // point
}

void drawHeart(int x0, int y0, int size, uint16_t color) {
    tft.fillCircle(x0 - size / 2, y0, size / 2, color);
    tft.fillCircle(x0 + size / 2, y0, size / 2, color);
    tft.fillTriangle(x0 - size, y0, x0 + size, y0, x0, y0 + size, color);
}

void drawSmile(int x, int y, int r, uint16_t color) {
    for (int i = 0; i <= 180; i += 5) {
        int x1 = x + r * cos(radians(i));
        int y1 = y + r * sin(radians(i));
        int x2 = x + r * cos(radians(i + 5));
        int y2 = y + r * sin(radians(i + 5));
        tft.drawLine(x1, y1, x2, y2, color);
    }
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_ACCELEROMETER
#error "Invalid model for current sensor"
#endif
