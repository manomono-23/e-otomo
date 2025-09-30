/**
 * AI Sticky Note with Bluetooth Support for nRF52840
 * nRF52840 + Waveshare 1.54inch e-Paper Module (B)
 *
 * Features:
 * - Bluetooth communication for receiving 200x200 drawing data
 * - 2-color or 3-color e-paper display (configurable)
 * - Deep sleep with button wake-up
 * - LED status indicator
 */

// ========== CONFIGURATION ==========
// Display type selection (uncomment one)
// #define DISPLAY_2COLOR  // 2-color display (Black/White)
 #define DISPLAY_3COLOR  // 3-color display (Black/White/Red)

// Device ID (change this for each device)
#define DEVICE_ID "0002"

/**
 * Pin Configuration:
 * nRF52840 → e-Paper Module / LED / Switch
 * 3V3  → VCC
 * GND  → GND
 * D6   → DIN (MOSI)
 * D7   → CLK (SCK)
 * D3   → BUSY
 * D1   → CS
 * D5   → DC
 * D4   → RST
 * D0   → LED (Status indicator)
 * D2   → Push Switch → 3.3V (Wake up from deep sleep)
 */

#ifdef DISPLAY_2COLOR
  #include <GxEPD2_BW.h>  // 2-color display library
#elif defined(DISPLAY_3COLOR)
  #include <GxEPD2_3C.h>  // 3-color display library
#endif

#include <Fonts/FreeMonoBold9pt7b.h>
#include <SPI.h>
#include <bluefruit.h>
#include "qr_pattern.h"

// Pin definitions for nRF52840
#define CS_PIN      D1   // Chip Select
#define DC_PIN      D5   // Data/Command
#define RST_PIN     D4   // Reset
#define BUSY_PIN    D3   // Busy
#define LED_PIN     D0   // Status LED
// Blue LED is built-in LED_BLUE pin (P0.06 on XIAO nRF52840)
#define SWITCH_PIN  D2  // Push Switch (Wake up)

// Display constructor (conditional based on display type)
#ifdef DISPLAY_2COLOR
  // 2-color display (Black/White)
  GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));
#elif defined(DISPLAY_3COLOR)
  // 3-color display (Black/White/Red)
  GxEPD2_3C<GxEPD2_154_Z90c, GxEPD2_154_Z90c::HEIGHT> display(GxEPD2_154_Z90c(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));
#endif

// LED control variables
volatile bool displayUpdating = false;
const unsigned long LED_ON_TIME = 10;    // LEDを点灯する時間(ms)

// Deep sleep management variables
unsigned long lastActivityTime = 0;
unsigned long connectionStartTime = 0;
bool isConnected = false;
const unsigned long NO_CONNECTION_TIMEOUT = 30000;  // 30 seconds
const unsigned long CONNECTED_TIMEOUT = 90000;     // 90 seconds
const unsigned long LED_OFF_TIME = 2000;  // LEDを消灯する時間(ms)
const unsigned long LONG_PRESS_THRESHOLD = 1500;   // 長押し判定時間(ms)

// Bluetooth Low Energy
BLEService aiStickyService;
BLECharacteristic drawingDataChar;

// Command types
#define CMD_DRAW_DATA     0x01
#define CMD_RLE_COMPRESSED_DATA 0x02
#define CMD_COMPRESSED_DATA 0x03
#define CMD_STATUS_REQUEST 0x04

// Drawing data storage (200x200 pixels = 40,000 pixels)
// 1 pixel = 2 bits (00=white, 01=black, 10=red, 11=reserved)
// Total: 40,000 pixels × 2 bits = 80,000 bits = 10,000 bytes
uint8_t drawingData[10000];
uint16_t dataReceiveOffset = 0;
bool newDataReceived = false;
bool drawingInProgress = false;

// Compressed data handling
uint8_t compressedDataBuffer[8192]; // Buffer for compressed data
uint16_t compressedDataReceived = 0;
uint16_t expectedCompressedLength = 0;
bool compressedDataInProgress = false;

// RLE compressed data handling
uint8_t rleDataBuffer[8192]; // Buffer for RLE compressed data
uint16_t rleDataReceived = 0;
uint16_t expectedRLELength = 0;
bool rleDataInProgress = false;
uint8_t rleDisplayMode = 4; // Default to 4-color mode

