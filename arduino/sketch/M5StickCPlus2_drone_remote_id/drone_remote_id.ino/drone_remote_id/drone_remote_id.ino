/*
 * @file drone_remote_id.ino
 * @author igapon (igapon@gmail.com)
 * @brief M5StickCPlus2 get drone remote id
 * @version 0.2 (Updated with improvements)
 * @date 2025-06-22 // 仮の日付更新
 * @Hardwares: M5StickCPlus2
 * @Platform Version: Arduino M5Stack Board Manager v2.1.4
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * M5StickCPlus2: https://github.com/m5stack/M5StickCPlus2
 * 参考: https://lang-ship.com/blog/work/esp32-wifi-sniffer/
 * 参考: https://qiita.com/kobatan/items/dac5d4696d631003e037
 * 参考: https://interface.cqpub.co.jp/wp-content/uploads/if2402_041.pdf
 */
#include "M5StickCPlus2.h"
#include <Arduino.h>
#include <string.h>
#include <ctime>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "RemoteIDDataManager.h"

#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)
#define WIFI_CHANNEL_MAX               (13) // 日本のWi-Fiチャンネルは通常1-13ch、14chは特殊。

// target_rid を空文字列にすると、特に優先するRIDはないという意味になります。
// もし特定のシリアルナンバーを優先したい場合は、ここに指定します。
RemoteIDDataManager dataManager(""); // グローバル変数としてインスタンス化など
static wifi_country_t wifi_country = {.cc = "JP", .schan = 1, .nchan = WIFI_CHANNEL_MAX};
int channel = 1;
SemaphoreHandle_t dataManagerSemaphore; // dataManagerアクセス用のセマフォ
const uint8_t ASTM_OUI[] = {0xFA, 0x0B, 0xBC};
const uint8_t ASTM_OUI_TYPE_RID = 0x0D;

#pragma pack(push,1)			// データを詰めて配置
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
    if (type != WIFI_PKT_MGMT) { // Only interested in management frames
        return;
    }
    wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t*)buf;
    wifi_mac_hdr_t *mac_hdr = (wifi_mac_hdr_t *)ppkt->payload;
    // Check for Beacon frame (Type:0, SubType:8)
    if (mac_hdr->fctl != 0x0080) {
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
                        rid_serial_from_ssid = temp_ssid.substring(4); // まず substring の結果を代入
                        rid_serial_from_ssid.trim();                  // 次に trim() を呼び出して文字列自体を変更
                    }
                    // M5.Log.printf("Found RID SSID: %s, extracted S/N: %s\n", temp_ssid.c_str(), rid_serial_from_ssid.c_str());
                    break;
                } else {
                    return; // Not an RID-prefixed SSID, ignore this packet
                }
            }
        }
        int element_total_len = sizeof(element_head_t) + current_element->len;
        if (element_total_len == 0 || element_total_len > current_remaining_len) break;
        current_remaining_len -= element_total_len;
        current_element = (element_head_t *)((uint8_t *)current_element + element_total_len);
    }
    if (!is_rid_ssid_found) {
        return; // No "RID-" SSID found
    }
    // --- Pass 2: Find Vendor Specific IE for ASTM RID ---
    current_element = element; // Reset to start of elements
    current_remaining_len = remaining_len;
    while(current_remaining_len > sizeof(element_head_t)){
        // Vendor Specific Element ID (221), OUI(3) + Type(1) = 4 bytes minimum for OUI header
        const size_t min_vendor_ie_len = sizeof(ASTM_OUI) + 1; // OUI (3) + OUI Type (1)
        if (current_element->id == 221 && current_element->len >= min_vendor_ie_len) {
            const uint8_t* vendor_payload_ptr = (const uint8_t*)current_element->payload;
            if (memcmp(vendor_payload_ptr, ASTM_OUI, sizeof(ASTM_OUI)) == 0 &&
                vendor_payload_ptr[sizeof(ASTM_OUI)] == ASTM_OUI_TYPE_RID) { // ASTM OUI and RID type
                const size_t oui_header_size = sizeof(ASTM_OUI) + 1; // OUI + OUI Type
                if (current_element->len >= oui_header_size + sizeof(RID_Data)) { // Check if enough data for RID_Data
                    RID_Data *data = (RID_Data *)(vendor_payload_ptr + oui_header_size);
                    char sn_buf[sizeof(data->serial_no) + 1];
                    memcpy(sn_buf, data->serial_no, sizeof(data->serial_no));
                    sn_buf[sizeof(data->serial_no)] = '\0'; // Ensure null termination
                    String rid_from_payload_temp = String(sn_buf);
                    rid_from_payload_temp.trim(); // trim() を適用
                    String rid_from_payload = rid_from_payload_temp;
                    if (rid_from_payload.isEmpty()) {
                        rid_from_payload = rid_serial_from_ssid; // Use SSID's RID if payload's is empty
                    }
                    if (rid_from_payload.isEmpty()) {
                        M5.Log.printf("[WARNING] RID is empty after checking payload and SSID. Skipping.\n");
                        return; // Still empty, skip
                    }
                    time_t current_time_sec = time(NULL);
                    float latitude = static_cast<float>(data->lat) * 1e-7f;
                    float longitude = static_cast<float>(data->lng) * 1e-7f;
                    float pressure_alt = static_cast<float>(data->Pressur_Altitude) * 0.1f;
                    float gps_alt = static_cast<float>(data->Geodetic_Altitude) * 0.1f;
                    if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
                        dataManager.addData(
                            rid_from_payload,
                            ppkt->rx_ctrl.rssi,
                            current_time_sec,
                            latitude,
                            longitude,
                            pressure_alt,
                            gps_alt
                        );
                        xSemaphoreGive(dataManagerSemaphore);
                        // M5.Log.printf("RID: %s, RSSI: %d, Lat: %.4f, Lon: %.4f, PAlt: %.1f, GAlt: %.1f\n",
                        //    rid_from_payload.c_str(), ppkt->rx_ctrl.rssi, latitude, longitude, pressure_alt, gps_alt);
                    } else {
                        M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in sniffer_cb\n");
                    }
                    return; // Processed ASTM RID data, exit handler for this packet
                } else {
                    M5.Log.printf("[DEBUG] Vendor IE for ASTM RID too short for RID_Data. Len: %u, Expected Min after OUI: %u\n",
                                  current_element->len, oui_header_size + sizeof(RID_Data));
                }
            }
        }
        // Move to next element
        int element_total_len = sizeof(element_head_t) + current_element->len;
        if (element_total_len == 0 || element_total_len > current_remaining_len) {
             M5.Log.printf("[ERROR] Invalid element length: %d in packet. Remaining len: %d\n", current_element->len, current_remaining_len);
             break;
        }
        current_remaining_len -= element_total_len;
        current_element = (element_head_t *)((uint8_t *)current_element + element_total_len);
    }
}

