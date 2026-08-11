/*
 * EdgeWake - Tiered Cascade Guardian System
 * Main Firmware for ESP32-CAM
 * 
 * 5-Tier Cascade Architecture:
 *   Tier 1: Deep Sleep (~115µA)
 *   Tier 2: Hardware Interrupt - Flame/Vibration
 *   Tier 3: Edge AI Audio Verification (ZCR + RMS)
 *   Tier 4: Visual Capture (OV2640 Camera)
 *   Tier 5: Cloud Handoff (WiFi POST)
 * 
 * Team: EdgeWake - SIH 2026
 * Lead: Parth Arsid, VIT-AP University
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_camera.h>
#include <esp_sleep.h>
#include <driver/i2s.h>
#include "config.h"

// ======================= PIN DEFINITIONS =======================
// Tier 2: Hardware Interrupt Sensors
#define PIN_FLAME_SENSOR    GPIO_NUM_13  // KY-026 Digital Output
#define PIN_VIBRATION       GPIO_NUM_14  // SW-420 Digital Output

// Tier 3: INMP441 I2S Microphone
#define PIN_I2S_WS          15  // Word Select (LRCLK)
#define PIN_I2S_SCK         2   // Serial Clock (BCLK)
#define PIN_I2S_SD          12  // Serial Data (DOUT)

// Power Management
#define PIN_MIC_POWER       3   // MOSFET gate for mic power

// Camera (ESP32-CAM AI Thinker pin mapping)
#define PWDN_GPIO_NUM       32
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       0
#define SIOD_GPIO_NUM       26
#define SIOC_GPIO_NUM       27
#define Y9_GPIO_NUM         35
#define Y8_GPIO_NUM         34
#define Y7_GPIO_NUM         39
#define Y6_GPIO_NUM         36
#define Y5_GPIO_NUM         21
#define Y4_GPIO_NUM         19
#define Y3_GPIO_NUM         18
#define Y2_GPIO_NUM         5
#define VSYNC_GPIO_NUM      25
#define HREF_GPIO_NUM       23
#define PCLK_GPIO_NUM       22

// ======================= CONSTANTS =======================
#define AUDIO_SAMPLE_RATE   16000       // 16kHz sampling
#define AUDIO_BUFFER_SIZE   32000       // 2 seconds of audio (16000 * 2)
#define ZCR_THRESHOLD_HIGH  0.30        // Chainsaw detection threshold
#define ZCR_THRESHOLD_LOW   0.10        // Fire detection threshold
#define RMS_THRESHOLD_MIN   500.0       // Minimum energy to be a threat
#define RMS_THRESHOLD_FIRE  1500.0      // Fire crackling energy

// ======================= GLOBAL VARIABLES =======================
int16_t audioBuffer[AUDIO_BUFFER_SIZE];
RTC_DATA_ATTR int wakeCount = 0;        // Persists across deep sleep
RTC_DATA_ATTR int falseAlarmCount = 0;

// ======================= TIER 3: EDGE AI =======================

/**
 * Calculate Zero-Crossing Rate (ZCR)
 * Measures how often the audio signal changes sign.
 * High ZCR = noise-like (chainsaw friction)
 * Low ZCR = tonal (wind, animal calls)
 */
float calculateZCR(int16_t* samples, int numSamples) {
    int crossings = 0;
    for (int i = 1; i < numSamples; i++) {
        if ((samples[i] >= 0 && samples[i-1] < 0) ||
            (samples[i] < 0 && samples[i-1] >= 0)) {
            crossings++;
        }
    }
    return (float)crossings / (float)(numSamples - 1);
}

/**
 * Calculate Root Mean Square (RMS) Energy
 * Measures the overall loudness/energy of the audio signal.
 */
float calculateRMS(int16_t* samples, int numSamples) {
    int64_t sumSquares = 0;
    for (int i = 0; i < numSamples; i++) {
        sumSquares += (int64_t)samples[i] * (int64_t)samples[i];
    }
    return sqrt((double)sumSquares / (double)numSamples);
}