// BLE Service and Characteristic UUIDs (Fixed)
const char* SERVICE_UUID = "12345678-1234-5678-9abc-123456789abd";
const char* CHARACTERISTIC_UUID = "87654321-4321-8765-cba9-987654321abd";


// Function prototypes
void setupBluetooth();
void processDrawingData();
void processCommand(uint8_t* data, size_t length);
void showWelcomeMessage();
void enterDeepSleep();
void wakeUpSignal();
bool checkLongPress();
void drawingDataWriteCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len);
void decompressRLEData(uint8_t* compressedData, size_t dataLength, uint16_t expectedLength);
void decompressRLEToDrawingData(uint8_t* rleData, size_t dataLength, uint8_t displayMode);

void setup() {
  // Serial.begin(115200);
  // while (!Serial) delay(10);

  Serial.println("=== AI Sticky Note for nRF52840 ===");

  // Signal wake up from deep sleep with LED pattern
  wakeUpSignal();
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize blue LED and keep it ON (LOW = ON for XIAO nRF52840)
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  
  // Initialize switch pin (external pull-down resistor connected)
  pinMode(SWITCH_PIN, INPUT);

  // Manual reset of the e-Paper display
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(20);
  digitalWrite(RST_PIN, HIGH);
  delay(200);

  // Set control pins as output
  pinMode(CS_PIN, OUTPUT);
  pinMode(DC_PIN, OUTPUT);

  // Initialize SPI
  SPI.begin();

  // Initialize display
  display.init(115200, true, 2, false, SPI, SPISettings(1000000, MSBFIRST, SPI_MODE0));
  Serial.println("Display initialized");

  // Check if button was long-pressed during wake-up
  if (checkLongPress()) {
    Serial.println("Long press detected - showing QR code");
    showWelcomeMessage();
  } else {
    Serial.println("Short press detected - skipping display update");
  }
  
  Serial.println("Device initialization:");
  Serial.printf("Device ID: %s\n", DEVICE_ID);
  Serial.printf("Service UUID: %s\n", SERVICE_UUID);
  Serial.printf("Characteristic UUID: %s\n", CHARACTERISTIC_UUID);

  // Setup Bluetooth
  setupBluetooth();

  // Initialize timing for deep sleep management
  lastActivityTime = millis();
  connectionStartTime = millis();

  Serial.println("Setup completed - ready for Bluetooth connection");
}

void loop() {
  // Check for deep sleep conditions
  unsigned long currentTime = millis();

  // Check for no connection timeout (30 seconds)
  if (!isConnected) {
    if (currentTime - connectionStartTime > NO_CONNECTION_TIMEOUT) {
      Serial.println("No connection timeout - entering deep sleep");
      enterDeepSleep();
    }
  } else {
    // Check for connected but no activity timeout (90 seconds)
    if (currentTime - lastActivityTime > CONNECTED_TIMEOUT) {
      Serial.println("Connected but no activity timeout - entering deep sleep");
      enterDeepSleep();
    }
  }
  
  // LED blinking continuously during operation
  static bool ledState = false;
  static unsigned long lastToggle = 0;

  if (currentTime - lastToggle > (ledState ? LED_ON_TIME : LED_OFF_TIME)) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    lastToggle = currentTime;
  }
  
  // Check for new drawing data from Bluetooth
  if (newDataReceived) {
    newDataReceived = false;
    lastActivityTime = millis();  // Reset activity timer
    
    Serial.println("Processing received drawing data...");
    
    // Process and display the drawing data
    processDrawingData();

    displayUpdating = false;

    Serial.println("Drawing data processed and displayed");
  }
  
  
  delay(10);
}