void display_init(void) {
    auto cfg = M5.config();
    // StickCP2.begin(cfg); // M5Unifiedでは M5.begin() で統合されることが多い
    M5.begin(cfg); // M5Unified の標準的な初期化
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(GREEN, BLACK); // 背景色も指定するとチラつきを抑えられる場合がある
    M5.Display.setFont(&fonts::Font0);
    M5.Display.setTextSize(1);
    M5.Display.setRotation(1); // M5StickCPlus2 は通常縦持ちなので1か3
    M5.Display.setTextWrap(false);
    M5.Display.setCursor(0, 0);
    M5.Log.printf("Display initialized.\n");
}

void wifi_sniffer_init(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  wifi_promiscuous_filter_t filter;
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT; // Only management frames
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE)); // 初期チャンネル設定
  M5.Log.printf("Wi-Fi Sniffer initialized on Channel %d.\n", channel);
}

void setup()
{
    // M5.Rtc.begin(); // RTCを使用する前に初期化が必要な場合がある (M5UnifiedではM5.begin()に含まれることが多い)
    display_init(); // M5.begin() は display_init 内でコールされる
    dataManagerSemaphore = xSemaphoreCreateMutex();
    if (dataManagerSemaphore == NULL) {
        M5.Log.printf("[FATAL] Failed to create dataManagerSemaphore!\n");
        while(1); // Halt
    }
    M5.Log.printf("[INFO] Setting up RTC and system time...\n");
    auto dt = M5.Rtc.getDateTime();
    if (dt.date.year < 2023) { // Check if RTC date is sensible
        M5.Log.printf("[INFO] RTC date is old (%04d-%02d-%02d), setting to a default time.\n",
                      dt.date.year, dt.date.month, dt.date.date);
        decltype(M5.Rtc.getDateTime()) default_datetime;
        default_datetime.date.year = 2024; // Example: Current year or compile year
        default_datetime.date.month = 1;   // Example: January
        default_datetime.date.date = 1;    // Example: 1st
        default_datetime.time.hours = 0;   // Example: Midnight
        default_datetime.time.minutes = 0;
        default_datetime.time.seconds = 0;
        M5.Rtc.setDateTime(default_datetime);
        M5.Log.printf("[INFO] RTC set to default: %04d-%02d-%02d %02d:%02d:%02d\n",
                      default_datetime.date.year, default_datetime.date.month, default_datetime.date.date,
                      default_datetime.time.hours, default_datetime.time.minutes, default_datetime.time.seconds);
        dt = M5.Rtc.getDateTime(); // Re-fetch the (now set) time
    }
    struct tm tminfo;
    tminfo.tm_year = dt.date.year - 1900;
    tminfo.tm_mon  = dt.date.month - 1;
    tminfo.tm_mday = dt.date.date;
    tminfo.tm_hour = dt.time.hours;
    tminfo.tm_min  = dt.time.minutes;
    tminfo.tm_sec  = dt.time.seconds;
    tminfo.tm_isdst = -1; // Let mktime determine DST
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
    // M5.Log.printf("[INFO] Current system time (string): %s", ctime(¤t_system_time)); // ctime adds newline
    wifi_sniffer_init(); // Moved after RTC setup for accurate timestamps from sniffer start
    M5.Log.printf("Setup completed. Starting RID sniffing...\n");
}

