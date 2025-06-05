/*
 * @file drone_remote_id.ino
 * @author igapon (igapon@gmail.com)
 * @brief M5StickCPlus2 get drone remote id
 * @version 0.3 (Updated with M5CanvasTextDisplayController)
 * @date 2025-06-22 // 仮の日付
 * @Hardwares: M5StickCPlus2
 * @Platform Version: Arduino M5Stack Board Manager v2.1.4
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * M5StickCPlus2: https://github.com/m5stack/M5StickCPlus2
 * PlantUML Class図: https://www.plantuml.com/plantuml/uml/dLTjRnD74FxkNx7obzXfRDGFbIg48b1Um19HKHn4gwewRUxMzZRdM-jkBt5AaN9tG0dGGjL2Irk90Yb9eGGGhTeA4lvBro_aNtJsV3dVfaj8iIpVxRpCxCxCFZC-wd7iShzfvqPGkLn6XUxgboBOt8askfSkYk1n_wUWV-ENekJbGAHKAg5ZhkLJXudmRn4-5L4aeW0qEdyzVtK_W6__zvzHGOHFHFH8H2z4T5r43xjhgxqRJqHuVY2rS_EkN0OR8lXE1FTVtTdeNzikIkEv5ZONS9sW6KoTwWBXPWCTEJ9u6Hz7PtC8FgQDFGzPBdE8uP8cuyIWbW5RalpaSnREnQbv3hs3BCpn37R0geiEezbOeJ8vEQHgbQZNil7o17EuosoRk4QBI_MPzoQmiuYzETBcatjbHj_WlagQkDLWBW53wVi9x5WsCRXErsnWno4sY5D7Ax14QzGmcGyxM0Qfx59lyLXrRiRuu7G5X4tcU1pvOFbzWt_yoNxInjn7fwQCsShaNhLKgLJIwNk4-wr24RGM6RLq4ZPZUW7bX9v3QpMP0wWEmer673AtCE_NnX1VRf436w0EvQgxS_6lmpX1dBKGg-aAAx4heZiYN1FHCwXttIrO2HD1VVa76shvh_VMh_GkNTzL8uuLxoCdLzhoBfV7TVKBn_4zuksdjb_T7im09D-GFXDPLA0E6gG39Uae7cZVKx8sKoEB5AFCIVOtCLKzPPofJ5UCOwSggGNYjOmunQDZ252YN1o4RmJzy_s5mHTrNcxs7jthRtxHMrqPaeTvTsczmI7pYeCaaq0gLIQd8U5dQ6cQGkfsLkwbx8nxMLbZA5r2vHijbpMfP_hCzmotRPZpoGwLRE-xMn5y4t-teLWoEp-t2yNTjPk0r2BwDIxBc_8bUd18XgVGkxprwkcT_W_dHV1G1x1JRKvS1zidwRm740iF5xlBAiP2rHoV_Z23gYC3IWhtyONOYgj_5HKwp--1fzZb0n6685LCDMCic6NoLZAdaAKlKOit54g3o8Gef1grxQhf4WAwgarPq5Nw6T7H9vZDN8LXkNYfobtgr5NppE4dD5w1VEO_suDjO4wgBPToy8f6qhlL_-Dg_zkdaAhkroz40EtkgWYlP1HRljVOLKkn9mbrQaIDTa9ycu2doeLwhNruaBZHrvvZQGmrb40HcmmFgJz4UmmjxqrImZ9bvFz7fgOXqc-V4PdLYGO-KAh1uOJlUipLvk2q0vUNMEYi4aTqw10wRhDvR2Cs_oankS8jpvCwLUiIY7xBWkZkfPvaTPKmoqsLa80p9B_GtRpTsOBR_pZEtUvsuqTQBQNIv-F9DJsC8900Anw2y3icQN4VssCe4OpHKehL8Ek-IxpY402PWspgB6J66qGzXEFMAd7HYvCAmNS7hdAPcEKJX4GnFkOHzXGO1MY1tnHMv24Lo2mKKxg2dSaGypfsarb4ovFJhChGup7KAYUkHbAFt0SGV9r8CiDbsErqzfk8h-2CpqbRov_32qJBE4uNimnEczabk9KKLhDmDeBDXJroDZEn3JG5ZvHkfEGIy4L4NyLJpIMeokoCthru0VfCV_rvzygDUAY_BOM3zRXx0MIjWdntzVBEhJLePi25fGYVIJIBdabhuLqHteUNeUTaA-bafk_F8vaEVRLtxNfd-qSHt4fX5zYTBTZnRoAw1NCOgFMtrnBMi0EdxHIwU-Tbs5jPbokWxHdZajbFJdmlavTWSp0Ho7yyS8hBtGj_mfKTTDrYJZV8vD_Der7m9P_wMIvlPPQ5UemxnVoRZBrnmijh_z9a71o5WFXD-ry0
 * 参考: https://lang-ship.com/blog/work/esp32-wifi-sniffer/
 * 参考: https://qiita.com/kobatan/items/dac5d4696d631003e037
 * 参考: https://interface.cqpub.co.jp/wp-content/uploads/if2402_041.pdf
 */