/**
 * Classify threat based on ZCR and RMS values
 * Returns: 0 = no threat, 1 = chainsaw, 2 = fire
 */
int classifyThreat(float zcr, float rms) {
    // Not enough energy = background noise
    if (rms < RMS_THRESHOLD_MIN) {
        return 0;  // No threat
    }
    
    // High ZCR + High energy = mechanical (chainsaw)
    if (zcr > ZCR_THRESHOLD_HIGH && rms > RMS_THRESHOLD_MIN) {
        return 1;  // CHAINSAW detected
    }
    
    // Low-medium ZCR + Medium energy = fire crackling
    if (zcr < ZCR_THRESHOLD_LOW && rms > RMS_THRESHOLD_FIRE) {
        return 2;  // FIRE detected
    }
    
    return 0;  // No threat - return to sleep
}

// ======================= HARDWARE FUNCTIONS =======================

/**
 * Initialize I2S for INMP441 microphone
 */
void initMicrophone() {
    // Power on the microphone via MOSFET
    digitalWrite(PIN_MIC_POWER, HIGH);
    delay(100);  // Let mic stabilize
    
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pinConfig = {
        .bck_io_num = PIN_I2S_SCK,
        .ws_io_num = PIN_I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_I2S_SD
    };
    
    i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pinConfig);
    i2s_zero_dma_buffer(I2S_NUM_0);
}

/**
 * Capture audio samples into buffer
 */
bool captureAudio() {
    size_t bytesRead = 0;
    size_t totalBytesRead = 0;
    int samplesNeeded = AUDIO_BUFFER_SIZE;
    
    while (totalBytesRead < samplesNeeded * 2) {  // 2 bytes per sample
        i2s_read(I2S_NUM_0, 
                 (void*)(audioBuffer + totalBytesRead / 2),
                 (samplesNeeded * 2) - totalBytesRead,
                 &bytesRead, 
                 portMAX_DELAY);
        totalBytesRead += bytesRead;
    }
    
    return totalBytesRead == samplesNeeded * 2;
}

/**
 * Shutdown microphone and I2S
 */
void shutdownMicrophone() {
    i2s_driver_uninstall(I2S_NUM_0);
    digitalWrite(PIN_MIC_POWER, LOW);  // Power off mic via MOSFET
}

/**
 * Initialize and capture photo with OV2640
 */
camera_fb_t* capturePhoto() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;     // 640x480
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return NULL;
    }
    
    // Take photo
    camera_fb_t* fb = esp_camera_fb_get();
    return fb;
}

/**
 * Send alert to cloud via HTTP POST
 */
bool sendAlert(int threatType, float confidence, camera_fb_t* photo) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(250);
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection failed!");
        return false;
    }
    
    HTTPClient http;
    http.begin(WEBHOOK_URL);
    http.addHeader("Content-Type", "application/json");
    
    // Build JSON payload
    String threatName = (threatType == 1) ? "chainsaw" : "fire";
    String payload = "{";
    payload += "\"node_id\":\"" + String(NODE_ID) + "\",";
    payload += "\"threat_type\":\"" + threatName + "\",";
    payload += "\"confidence\":" + String(confidence, 2) + ",";
    payload += "\"battery\":" + String(analogRead(34) * 100 / 4095) + ",";
    payload += "\"wake_count\":" + String(wakeCount) + ",";
    payload += "\"timestamp\":\"" + String(millis()) + "\"";
    // TODO: Add base64 encoded image
    payload += "}";
    
    int httpCode = http.POST(payload);
    bool success = (httpCode == 200 || httpCode == 201);
    
    http.end();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    return success;
}

/**
 * Enter deep sleep with ext_wakeup on sensor pins
 */