void setupBluetooth() {
  Serial.println("Setting up Bluetooth LE...");

  // Disable automatic connection LED blinking on blue LED (P0.06)
  Bluefruit.autoConnLed(false);

  // Initialize Bluefruit with maximum performance configuration
  Bluefruit.begin();
  // Set device name with unique ID
  String deviceName;
#ifdef DISPLAY_2COLOR
  deviceName = "Otomoe2C " + String(DEVICE_ID);
#elif defined(DISPLAY_3COLOR)
  deviceName = "Otomoe3C " + String(DEVICE_ID);
#else
  #error "Please define either DISPLAY_2COLOR or DISPLAY_3COLOR"
#endif

  Bluefruit.setName(deviceName.c_str());
  Serial.printf("BLE Device Name: %s\n", deviceName.c_str());

  // Set connection callbacks for sleep management
  Bluefruit.Periph.setConnectCallback([](uint16_t conn_handle) {
    isConnected = true;
    connectionStartTime = millis();
    lastActivityTime = millis();
    Serial.println("Connected to device");
  });

  Bluefruit.Periph.setDisconnectCallback([](uint16_t conn_handle, uint8_t reason) {
    isConnected = false;
    Serial.println("Disconnected from device");
  });

  // Set maximum speed connection parameters
  Bluefruit.Periph.setConnInterval(6, 6);   // Fixed 7.5ms interval (minimum allowed)
  Bluefruit.Periph.setConnSlaveLatency(0);  // No latency
  Bluefruit.Periph.setConnSupervisionTimeout(4000); // 40 second timeout
  
  // Request 2M PHY for higher throughput (if supported)
  // Note: PHY switching is handled automatically by the BLE stack
  
  // Configure and start the BLE service with compile-time UUID
  aiStickyService = BLEService(BLEUuid(SERVICE_UUID));
  aiStickyService.begin();

  // Configure the drawing data characteristic with compile-time UUID
  drawingDataChar = BLECharacteristic(BLEUuid(CHARACTERISTIC_UUID));
  drawingDataChar.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
  drawingDataChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  drawingDataChar.setMaxLen(244); // Use BLE 4.2 maximum (244 bytes payload + 3 bytes header = 247)
  drawingDataChar.setWriteCallback(drawingDataWriteCallback);
  drawingDataChar.begin();
  
  // Data length extension is automatically negotiated by the BLE stack
  
  // Setup advertising
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(aiStickyService);
  Bluefruit.Advertising.addName();
  
  // Start advertising
  Bluefruit.Advertising.start(0); // 0 = Don't stop advertising
  
  Serial.println("Bluetooth LE advertising started");
  Serial.printf("Service UUID: %s\n", SERVICE_UUID);
  Serial.printf("Characteristic UUID: %s\n", CHARACTERISTIC_UUID);
  Serial.println("Waiting for client connection...");
}

void drawingDataWriteCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  // Update activity time when data is received
  lastActivityTime = millis();

  // Minimize serial output for faster processing (only for debugging when needed)
  #ifdef ENABLE_DEBUG_VERBOSE
  Serial.printf("Received data: %d bytes\n", len);
  #endif
  
  if (len > 0) {
    // Process command immediately without unnecessary delays
    processCommand(data, len);
  }
}


void processDrawingData() {
  Serial.println("Converting drawing data to display format with 180-degree rotation...");

  display.setRotation(2);  // 180-degree rotation
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    // Convert received data to display pixels
#ifdef DISPLAY_2COLOR
    // 2-color mode: 1 bit per pixel (0=white, 1=black)
    for (int y = 0; y < 200; y++) {
      for (int x = 0; x < 200; x++) {
        int pixelIndex = y * 200 + x;
        int byteIndex = pixelIndex / 8;
        int bitOffset = pixelIndex % 8;

        if (byteIndex < sizeof(drawingData)) {
          uint8_t pixelValue = (drawingData[byteIndex] >> bitOffset) & 0x01;
          if (pixelValue == 1) {
            display.drawPixel(x, y, GxEPD_BLACK);
          }
        }
      }
    }
#else
    // 3-color mode: 2 bits per pixel (00=white, 01=black, 10=red)
    for (int y = 0; y < 200; y++) {
      for (int x = 0; x < 200; x++) {
        int pixelIndex = y * 200 + x;
        int byteIndex = pixelIndex / 4;
        int bitOffset = (pixelIndex % 4) * 2;

        if (byteIndex < sizeof(drawingData)) {
          uint8_t pixelValue = (drawingData[byteIndex] >> bitOffset) & 0x03;

          switch (pixelValue) {
            case 0x01: // Black
              display.drawPixel(x, y, GxEPD_BLACK);
              break;
            case 0x02: // Red
              display.drawPixel(x, y, GxEPD_RED);
              break;
            case 0x00: // White (default)
            default:
              // Already white from fillScreen
              break;
          }
        }
      }
    }
#endif

  } while (display.nextPage());

  display.hibernate();

  Serial.println("Drawing data displayed on e-paper with 180-degree rotation");

  // Enter deep sleep immediately after drawing completion
  Serial.println("Drawing complete - entering deep sleep...");
  delay(100);  // Small delay to ensure message is sent
  enterDeepSleep();
}