#include <Arduino.h>
#include <string.h>
#include <ctime>
#include <M5StickCPlus2.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "RemoteIDDataManager.h"
#include "M5CanvasTextDisplayController.h"

#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)
#define WIFI_CHANNEL_MAX               (13) // 日本のWi-Fiチャンネルは通常1-13ch、14chは特殊。
#define MAX_SSID_LEN                   (32) // SSIDの最大長 (esp_wifi_types.hより)
#define SEND_MODE_TOP_RSSI 1       // 1: RSSI上位1件のデータを送信, 0: 指定登録記号のデータを送信
                                   // SEND_MODE_TOP_RSSI を 0 にすると指定登録記号モードになります。

const char* TARGET_REG_NO_FOR_JSON = "JA.TEST012345"; // 指定登録記号モードの場合の対象
const size_t MAX_ENTRIES_IN_JSON = 600; // 1つのRIDに対してJSONに含める最大エントリ数 (メモリと相談)
RemoteIDDataManager dataManager("");
M5CanvasTextDisplayController* displayController_ptr = nullptr;
static wifi_country_t wifi_country = {.cc = "JP", .schan = 1, .nchan = WIFI_CHANNEL_MAX};
int channel = 1;
SemaphoreHandle_t dataManagerSemaphore;
const uint8_t ASTM_OUI[] = {0xFA, 0x0B, 0xBC};
const uint8_t ASTM_OUI_TYPE_RID = 0x0D;
const int HEADER_LINES = 2; // ヘッダ情報が表示される行数 (例: "Ch: RIDs: Heap:", "-------")
const int LINES_PER_RID_ENTRY = 5;
int max_rids_to_display_calculated = 0;
// --- チャンネル固定モード用変数 ---
bool channelLockModeActive = false;
int lockedChannel = -1; // 固定中のチャンネル (-1なら固定されていない)
unsigned long lastChannelLockCheck = 0; // チャンネル固定モード時にターゲットチャンネルを再確認する間隔用
const unsigned long CHANNEL_LOCK_CHECK_INTERVAL = 5000; // 5秒ごとにターゲットのchを再確認

#pragma pack(push,1)
typedef struct{
	uint8_t ver:4;			    // bit[3-0] Protocol version
	uint8_t type:4;			    // bit[7-4] message type
} Message_head;

typedef struct{
	uint16_t fragment_num:4;	// bit[3-0]  Fragment Number
	uint16_t sequence_num:12;	// bit[15-4] Sequence Number
} Seq_ctl;

typedef struct{
	uint16_t fctl;				// Frame Control
	uint16_t duration = 0;	    // Duration
	uint8_t da[6];				// Destination Address
	uint8_t sa[6];				// Source Address
	uint8_t bssid[6];			// BSSID
	Seq_ctl seqctl;				// Sequence number
    uint64_t timestamp;		    // Timestamp
	uint16_t interval;			// Beacon interval
	uint16_t cap;				// Capability info
	uint8_t payload[0];
} wifi_mac_hdr_t;

typedef struct{
	uint8_t id;					// Element ID
	uint8_t len;				// Length
	uint8_t payload[0];
} element_head_t;

typedef struct{
	uint8_t speed_mul:1;	    // bit[0] 0:x0.25, 1:x0.75
	uint8_t dir_seg:1;		    // bit[1] 0:<180, 1:>=180
	uint8_t heght_type:1;	    // bit[2] 0:Abave Takeoff, 1:AGL
	uint8_t resv:1;			    // bit[3] reserved
	uint8_t status:4;		    // bit[7-4] status
} Status_flag;

typedef struct{
	uint8_t horizontal:4;	    // bit[3-0]
	uint8_t vetical:4;		    // bit[7-4]
} H_V_accuracy;				    // 水平垂直の正確さ

