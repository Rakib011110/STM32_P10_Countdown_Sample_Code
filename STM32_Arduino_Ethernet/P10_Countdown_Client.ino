/**
 * ============================================================================
 *  P10 Display TCP Client for STM32 Nucleo (Arduino / STM32Duino Framework)
 *  Board: STM32 Nucleo-F767ZI / F429ZI / any STM32 with Ethernet
 * ============================================================================
 *  Connects over Ethernet TCP port 5000 to ESP32 P10 Display Server.
 *  Sends commands: COUNTDOWN, PAUSE, RESUME, STOP, TEXT, STATUS.
 * ============================================================================
 */

#include <SPI.h>
#include <STM32Ethernet.h>

// ─── Network Configuration ──────────────────────────────────────
// STM32 Static IP Configuration (Must be on the same subnet as ESP32)
byte mac[] = { 0x00, 0x80, 0xE1, 0x01, 0x02, 0x03 };
IPAddress ip(172, 1, 1, 100);          // STM32 IP Address
IPAddress gateway(172, 1, 1, 1);       // Gateway
IPAddress subnet(255, 255, 255, 0);    // Subnet Mask

// ESP32 P10 Display Target Server
IPAddress display_ip(172, 1, 1, 4);    // ESP32 Display IP
const uint16_t display_port = 5000;    // ESP32 Port

EthernetClient client;
unsigned long lastStatusCheck = 0;
bool countdownStarted = false;

// ─── Helper Functions ───────────────────────────────────────────

void sendDisplayCommand(const char* cmd) {
    if (!client.connected()) {
        Serial.println("[TCP] Reconnecting to P10 Display...");
        if (!client.connect(display_ip, display_port)) {
            Serial.println("[TCP] Connection Failed!");
            return;
        }
        Serial.println("[TCP] Reconnected successfully!");
    }

    Serial.print("[SEND] --> ");
    Serial.println(cmd);

    // Send command with newline delimiter
    client.print(cmd);
    client.print("\n");
}

void readDisplayResponses() {
    while (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            Serial.print("[P10 ACK] <-- ");
            Serial.println(line);
        }
    }
}

// ─── Setup ──────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Wait for USB Serial monitor

    Serial.println("==================================================");
    Serial.println("   STM32 Nucleo -> ESP32 P10 Display Controller  ");
    Serial.println("==================================================");

    // Initialize STM32 built-in Ethernet (LAN8742A RMII)
    Serial.println("[ETH] Initializing STM32 Ethernet...");
    Ethernet.begin(mac, ip, gateway, gateway, subnet);
    delay(1000);

    Serial.print("[ETH] STM32 IP: ");
    Serial.println(Ethernet.localIP());

    // Connect to ESP32 Display Server
    Serial.print("[TCP] Connecting to ESP32 Display at ");
    Serial.print(display_ip);
    Serial.print(":");
    Serial.println(display_port);

    if (client.connect(display_ip, display_port)) {
        Serial.println("[TCP] Connected successfully!");

        // 1. Query status
        sendDisplayCommand("STATUS");

        // 2. Start a 30-second countdown
        delay(500);
        sendDisplayCommand("COUNTDOWN:30");
        countdownStarted = true;
    } else {
        Serial.println("[TCP] Initial connection failed! Will retry in loop.");
    }
}

// ─── Main Loop ──────────────────────────────────────────────────

void loop() {
    // 1. Read incoming ACKs from ESP32
    readDisplayResponses();

    // 2. Check connection health every 10 seconds
    if (millis() - lastStatusCheck > 10000) {
        lastStatusCheck = millis();

        if (client.connected()) {
            sendDisplayCommand("STATUS");
        } else {
            Serial.println("[TCP] Disconnected! Attempting reconnect...");
            if (client.connect(display_ip, display_port)) {
                Serial.println("[TCP] Reconnected!");
                if (!countdownStarted) {
                    sendDisplayCommand("COUNTDOWN:30");
                    countdownStarted = true;
                }
            }
        }
    }

    // 3. User serial input to test commands manually
    if (Serial.available()) {
        String userCmd = Serial.readStringUntil('\n');
        userCmd.trim();
        if (userCmd.length() > 0) {
            sendDisplayCommand(userCmd.c_str());
        }
    }
}
