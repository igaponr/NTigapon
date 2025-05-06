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
#include "M5StickCPlus2.h"
#include "WiFi.h"
#include "esp_wifi.h"
#include "esp_mac.h"

// メモリ描画領域表示（スプライト）のインスタンスを作成(必要に応じて複数作成)
// M5Canvas canvas(&M5.Lcd);  // &M5.Lcd を &StickCP2.Display と書いても同じ

void setup()
{
    auto cfg = M5.config();
    StickCP2.begin(cfg);
    StickCP2.Display.fillScreen(BLACK); // 背景色
    // StickCP2.Display.setRotation(1);  // 画面向き設定（USB位置基準 0：下/ 1：右/ 2：上/ 3：左）
    StickCP2.Display.setTextColor(GREEN);  //(文字色, 背景)
    // StickCP2.Display.setTextDatum(middle_center);
    // StickCP2.Display.setTextWrap(false);  // 改行をしない（画面をはみ出す時自動改行する場合はtrue。書かないとtrue）
    // canvas.createSprite(M5.Lcd.width(), M5.Lcd.height()); // lcdサイズ（メモリ描画領域）設定（画面サイズに設定）
    // メモリ描画領域を座標を指定して一括表示（スプライト）
    // canvas.pushSprite(&M5.Lcd, 0, 0);  // (0, 0)座標に一括表示実行
    StickCP2.Display.setFont(&fonts::Font0);
    StickCP2.Display.setTextSize(1);
    // Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    StickCP2.Display.println("Setup done");
}

void loop()
{
    int n = WiFi.scanNetworks();
    int vol = StickCP2.Power.getBatteryVoltage();
    StickCP2.Display.clear();
    StickCP2.Display.setCursor(0, 0);
    StickCP2.Display.printf("BAT: %dmv,", vol);
    if (n == 0) {
        StickCP2.Display.println("no networks found");
    } else {
        StickCP2.Display.print(n);
        StickCP2.Display.println(" networks found");
        // StickCP2.Display.println("No|RSSI|CH|Encryption|SSID|");
        StickCP2.Display.println("|RSSI|CH|Encrypt|SSID|");
        for (int i = 0; i < n; ++i) {
            if (i >= 6) {
                break;
            }
            // Print SSID and RSSI for each network found
            // StickCP2.Display.printf("%2d",i + 1);
            // StickCP2.Display.print("|");
            StickCP2.Display.printf("%4d", WiFi.RSSI(i));
            StickCP2.Display.print("|");
            StickCP2.Display.printf("%2d", WiFi.channel(i));
            StickCP2.Display.print("|");
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
            StickCP2.Display.print("|");
            // StickCP2.Display.printf("%-32.32s", WiFi.SSID(i).c_str());
            StickCP2.Display.printf("%s", WiFi.SSID(i).c_str());
            StickCP2.Display.println("");
        }
    }
    // StickCP2.Display.println("");
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
    // Wait a bit before scanning again.
    delay(5000);
}