typedef struct{
	uint8_t speed:4;		    // bit[3-0]
	uint8_t baro:4;			    // bit[7-4]
}B_S_accuracy;				    // 速度方位の正確さ

typedef struct
{
	uint8_t counter = 0;		// Counter
	Message_head msg;			// message type Pack (0xf0)
	uint8_t block_size = 25;	// block size
	uint8_t block_n = 4;		// block count
	//--- Basic ID (25byte)--------------------------
	Message_head msg1;				// BASIC_ID
	uint8_t UA_type1:4;				// bit[3-0] 機体種別
	uint8_t ID_type1:4;				// bit[7-4] ID_TYPE_SerialNo
	char serial_no[20];				// 製造番号
	uint8_t resv1[3];
	//--- Basic ID (25byte)--------------------------
	Message_head msg2;				// BASIC_ID
	uint8_t UA_type2:4;				// bit[3-0] 機体種別
	uint8_t ID_type2:4;				// bit[7-4] ID_TYPE_ASSIGED_REG
	char reg_no[20];				// 登録記号 (例： JA.JU012345ABCDE)
	uint8_t resv2[3];
	//--- Location (25byte)--------------------------
	Message_head msg3;				// Location
	Status_flag status;				// 飛行中、方角E/W、速度倍率などの状態
	uint8_t dir;					// 方角
	uint8_t speed;					// 速度
	uint8_t Ver_speed;				// 垂直速度
	int32_t lat;					// 緯度
	int32_t lng;					// 経度
	int16_t Pressur_Altitude;  	    // 気圧高度
	int16_t Geodetic_Altitude; 	    // GPS高度
	uint16_t Height;				// 地面からの高さ
	H_V_accuracy hv_Accuracy;		// 水平垂直精度
	B_S_accuracy bs_Accuracy;		// 方位・速度精度
	uint16_t timestamp_detail;		// 現在時刻の分以下の小数点１までの秒数ｘ10
	uint8_t T_Accuracy;				// 時間精度(*0.1s)
	uint8_t resv3;
	//--- Page0 (25byte)-----------------------------
	Message_head msg4;				// 認証情報
	uint8_t auth_type = 0x30;		// Authentication　Message [認証情報]
	uint8_t page_count = 0;			// Page0
	uint8_t Length = 17;   			// headからのサイズ
	uint32_t timestamp_auth;		// 認証時刻？(2019.1.1からの秒数)
	uint8_t auth_head = 0; 	  	    // ヘッダ 0:AES-128bit-CCM
	uint8_t auth_data[16] = {0};    // 認証データ(AES-128bit-CCM ってなんだ？ 解からないので無視)
} RID_Data;
#pragma pack(pop)