void processCommand(uint8_t* data, size_t length) {
  if (length == 0) return;

  // If already receiving RLE compressed data, treat incoming data as payload only
  if (rleDataInProgress) {
    size_t remaining = expectedRLELength - rleDataReceived;
    size_t copyLen = length < remaining ? length : remaining;
    if (copyLen > 0 && rleDataReceived + copyLen <= sizeof(rleDataBuffer)) {
      memcpy(rleDataBuffer + rleDataReceived, data, copyLen);
      rleDataReceived += copyLen;
    }

    Serial.printf("RLE data: +%d bytes (%d/%d)\n", (int)copyLen, rleDataReceived, expectedRLELength);

    if (rleDataReceived >= expectedRLELength) {
      // All RLE data received, now decompress
      Serial.println("All RLE data received, decompressing...");
      decompressRLEToDrawingData(rleDataBuffer, rleDataReceived, rleDisplayMode);

      newDataReceived = true;
      rleDataInProgress = false;
      rleDataReceived = 0;
      displayUpdating = false;
      Serial.println("RLE data processing complete");
    }
    return;
  }

  // If already receiving compressed data, treat incoming data as payload only
  if (compressedDataInProgress) {
    size_t remaining = expectedCompressedLength - compressedDataReceived;
    size_t copyLen = length < remaining ? length : remaining;
    if (copyLen > 0 && compressedDataReceived + copyLen <= sizeof(compressedDataBuffer)) {
      memcpy(compressedDataBuffer + compressedDataReceived, data, copyLen);
      compressedDataReceived += copyLen;
    }
    
    Serial.printf("Compressed data: +%d bytes (%d/%d)\n", (int)copyLen, compressedDataReceived, expectedCompressedLength);
    
    if (compressedDataReceived >= expectedCompressedLength) {
      // All compressed data received, now decompress
      Serial.println("All compressed data received, decompressing...");
      decompressRLEData(compressedDataBuffer, compressedDataReceived, expectedCompressedLength);
      
      newDataReceived = true;
      compressedDataInProgress = false;
      compressedDataReceived = 0;
      displayUpdating = false;
      Serial.println("Compressed data processing complete");
    }
    return;
  }
  
  // If already receiving a drawing, treat incoming data as payload only
  if (drawingInProgress) {
    size_t remaining = sizeof(drawingData) - dataReceiveOffset;
    size_t copyLen = length < remaining ? length : remaining;
    if (copyLen > 0) {
      // Fast memory copy without function call overhead
      uint8_t* src = data;
      uint8_t* dst = drawingData + dataReceiveOffset;
      for (size_t i = 0; i < copyLen; i++) {
        *dst++ = *src++;
      }
      dataReceiveOffset += copyLen;
    }
    
    #ifdef ENABLE_DEBUG_VERBOSE
    Serial.printf("Receiving: +%d bytes (%d/%d)\n", (int)copyLen, dataReceiveOffset, (int)sizeof(drawingData));
    #endif
    
    // Check completion based on actual expected data size
#ifdef DISPLAY_2COLOR
    int expectedDataSize = 5000;  // 200*200/8 = 5000 bytes for 2-color
#else
    int expectedDataSize = 10000; // 200*200*2/8 = 10000 bytes for 3-color
#endif

    if (dataReceiveOffset >= expectedDataSize) {
      newDataReceived = true;
      dataReceiveOffset = 0;
      drawingInProgress = false;
      displayUpdating = false;
      Serial.printf("Drawing data complete: received %d bytes (expected %d)\n", dataReceiveOffset, expectedDataSize);
    }
    return;
  }
  
  uint8_t commandType = data[0];
  
  #ifdef ENABLE_DEBUG_VERBOSE  
  Serial.printf("Command: 0x%02X, length: %d\n", commandType, (int)length);
  #endif
  
  switch (commandType) {
    case CMD_DRAW_DATA:
      // Start of new drawing transfer (uncompressed)
      Serial.printf("Received drawing data chunk (start): %d bytes\n", (int)length);
      
      // Reset buffer and mark as in-progress
      memset(drawingData, 0, sizeof(drawingData));
      dataReceiveOffset = 0;
      drawingInProgress = true;
      displayUpdating = true;
      
      // Copy payload after header (skip command type + display mode = 2 bytes)
      if (length > 2) {
        size_t remaining = sizeof(drawingData) - dataReceiveOffset;
        size_t chunkLen = length - 2;  // Skip 2-byte header
        size_t copyLen = chunkLen < remaining ? chunkLen : remaining;
        if (copyLen > 0) {
          memcpy(drawingData + dataReceiveOffset, data + 2, copyLen);  // Start from data[2]
          dataReceiveOffset += copyLen;
        }
      }
      break;

    case CMD_RLE_COMPRESSED_DATA:
      // Start of RLE compressed data transfer
      Serial.println("Starting RLE compressed data reception");

      if (length >= 4) {
        rleDisplayMode = data[1]; // Extract display mode
        expectedRLELength = data[2] | (data[3] << 8);
        Serial.printf("Expected RLE data length: %d bytes, Display mode: %d\n", expectedRLELength, rleDisplayMode);

        // Reset RLE data reception
        rleDataReceived = 0;
        rleDataInProgress = true;
        displayUpdating = true;

        // Copy any payload in the first packet (after 4-byte header)
        if (length > 4) {
          size_t firstChunkLen = length - 4;
          if (firstChunkLen <= sizeof(rleDataBuffer)) {
            memcpy(rleDataBuffer, data + 4, firstChunkLen);
            rleDataReceived = firstChunkLen;
            Serial.printf("First RLE chunk: %d bytes\n", (int)firstChunkLen);
          }
        }

        // Check if all data received in first packet
        if (rleDataReceived >= expectedRLELength) {
          Serial.println("All RLE data in first packet, decompressing...");
          decompressRLEToDrawingData(rleDataBuffer, rleDataReceived, rleDisplayMode);

          newDataReceived = true;
          rleDataInProgress = false;
          rleDataReceived = 0;
          displayUpdating = false;
          Serial.println("RLE data processing complete");
        }
      }
      break;

    case CMD_COMPRESSED_DATA:
      // Start of compressed data transfer
      Serial.println("Starting compressed data reception");
      
      if (length >= 3) {
        expectedCompressedLength = data[1] | (data[2] << 8);
        Serial.printf("Expected compressed data length: %d bytes\n", expectedCompressedLength);
        
        // Reset compressed data reception
        compressedDataReceived = 0;
        compressedDataInProgress = true;
        displayUpdating = true;
        
        // Copy any payload in the first packet (after 3-byte header)
        if (length > 3) {
          size_t firstChunkLen = length - 3;
          if (firstChunkLen <= sizeof(compressedDataBuffer)) {
            memcpy(compressedDataBuffer, data + 3, firstChunkLen);
            compressedDataReceived = firstChunkLen;
            Serial.printf("First chunk: %d bytes\n", (int)firstChunkLen);
          }
        }
        
        // Check if all data received in first packet
        if (compressedDataReceived >= expectedCompressedLength) {
          Serial.println("All data in first packet, decompressing...");
          decompressRLEData(compressedDataBuffer, compressedDataReceived, expectedCompressedLength);
          
          newDataReceived = true;
          compressedDataInProgress = false;
          compressedDataReceived = 0;
          displayUpdating = false;
          Serial.println("Compressed data processing complete");
        }
      }
      break;
    
    // Some clients may send raw drawing bytes without the 0x01 header
    case 0x00: {
      Serial.println("Treating as raw drawing data start (no header)");
      // Start a raw-mode transfer
      memset(drawingData, 0, sizeof(drawingData));
      dataReceiveOffset = 0;
      drawingInProgress = true;
      displayUpdating = true;
      size_t remaining = sizeof(drawingData) - dataReceiveOffset;
      size_t copyLen = length < remaining ? length : remaining;
      if (copyLen > 0) {
        memcpy(drawingData + dataReceiveOffset, data, copyLen);
        dataReceiveOffset += copyLen;
      }
      break;
    }
      
    case CMD_STATUS_REQUEST:
      Serial.println("Status request received");
      break;
      
    default:
      Serial.printf("Unknown command: 0x%02X\n", commandType);
      break;
  }
}

