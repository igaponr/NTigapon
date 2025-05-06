/*
 * @file drone_remote_id.ino
 * @author igapon (igapon@gmail.com)
 * @brief M5StickCPlus2 get drone remote id
 * @version 0.1
 * @date 2023-12-12
 * @Hardwares: M5StickCPlus2
 * @Platform Version: Arduino M5Stack Board Manager v2.1.4
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * M5StickCPlus2: https://github.com/m5stack/M5StickCPlus2
 */
#include "WiFi.h"
#include "M5StickCPlus2.h"

void setup()
{
    auto cfg = M5.config();
    StickCP2.begin(cfg);
    // StickCP2.Display.setRotation(1);
    StickCP2.Display.setTextColor(GREEN);
    StickCP2.Display.setTextDatum(middle_center);
    StickCP2.Display.setFont(&fonts::Font0);
    StickCP2.Display.setTextSize(1);
    Serial.begin(115200);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    StickCP2.Display.println("Setup done");
}

void loop()
{
    StickCP2.Display.clear();
    int vol = StickCP2.Power.getBatteryVoltage();
    StickCP2.Display.setCursor(10, 30);
    StickCP2.Display.printf("BAT: %dmv", vol);
    StickCP2.Display.println("Scan start");

    // WiFi.scanNetworks will return the number of networks found.
    int n = WiFi.scanNetworks();
    StickCP2.Display.println("Scan done");
    if (n == 0) {
        StickCP2.Display.println("no networks found");
    } else {
        StickCP2.Display.print(n);
        StickCP2.Display.println(" networks found");
        StickCP2.Display.println("Nr | SSID                             | RSSI | CH | Encryption");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            StickCP2.Display.printf("%2d",i + 1);
            StickCP2.Display.print(" | ");
            StickCP2.Display.printf("%-32.32s", WiFi.SSID(i).c_str());
            StickCP2.Display.print(" | ");
            StickCP2.Display.printf("%4d", WiFi.RSSI(i));
            StickCP2.Display.print(" | ");
            StickCP2.Display.printf("%2d", WiFi.channel(i));
            StickCP2.Display.print(" | ");
            switch (WiFi.encryptionType(i))
            {
            case WIFI_AUTH_OPEN:
                StickCP2.Display.print("open");
                break;
            case WIFI_AUTH_WEP:
                StickCP2.Display.print("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                StickCP2.Display.print("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                StickCP2.Display.print("WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                StickCP2.Display.print("WPA+WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                StickCP2.Display.print("WPA2-EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                StickCP2.Display.print("WPA3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                StickCP2.Display.print("WPA2+WPA3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                StickCP2.Display.print("WAPI");
                break;
            default:
                StickCP2.Display.print("unknown");
            }
            StickCP2.Display.println();
            delay(10);
        }
    }
    StickCP2.Display.println("");

    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();

    // Wait a bit before scanning again.
    delay(5000);
}