void wifi_sniffer_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) {
        return;
    }
    wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t*)buf;
    wifi_mac_hdr_t *mac_hdr = (wifi_mac_hdr_t *)ppkt->payload;
    if (mac_hdr->fctl != 0x0080) { // Beacon Frame (Type 0, SubType 8)
        return;
    }
    int payload_len = ppkt->rx_ctrl.sig_len - sizeof(wifi_mac_hdr_t);
    if (payload_len <= 0) return;
    element_head_t *element = (element_head_t *)mac_hdr->payload;
    int remaining_len = payload_len;
    bool is_rid_ssid_found = false;
    String rid_serial_from_ssid = "";
    char ssid_str_buf[MAX_SSID_LEN + 1];
    // --- Pass 1: Find SSID and check if it's an RID SSID ---
    element_head_t *current_element = element;
    int current_remaining_len = remaining_len;
    while(current_remaining_len > sizeof(element_head_t)){
        if (current_element->id == 0) { // SSID Element ID
            if (current_element->len > 0 && current_element->len <= MAX_SSID_LEN) {
                strncpy(ssid_str_buf, (char*)current_element->payload, current_element->len);
                ssid_str_buf[current_element->len] = '\0';
                String temp_ssid = String(ssid_str_buf);
                if (temp_ssid.startsWith("RID-")) {
                    is_rid_ssid_found = true;
                    if (temp_ssid.length() > 4) {
                        rid_serial_from_ssid = temp_ssid.substring(4);
                        rid_serial_from_ssid.trim();
                    }
                    break;
                } else {
                    return;
                }
            }
        }
        int element_total_len = sizeof(element_head_t) + current_element->len;
        if (element_total_len == 0 || element_total_len > current_remaining_len) break;
        current_remaining_len -= element_total_len;
        current_element = (element_head_t *)((uint8_t *)current_element + element_total_len);
    }
    if (!is_rid_ssid_found) {
        return;
    }
    // --- Pass 2: Find Vendor Specific IE for ASTM RID ---
    current_element = element;
    current_remaining_len = remaining_len;
    while(current_remaining_len > sizeof(element_head_t)){
        const size_t min_vendor_ie_len = sizeof(ASTM_OUI) + 1;
        if (current_element->id == 221 && current_element->len >= min_vendor_ie_len) {
            const uint8_t* vendor_payload_ptr = (const uint8_t*)current_element->payload;
            if (memcmp(vendor_payload_ptr, ASTM_OUI, sizeof(ASTM_OUI)) == 0 &&
                vendor_payload_ptr[sizeof(ASTM_OUI)] == ASTM_OUI_TYPE_RID) {
                const size_t oui_header_size = sizeof(ASTM_OUI) + 1;
                if (current_element->len >= oui_header_size + sizeof(RID_Data)) {
                    RID_Data *data = (RID_Data *)(vendor_payload_ptr + oui_header_size);
                    char sn_buf[sizeof(data->serial_no) + 1];
                    memcpy(sn_buf, data->serial_no, sizeof(data->serial_no));
                    sn_buf[sizeof(data->serial_no)] = '\0';
                    String rid_from_payload_temp = String(sn_buf);
                    rid_from_payload_temp.trim();
                    String rid_from_payload = rid_from_payload_temp;
                    if (rid_from_payload.isEmpty()) {
                        rid_from_payload = rid_serial_from_ssid;
                    }
                    if (rid_from_payload.isEmpty()) {
                        M5.Log.printf("[WARNING] RID is empty after checking payload and SSID. Skipping.\n");
                        return;
                    }
                    time_t current_time_sec = time(NULL);
                    float latitude = static_cast<float>(data->lat) * 1e-7f;
                    float longitude = static_cast<float>(data->lng) * 1e-7f;
                    float pressure_alt = static_cast<float>(data->Pressur_Altitude) * 0.1f;
                    float gps_alt = static_cast<float>(data->Geodetic_Altitude) * 0.1f;
                    // 登録記号の取得
                    char reg_no_buf[sizeof(data->reg_no) + 1];
                    memcpy(reg_no_buf, data->reg_no, sizeof(data->reg_no));
                    reg_no_buf[sizeof(data->reg_no)] = '\0'; // ヌル終端
                    String registration_number_from_payload = String(reg_no_buf);
                    registration_number_from_payload.trim();
                    if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
                        dataManager.addData(
                            rid_from_payload,
                            ppkt->rx_ctrl.rssi,
                            current_time_sec,
                            mac_hdr->timestamp,
                            channel,
                            registration_number_from_payload,
                            latitude,
                            longitude,
                            pressure_alt,
                            gps_alt
                        );
                        xSemaphoreGive(dataManagerSemaphore);
                        // M5.Log.printf("RID: %s, Ch: %d, RSSI: %d, BcnTS: %llu, Lat: %.4f, Lon: %.4f, PAlt: %.1f, GAlt: %.1f\n",
                        //    rid_from_payload.c_str(), channel, ppkt->rx_ctrl.rssi, mac_hdr->timestamp, latitude, longitude, pressure_alt, gps_alt);
                    } else {
                        M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in sniffer_cb\n");
                    }
                    return;
                } else {
                     M5.Log.printf("[DEBUG] Vendor IE for ASTM RID too short. Len: %u, Expected Min: %u\n",
                                  current_element->len, oui_header_size + sizeof(RID_Data));
                }
            }
        }
        int element_total_len = sizeof(element_head_t) + current_element->len;
        if (element_total_len == 0 || element_total_len > current_remaining_len) {
             M5.Log.printf("[ERROR] Invalid element length: %d. Remaining: %d\n", current_element->len, current_remaining_len);
             break;
        }
        current_remaining_len -= element_total_len;
        current_element = (element_head_t *)((uint8_t *)current_element + element_total_len);
    }
}

void wifi_sniffer_init(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  wifi_promiscuous_filter_t filter;
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
  M5.Log.printf("Wi-Fi Sniffer initialized on Channel %d.\n", channel);
}