void showWelcomeMessage() {
  Serial.println("Showing welcome message with QR code...");

  display.setRotation(2);  // 180-degree rotation
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    // Draw device ID at top center in black
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);

    // Calculate center position for device ID
    String deviceId = String(DEVICE_ID);
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(deviceId.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    int16_t x = (200 - tbw) / 2;
    int16_t y = 20; // Top margin

    display.setCursor(x, y);
    display.print(deviceId);

    // Draw QR code moved down (start at y=30)
    drawQRCode();

    // Draw 3-color stripe at bottom (195-200, full width)
    for (int x = 0; x < 200; x++) {
      for (int y = 195; y < 200; y++) {
        if (x < 67) {
          display.drawPixel(x, y, GxEPD_BLACK);  // Black stripe
        } else if (x < 134) {
          // White stripe (default background, no drawing needed)
        } else {
#ifdef DISPLAY_3COLOR
          display.drawPixel(x, y, GxEPD_RED);    // Red stripe (3-color only)
#endif
        }
      }
    }

  } while (display.nextPage());

  display.hibernate();
  Serial.println("Welcome message with QR code displayed");
}

void drawQRCode() {
  // QR Code parameters - medium size display with device ID at top
  const uint8_t qr_scale = 5;  // Medium scale (33*5=165)
  const uint8_t qr_x = (200 - 165) / 2;  // Center horizontally (17px margin)
  const uint8_t qr_y = 15;     // Y position below device ID


  // Draw the QR code
  for (int y = 0; y < qr_size; y++) {
    for (int x = 0; x < qr_size; x++) {
      // Get bit from pattern array (33 bits per row, 5 bytes per row)
      int byte_index = y * 5 + (x / 8);
      int bit_offset = 7 - (x % 8);
      bool is_black = (qr_pattern[byte_index] >> bit_offset) & 0x01;

      if (is_black) {
        // Draw a scaled pixel (qr_scale x qr_scale)
        for (int dy = 0; dy < qr_scale; dy++) {
          for (int dx = 0; dx < qr_scale; dx++) {
            int pixel_x = qr_x + x * qr_scale + dx;
            int pixel_y = qr_y + y * qr_scale + dy;
            if (pixel_x < 200 && pixel_y < 200) {  // Boundary check
              display.drawPixel(pixel_x, pixel_y, GxEPD_BLACK);
            }
          }
        }
      }
    }
  }
}