const int MAX_RIDS_TO_DISPLAY = 4; // 画面サイズに合わせて調整 (M5StickCPlus2では4行程度が適切か)

void loop()
{
    M5.update(); // M5Unified: Process button events, etc.
    delay(WIFI_CHANNEL_SWITCH_INTERVAL);
    channel = (channel % WIFI_CHANNEL_MAX) + 1; // Cycles 1->...->WIFI_CHANNEL_MAX->1
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
    // M5.Log.printf("Switched to Wi-Fi channel: %d\n", channel);
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);
    int current_rid_count = 0;
    if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(5)) == pdTRUE) { // Shorter timeout for count
        current_rid_count = dataManager.getRIDCount();
        xSemaphoreGive(dataManagerSemaphore);
    } else {
        M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in loop for RID count.\n");
    }
    M5.Display.printf("Ch:%2d RIDs:%d Heap:%d\n", channel, current_rid_count, ESP.getFreeHeap());
    M5.Display.println("---------------------");
    if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(50)) == pdTRUE) {
        // 最初にソート済みRIDのリストを取得 (Stringのコピーは発生する)
        std::vector<std::pair<int, String>> sorted_rids_info = dataManager.getSortedRIDsByRSSI();
        int rid_count_for_display = sorted_rids_info.size();
        int displayed_count = 0;
        if (rid_count_for_display > 0) {
            for (int i = 0; i < rid_count_for_display && displayed_count < MAX_RIDS_TO_DISPLAY; ++i) {
                String current_rid_str = sorted_rids_info[i].second; // ソート済みリストから取得
                if (!current_rid_str.isEmpty()) {
                    RemoteIDEntry latest_entry;
                    if (dataManager.getLatestEntryForRID(current_rid_str, latest_entry)) {
                        // Display RID (truncate if too long for screen)
                        M5.Display.printf("%.10s (%d)\n", current_rid_str.c_str(), latest_entry.rssi);
                        // Display Lat/Lon
                        M5.Display.printf(" L:%.3f Lo:%.3f\n", latest_entry.latitude, latest_entry.longitude);
                        // Display Altitudes
                        M5.Display.printf(" P:%.0fm G:%.0fm ", latest_entry.pressureAltitude, latest_entry.gpsAltitude);
                        // Display Time of last update
                        time_t entry_time = latest_entry.timestamp;
                        struct tm *tm_info = localtime(&entry_time);
                        if (tm_info) {
                             M5.Display.printf("%02d:%02d:%02d\n", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
                        } else {
                             M5.Display.println(" N/A Time");
                        }
                        displayed_count++;
                        if (M5.Display.getCursorY() > M5.Display.height() - M5.Display.fontHeight()) break;
                    }
                }
            }
        } else {
            M5.Display.println("No RID data yet.");
        }
        xSemaphoreGive(dataManagerSemaphore);
    } else {
        M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in loop for display.\n");
        M5.Display.println("Failed to get data...");
    }
}