void setup()
{
    M5.begin(); // M5Unified の標準的な初期化 (DisplayControllerより先が良い)
    // M5CanvasTextDisplayController の初期化
    displayController_ptr = new M5CanvasTextDisplayController(M5.Display);
    if (!displayController_ptr) {
        M5.Log.printf("[FATAL] Failed to allocate Display Controller!\n");
        while(1); // Halt
    }
    // 初期設定: 文字サイズ1, 行ラップ無効, 文字色GREEN, 背景色BLACK, 回転1 (横向き)
    if (!displayController_ptr->begin(1, false, GREEN, BLACK, 1)) {
        M5.Log.printf("[FATAL] Failed to initialize Display Controller!\n");
        while(1); // Halt
    }
    M5CanvasTextDisplayController& dc = *displayController_ptr; // エイリアス
    // MAX_RIDS_TO_DISPLAY に相当する値を計算して設定
    int available_rows_for_rids = dc.getRows() - HEADER_LINES;
    if (available_rows_for_rids < 0) available_rows_for_rids = 0;
    max_rids_to_display_calculated = available_rows_for_rids / LINES_PER_RID_ENTRY;
    if (max_rids_to_display_calculated < 0) max_rids_to_display_calculated = 0;
    M5.Log.printf("[INFO] Calculated max RIDs to display: %d\n", max_rids_to_display_calculated);
    dataManagerSemaphore = xSemaphoreCreateMutex();
    if (dataManagerSemaphore == NULL) {
        M5.Log.printf("[FATAL] Failed to create dataManagerSemaphore!\n");
        dc.fillScreen(RED); // エラーを画面に表示
        dc.setTextColor(WHITE);
        dc.setCursor(0,0);
        dc.println("Semaphore FAIL");
        dc.show();
        while(1);
    }
    M5.Log.printf("[INFO] Setting up RTC and system time...\n");
    // ... (RTC設定部分は変更なし) ...
    auto dt = M5.Rtc.getDateTime();
    if (dt.date.year < 2023) { 
        M5.Log.printf("[INFO] RTC date is old (%04d-%02d-%02d), setting to a default time.\n",
                      dt.date.year, dt.date.month, dt.date.date);
        decltype(M5.Rtc.getDateTime()) default_datetime;
        default_datetime.date.year = 2024; 
        default_datetime.date.month = 1;   
        default_datetime.date.date = 1;    
        default_datetime.time.hours = 0;   
        default_datetime.time.minutes = 0;
        default_datetime.time.seconds = 0;
        M5.Rtc.setDateTime(default_datetime);
        M5.Log.printf("[INFO] RTC set to default: %04d-%02d-%02d %02d:%02d:%02d\n",
                      default_datetime.date.year, default_datetime.date.month, default_datetime.date.date,
                      default_datetime.time.hours, default_datetime.time.minutes, default_datetime.time.seconds);
        dt = M5.Rtc.getDateTime(); 
    }
    struct tm tminfo;
    tminfo.tm_year = dt.date.year - 1900;
    tminfo.tm_mon  = dt.date.month - 1;
    tminfo.tm_mday = dt.date.date;
    tminfo.tm_hour = dt.time.hours;
    tminfo.tm_min  = dt.time.minutes;
    tminfo.tm_sec  = dt.time.seconds;
    tminfo.tm_isdst = -1; 
    time_t rtc_epoch = mktime(&tminfo);
    if (rtc_epoch == -1) {
        M5.Log.printf("[ERROR] Failed to convert RTC datetime to epoch. System time may be incorrect.\n");
    } else {
        M5.Log.printf("[INFO] RTC Epoch: %ld\n", rtc_epoch);
        struct timeval tv = { rtc_epoch, 0 };
        if (settimeofday(&tv, nullptr) == 0) {
            M5.Log.printf("[INFO] System time set successfully from RTC.\n");
        } else {
            M5.Log.printf("[ERROR] Failed to set system time (settimeofday).\n");
        }
    }
    time_t current_system_time = time(NULL);
    M5.Log.printf("[INFO] Current system time (epoch): %ld\n", current_system_time);
    wifi_sniffer_init();
    M5.Log.printf("Setup completed. Starting RID sniffing...\n");
    dc.clearDrawingCanvas(); // 既存のメッセージをクリア
    dc.setCursor(0, dc.getRows() / 3); // 少し上に
    dc.println("Setup Complete!");
    dc.println("A: Send JSON");
    dc.println("B: Ch Lock");
    dc.println("C: Reset Device");
    dc.show();
    delay(3000); // 表示時間
}