void decompressRLEData(uint8_t* compressedData, size_t dataLength, uint16_t expectedLength) {
  // Clear the drawing data buffer first
  memset(drawingData, 0, sizeof(drawingData));
  
  uint16_t pixelIndex = 0;
  size_t compressedIndex = 0;
  
  // Process RLE compressed data
  while (compressedIndex < dataLength && pixelIndex < 40000) {
    if (compressedIndex + 1 >= dataLength) break;
    
    uint8_t runLength = compressedData[compressedIndex++];
    uint8_t pixelValue = compressedData[compressedIndex++];
    
    // Expand run-length encoded pixels
    for (uint8_t i = 0; i < runLength && pixelIndex < 40000; i++) {
      // Pack into 2-bit format (4 pixels per byte)
      uint16_t byteIndex = pixelIndex / 4;
      uint8_t bitOffset = (pixelIndex % 4) * 2;
      
      if (byteIndex < sizeof(drawingData)) {
        drawingData[byteIndex] |= (pixelValue << bitOffset);
      }
      
      pixelIndex++;
    }
  }
  
  Serial.printf("Decompressed %d pixels from %d bytes\n", pixelIndex, dataLength);
}

void enterDeepSleep() {
  Serial.println("Entering deep sleep mode...");
  //Serial.flush();

  // Signal deep sleep entry with LED pattern (10ms on, 100ms off, 3 times)
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);   // LED on
    delay(10);                     // 10ms on
    digitalWrite(LED_PIN, LOW);    // LED off
    delay(100);                    // 100ms off
  }

  // Ensure status LED is off, but keep blue LED ON (LOW = ON for XIAO nRF52840)
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BLUE, HIGH);

  // Stop Bluetooth advertising and disconnect if connected
  if (isConnected) {
    Bluefruit.disconnect(0); // Disconnect first connection
  }
  Bluefruit.Advertising.stop();

  // Put display in hibernate mode to save power
  display.hibernate();

  // Configure D2 (SWITCH_PIN) as wake-up source for System OFF mode
  // Use INPUT_SENSE_HIGH to wake when pin goes HIGH (external pull-down resistor present)
  pinMode(SWITCH_PIN, INPUT_SENSE_HIGH);

  Serial.println("Deep sleep configured. Wake on D2 rising edge.");
  Serial.flush();  // Ensure all serial output is sent
  delay(100);

  // Enter System OFF mode (deepest sleep)
  // Device will reset when waking up from this mode
  sd_power_system_off();
}