void enterDeepSleep() {
    // Configure wake-up sources (Tier 2 sensors)
    esp_sleep_enable_ext0_wakeup(PIN_FLAME_SENSOR, HIGH);
    // Can also add: esp_sleep_enable_ext1_wakeup(bitmask, ESP_EXT1_WAKEUP_ANY_HIGH);
    
    Serial.println("💤 Entering deep sleep... (Tier 1)");
    Serial.flush();
    
    esp_deep_sleep_start();
}

// ======================= MAIN PROGRAM =======================

void setup() {
    Serial.begin(115200);
    Serial.println("\n🌲 EdgeWake - Tiered Cascade Guardian System");
    Serial.printf("Wake count: %d | False alarms: %d\n", wakeCount, falseAlarmCount);
    
    wakeCount++;
    
    // Configure power management pin
    pinMode(PIN_MIC_POWER, OUTPUT);
    digitalWrite(PIN_MIC_POWER, LOW);
    
    // ========== TIER 2: Check wake-up source ==========
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    
    if (wakeupReason != ESP_SLEEP_WAKEUP_EXT0 && 
        wakeupReason != ESP_SLEEP_WAKEUP_EXT1) {
        Serial.println("⚡ First boot or spurious wake - going to sleep");
        enterDeepSleep();
        return;
    }
    
    Serial.println("🔥 TIER 2: Hardware interrupt detected!");
    
    // ========== TIER 3: Edge AI Audio Verification ==========
    Serial.println("🎤 TIER 3: Powering on microphone for audio analysis...");
    initMicrophone();
    
    Serial.println("📊 Capturing 2 seconds of audio...");
    if (!captureAudio()) {
        Serial.println("❌ Audio capture failed!");
        shutdownMicrophone();
        enterDeepSleep();
        return;
    }
    
    // Calculate audio features
    float zcr = calculateZCR(audioBuffer, AUDIO_BUFFER_SIZE);
    float rms = calculateRMS(audioBuffer, AUDIO_BUFFER_SIZE);
    
    Serial.printf("📈 ZCR: %.4f | RMS: %.1f\n", zcr, rms);
    
    // Classify threat
    int threatType = classifyThreat(zcr, rms);
    
    shutdownMicrophone();  // Power off mic immediately
    
    if (threatType == 0) {
        Serial.println("✅ No threat detected - false alarm");
        falseAlarmCount++;
        enterDeepSleep();
        return;
    }
    
    float confidence = (threatType == 1) ? 
        min(1.0f, zcr / ZCR_THRESHOLD_HIGH) : 
        min(1.0f, rms / RMS_THRESHOLD_FIRE);
    
    Serial.printf("⚠️ THREAT CONFIRMED: %s (confidence: %.0f%%)\n", 
        (threatType == 1) ? "CHAINSAW" : "FIRE", confidence * 100);
    
    // ========== TIER 4: Visual Evidence Capture ==========
    Serial.println("📸 TIER 4: Capturing photo evidence...");
    camera_fb_t* photo = capturePhoto();
    
    if (photo) {
        Serial.printf("✅ Photo captured: %d bytes (%dx%d)\n", 
            photo->len, photo->width, photo->height);
    } else {
        Serial.println("⚠️ Photo capture failed - sending alert without image");
    }
    
    // ========== TIER 5: Cloud Handoff ==========
    Serial.println("📡 TIER 5: Sending alert to cloud...");
    bool sent = sendAlert(threatType, confidence, photo);
    
    if (sent) {
        Serial.println("✅ Alert sent successfully!");
    } else {
        Serial.println("❌ Alert sending failed - will retry on next wake");
        // TODO: Store alert in RTC memory for retry
    }
    
    // Cleanup
    if (photo) {
        esp_camera_fb_return(photo);
    }
    esp_camera_deinit();
    
    // Return to Tier 1
    enterDeepSleep();
}

void loop() {
    // Never reached - deep sleep restarts from setup()
}