void loop()
{
    M5.update();
    if (!displayController_ptr) return; // displayControllerが初期化失敗していたら何もしない
    M5CanvasTextDisplayController& dc = *displayController_ptr;
    // --- ボタンAが押されたらJSONデータをシリアル送信 ---
    if (M5.BtnA.wasPressed()) {
        M5.Log.println("Button A pressed. Preparing JSON data...");
        dc.clearDrawingCanvas();
        dc.setCursor(0,0);
        dc.println("BTN_A: Sending JSON...");
        dc.show();
        String json_string_to_send;
        if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
#           if SEND_MODE_TOP_RSSI == 1
                M5.Log.printf("Mode: Top RSSI, Max Entries: %u\n", MAX_ENTRIES_IN_JSON);
                json_string_to_send = dataManager.getJsonForTopRSSI(1, MAX_ENTRIES_IN_JSON);
#           else
                M5.Log.printf("Mode: Reg No '%s', Max Entries: %u\n", TARGET_REG_NO_FOR_JSON, MAX_ENTRIES_IN_JSON);
                json_string_to_send = dataManager.getJsonForRegistrationNo(String(TARGET_REG_NO_FOR_JSON), MAX_ENTRIES_IN_JSON);
#           endif
            xSemaphoreGive(dataManagerSemaphore);
            if (json_string_to_send.length() > 2 && json_string_to_send != "{}") { // "{}" は空オブジェクト
                M5.Log.printf("%s\n", json_string_to_send.c_str());
                M5.Log.printf("JSON data sent (%d bytes).\n", json_string_to_send.length());
                dc.clearDrawingCanvas();
                dc.setCursor(0,0);
                dc.println("JSON Sent via Serial!");
                dc.println("Check PC.");
                dc.show();
                delay(2500); // 送信完了メッセージ表示時間
            } else {
                M5.Log.println("No data to send or JSON generation failed (empty JSON).");
                dc.clearDrawingCanvas();
                dc.setCursor(0,0);
                dc.println("No JSON data found");
                dc.println("or generation failed.");
                dc.show();
                delay(2500);
            }
        } else {
            M5.Log.println("[ERROR] Could not obtain semaphore for JSON data generation.");
            dc.clearDrawingCanvas();
            dc.setCursor(0,0);
            dc.println("Error: Semaphore busy.");
            dc.println("Try again.");
            dc.show();
            delay(2000);
        }
    }
    // --- Bボタン: チャンネル固定モード切り替え ---
    if (M5.BtnB.wasPressed()) {
        channelLockModeActive = !channelLockModeActive;
        if (channelLockModeActive) {
            M5.Log.println("Channel lock mode ACTIVATED.");
            lockedChannel = -1; // 最初は不明
            lastChannelLockCheck = millis(); // チェックタイマーリセット
        } else {
            M5.Log.println("Channel lock mode DEACTIVATED. Resuming scan.");
            lockedChannel = -1;
            // 次の通常のチャンネル切り替えでスキャンが再開される
        }
        // 画面更新を即座に行うために、display updateのタイマーをリセットするのも良い
        // last_display_update = 0; // これにより次のループで即座に画面更新
    }
    // --- ボタンCが押されたことが認識できない ---
    if (M5.BtnPWR.wasPressed()) {
        M5.Log.println("Button C pressed. Resetting...");
        dc.clearDrawingCanvas();
        dc.setCursor(0, dc.getRows() / 2 -1);
        dc.println("Resetting...");
        dc.show();
        delay(1000); // メッセージ表示のための短い遅延
        ESP.restart();
    }
    // --- 以下、既存の定期的な画面表示ロジック ---
    static unsigned long last_display_update = 0;
    if (millis() - last_display_update > WIFI_CHANNEL_SWITCH_INTERVAL) {
        last_display_update = millis();
        // --- チャンネル制御 ---
        if (channelLockModeActive) {
            // チャンネル固定モードが有効
            if (lockedChannel == -1 || (millis() - lastChannelLockCheck > CHANNEL_LOCK_CHECK_INTERVAL)) {
                // 固定チャンネルが未設定、または定期チェックのタイミング
                lastChannelLockCheck = millis();
                int targetChannel = -1;
                if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(50)) == pdTRUE) {
#                   if SEND_MODE_TOP_RSSI == 1
                        // RemoteIDDataManagerにヘルパーがない場合はここでロジックを書く
                        std::vector<std::pair<int, String>> sorted_rids = dataManager.getSortedRIDsByRSSI();
                        if (!sorted_rids.empty()) {
                            RemoteIDEntry latest_entry;
                            if (dataManager.getLatestEntryForRID(sorted_rids[0].second, latest_entry)) {
                                targetChannel = latest_entry.channel;
                            }
                        }
#                   else
                        // RemoteIDDataManagerにヘルパーがない場合はここでロジックを書く
                        for (const auto& pair : dataManager._data_store) { // _data_storeへの直接アクセスは良くない。ヘルパー推奨
                            const auto& container = pair.second;
                            if (!container.entries.empty() && container.entries.back().registrationNo == String(TARGET_REG_NO_FOR_JSON)) {
                                targetChannel = container.entries.back().channel;
                                break;
                            }
                        }
#                   endif
                    xSemaphoreGive(dataManagerSemaphore);
                } else {
                    M5.Log.println("[WARNING] Failed to take semaphore for channel lock target check.");
                }
                if (targetChannel != -1 && targetChannel >= 1 && targetChannel <= WIFI_CHANNEL_MAX) {
                    if (lockedChannel != targetChannel) { // チャンネルが変わった場合のみ設定
                        esp_err_t err = esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);
                        if (err == ESP_OK) {
                            M5.Log.printf("Channel locked to: %d\n", targetChannel);
                            lockedChannel = targetChannel; // 現在の物理チャンネルを更新
                            channel = lockedChannel; // 表示用のグローバル `channel` 変数も更新
                        } else {
                            M5.Log.printf("[ERROR] Failed to lock channel to %d: %s\n", targetChannel, esp_err_to_name(err));
                            // lockedChannel = -1; // 設定失敗したらロック解除する？ or 前の値を維持？
                        }
                    }
                } else {
                    // ターゲットが見つからない場合、どうするか？
                    // M5.Log.println("Target for channel lock not found. Check if active.");
                    // ここでは何もしない (lockedChannelは変わらない or -1のまま)
                    // もしlockedChannelが-1のままなら、次の通常のチャンネルスキャンに進む
                    // 既存のlockedChannelがあれば、それを維持する
                }
            }
            // lockedChannelが有効なら、そのチャンネルを維持 (esp_wifi_set_channelは頻繁に呼ばない)
            // 表示用の 'channel' 変数はlockedChannelの値を使う
            if (lockedChannel != -1) {
                 channel = lockedChannel; // 表示用
            } else {
                 // ターゲットが見つからず、まだロックできていない場合は通常のチャンネルスキャンへ
                 // （または、固定モードを一旦解除するなどの挙動も考えられる）
                 // ここでは、次の通常のチャンネルスキャンロジックが実行されるようにする
                 channel = (channel % WIFI_CHANNEL_MAX) + 1;
                 esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            }
        } else {
            // 通常のチャンネルスキャンモード
            channel = (channel % WIFI_CHANNEL_MAX) + 1;
            esp_err_t err = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            if (err != ESP_OK) {
                M5.Log.printf("[ERROR] Failed to set channel to %d: %s\n", channel, esp_err_to_name(err));
            }
            lockedChannel = -1; // 固定モードではないので-1
        }
        dc.clearDrawingCanvas(); // 描画キャンバスをクリア
        dc.setCursor(0, 0);      // カーソルを左上にリセット
        int current_rid_count = 0;
        if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
            current_rid_count = dataManager.getRIDCount();
            xSemaphoreGive(dataManagerSemaphore);
        } else {
            M5.Log.println("[WARNING] Failed to take dataManagerSemaphore in loop for RID count.");
        }
        char header_buf[80];
        if (channelLockModeActive) {
            if (lockedChannel != -1) {
                snprintf(header_buf, sizeof(header_buf), "Ch:%2d(L) RIDs:%d H:%u", lockedChannel, current_rid_count, ESP.getFreeHeap());
            } else {
                snprintf(header_buf, sizeof(header_buf), "Ch:Lock? RIDs:%d H:%u", current_rid_count, ESP.getFreeHeap());
            }
        } else {
            snprintf(header_buf, sizeof(header_buf), "Ch:%2d(S) RIDs:%d H:%u", channel, current_rid_count, ESP.getFreeHeap());
        }
        dc.println(header_buf);
        String separator = "";
        for (int i = 0; i < dc.getCols(); ++i) {
            separator += "-";
        }
        dc.println(separator);
        // 表示可能なRID数を計算 (ヘッダ2行、各RID3行と仮定)
        int displayable_rids_on_screen = (dc.getRows() > 2) ? (dc.getRows() - 2) / LINES_PER_RID_ENTRY : 0;
        if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(50)) == pdTRUE) {
            std::vector<std::pair<int, String>> sorted_rids_info = dataManager.getSortedRIDsByRSSI();
            int rid_count_available = sorted_rids_info.size();
            int displayed_count = 0;
            // 実際に表示する数を決定 (利用可能なRID数と計算された最大表示数の小さい方)
            int rids_to_actually_display = min(rid_count_available, max_rids_to_display_calculated);
            if (rid_count_available > 0) {
                for (int i = 0; i < rids_to_actually_display && displayed_count < max_rids_to_display_calculated; ++i) {
                    // 次のRID情報が現在のカーソル位置から画面に収まるかチェック
                    if (dc.getPrintCursorRow() + LINES_PER_RID_ENTRY > dc.getRows()) {
                        break;
                    }
                    String current_rid_str = sorted_rids_info[i].second;
                    if (!current_rid_str.isEmpty()) {
                        RemoteIDEntry latest_entry;
                        if (dataManager.getLatestEntryForRID(current_rid_str, latest_entry)) {
                            char line_buf[128]; // 各行のバッファ
                            // Display RID (RSSI) - 画面幅に応じてRID文字列を切り詰める
                            String rid_to_display = current_rid_str;
                            // " (RSSI)" のために約13文字消費 + マージン
                            int max_rid_len = dc.getCols() - 14;
                            if (max_rid_len < 1) max_rid_len = 1;
                            if (rid_to_display.length() > (unsigned int)max_rid_len) {
                                rid_to_display = rid_to_display.substring(0, max_rid_len);
                            }
                            snprintf(line_buf, sizeof(line_buf), "%s (%d,Ch:%d)", rid_to_display.c_str(), latest_entry.rssi, latest_entry.channel);
                            dc.println(line_buf);
                            if (!latest_entry.registrationNo.isEmpty()) {  // 登録記号があれば表示
                                String reg_no_to_display = latest_entry.registrationNo;
                                // 画面幅に応じて登録記号を切り詰める (例: 最大表示文字数 getCols())
                                if (reg_no_to_display.length() > (unsigned int)dc.getCols()) {
                                    reg_no_to_display = reg_no_to_display.substring(0, dc.getCols());
                                }
                                snprintf(line_buf, sizeof(line_buf), "Reg:%.*s", dc.getCols() - 4, reg_no_to_display.c_str()); // "Reg:"の分を引く
                                dc.println(line_buf);
                            } else {  // 登録記号がない場合
                                dc.println("Reg:N/A");
                            }
                            // Display Lat/Lon
                            snprintf(line_buf, sizeof(line_buf), "L:%.3f Lo:%.3f", latest_entry.latitude, latest_entry.longitude);
                            dc.println(line_buf);
                            // Display Altitudes and Time
                            char time_str[10] = "N/A Time";
                            time_t entry_time = latest_entry.timestamp;
                            struct tm *tm_info = localtime(&entry_time);
                            if (tm_info) {
                                 snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
                            }
                            snprintf(line_buf, sizeof(line_buf), "P:%.0fm G:%.0fm %s", latest_entry.pressureAltitude, latest_entry.gpsAltitude, time_str);
                            dc.println(line_buf);
                            snprintf(line_buf, sizeof(line_buf), "BcnTS: ..%03lu.%06lu",
                                     (unsigned long)((latest_entry.beaconTimestamp / 1000000ULL) % 1000), // 秒の下3桁
                                     (unsigned long)(latest_entry.beaconTimestamp % 1000000ULL));       // マイクロ秒6桁
                            dc.println(line_buf);
                            displayed_count++;
                        }
                    }
                }
            }
            if (displayed_count == 0 && rid_count_available == 0) { // データが全くない場合
                 if (dc.getPrintCursorRow() < dc.getRows()) { // 空き行があれば表示
                    dc.println("No RID data yet.");
                 }
            } else if (displayed_count == 0 && rid_count_available > 0) { // データはあるが表示できなかった場合(行数不足など)
                if (dc.getPrintCursorRow() < dc.getRows()) {
                    dc.println("No space to show RIDs");
                }
            }
            xSemaphoreGive(dataManagerSemaphore);
        } else {
            M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in loop for display.\n");
            if (dc.getPrintCursorRow() < dc.getRows()) {
                dc.println("Failed to get data...");
            }
        }
        dc.show(); // 全ての描画が終わったら、描画キャンバスの内容をLCDに転送
    }
}