void wakeUpSignal() {
  // Initialize LED pins first
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize blue LED and keep it ON (LOW = ON for XIAO nRF52840)
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);

  // Signal wake up from deep sleep with LED pattern (10ms on, 100ms off, 5 times)
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);   // LED on
    delay(10);                     // 10ms on
    digitalWrite(LED_PIN, LOW);    // LED off
    delay(100);                    // 100ms off
  }

  // Ensure LED is off
  digitalWrite(LED_PIN, LOW);
}

bool checkLongPress() {
  // Check if button is currently pressed
  if (digitalRead(SWITCH_PIN) == LOW) {
    // Button is not pressed, this is a short wake-up pulse
    return false;
  }

  Serial.println("Button pressed - checking for long press...");

  // Wait and check if button is still pressed after threshold time
  unsigned long startTime = millis();
  bool stillPressed = true;

  while (millis() - startTime < LONG_PRESS_THRESHOLD && stillPressed) {
    stillPressed = digitalRead(SWITCH_PIN) == HIGH;
    delay(10);  // Small delay to avoid excessive polling
  }

  // If button is still pressed after threshold time, it's a long press
  if (stillPressed && (millis() - startTime >= LONG_PRESS_THRESHOLD)) {
    Serial.println("Long press confirmed");
    return true;
  } else {
    Serial.println("Short press detected");
    return false;
  }
}

void decompressRLEToDrawingData(uint8_t* rleData, size_t dataLength, uint8_t displayMode) {
  // Clear the drawing data buffer first
  memset(drawingData, 0, sizeof(drawingData));

  uint16_t pixelIndex = 0;
  size_t rleIndex = 0;

  Serial.printf("Decompressing RLE data: %d bytes for %d-color mode\n", dataLength, displayMode);

  // Process RLE compressed data
  while (rleIndex < dataLength && pixelIndex < 40000) {
    if (rleIndex + 1 >= dataLength) break;

    uint8_t runLength = rleData[rleIndex++];
    uint8_t pixelValue = rleData[rleIndex++];

    // Expand run-length encoded pixels
    for (uint8_t i = 0; i < runLength && pixelIndex < 40000; i++) {
      if (displayMode == 2) {
        // 2-color mode: 1 bit per pixel
        uint16_t byteIndex = pixelIndex / 8;
        uint8_t bitOffset = pixelIndex % 8;
        if (pixelValue > 0 && byteIndex < sizeof(drawingData)) {
          drawingData[byteIndex] |= (1 << bitOffset);
        }
      } else {
        // 3-color and 4-color mode: 2 bits per pixel
        uint16_t byteIndex = pixelIndex / 4;
        uint8_t bitOffset = (pixelIndex % 4) * 2;
        if (byteIndex < sizeof(drawingData)) {
          drawingData[byteIndex] |= (pixelValue << bitOffset);
        }
      }
      pixelIndex++;
    }
  }

  Serial.printf("RLE decompressed %d pixels from %d bytes\n", pixelIndex, dataLength);
}

