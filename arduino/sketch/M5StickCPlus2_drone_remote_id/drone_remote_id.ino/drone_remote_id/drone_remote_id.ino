/*
 * @file drone_remote_id.ino
 * @author igapon (igapon@gmail.com)
 * @brief M5StickCPlus2 get drone remote id
 * @version 0.1
 * @date 2025-06-33
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
#include "esp_wifi.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include <time.h>
#include "RemoteIDDataManager.h"

#define SERIAL_NO	"XYZ09876543210"	// RIDの製造番号
#define REG_SYMBOL	"JA.JU0987654321"	// 無人航空機の登録記号
#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)
#define WIFI_CHANNEL_MAX               (14)

// target_rid を空文字列にすると、特に優先するRIDはないという意味になります。
// もし特定のシリアルナンバーを優先したい場合は、ここに指定します。
RemoteIDDataManager dataManager(""); // グローバル変数としてインスタンス化など
static wifi_country_t wifi_country = {.cc = "JP", .schan = 1, .nchan = 14};  // Most recent esp32 library struct
int channel = 1;
SemaphoreHandle_t dataManagerSemaphore; // dataManagerアクセス用のセマフォ

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
} wifi_mac_hdr;

typedef struct{
	uint8_t id;					// Element ID
	uint8_t len;				// Length
	uint8_t payload[0];
} element_head;

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
	char serial_no[20] = SERIAL_NO;	// 製造番号
	uint8_t resv1[3];
	//--- Basic ID (25byte)--------------------------
	Message_head msg2;				// BASIC_ID
	uint8_t UA_type2:4;				// bit[3-0] 機体種別
	uint8_t ID_type2:4;				// bit[7-4] ID_TYPE_ASSIGED_REG
	char reg_no[20] = REG_SYMBOL;	// 登録記号 (例： JA.JU012345ABCDE)
	uint8_t resv2[3];
	//--- Location (25byte)--------------------------
	Message_head msg3;				// Location
	Status_flag status;				// 飛行中、方角E/W、速度倍率などの状態
	uint8_t dir;					// 方角
	uint8_t speed;					// 速度
	uint8_t Ver_speed;				// 垂直速度
	uint32_t lat;					// 緯度
	uint32_t lng;					// 経度
	uint16_t Pressur_Altitude;  	// 気圧高度
	uint16_t Geodetic_Altitude; 	// GPS高度
	uint16_t Height;				// 地面からの高さ
	H_V_accuracy hv_Accuracy;		// 水平垂直精度
	B_S_accuracy bs_Accuracy;		// 方位・速度精度
	uint16_t timestamp;				// 現在時刻の分以下の小数点１までの秒数ｘ10
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

const char *get_packet_name(wifi_promiscuous_pkt_type_t type)
{
  switch(type) {
  	case WIFI_PKT_MGMT: return "MGMT";
  	case WIFI_PKT_DATA: return "DATA";
	case WIFI_PKT_CTRL: return "CTRL";
  	case WIFI_PKT_MISC: return "MISC";
	default: return "";
	break;
  }
}

void wifi_sniffer_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
	int len,i;
	char ssid_str[MAX_SSID_LEN + 1];
    wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t*)buf;
	wifi_mac_hdr *mac =(wifi_mac_hdr *)ppkt->payload;
	if (mac->fctl != 0x0080){  // Wifiビーコンの場合は、Typeが管理フレーム（０）で、Sub Typeが（８）
        return;
    }
	element_head *e =(element_head *)mac->payload;
	len = ppkt->rx_ctrl.sig_len - sizeof(wifi_mac_hdr);
	if(len <= 0) return;
	bool is_rid_ssid = false;
	String rid_serial_from_ssid = ""; // SSIDから取得するシリアル番号 (例: "RID-SERIAL123" の "SERIAL123" 部分)
	// まずSSID要素を探す
	element_head *current_e = e;
	int remaining_len = len;
	while(remaining_len > sizeof(element_head)){ // 要素ヘッダサイズ以上残っているか
        if (current_e->id == 0) { // SSID Element ID
            if (current_e->len > 0 && current_e->len <= 32) {
                strncpy(ssid_str, (char*)current_e->payload, current_e->len);
                ssid_str[current_e->len] = 0;
                String temp_ssid = String(ssid_str);
                if (temp_ssid.startsWith("RID-")) {
                        is_rid_ssid = true;
                        // SSIDからシリアル番号を抽出 (例: "RID-SERIAL123" -> "SERIAL123")
                        // 実際のSSIDフォーマットに合わせて調整してください。
                        if (temp_ssid.length() > 4) {
                                rid_serial_from_ssid = temp_ssid.substring(4);
                        }
                        // M5.Log.printf("Found RID SSID: %s\n", ssid_str); // デバッグ用
                        break; // SSIDが見つかったら一旦ループを抜ける（後でVender Specificを探すため）
                } else {
                        return; // RID-で始まらないSSIDならこのパケットは無視
                }
            }
        }
        int dlen = sizeof(element_head) + current_e->len;
        if (dlen == 0 || dlen > remaining_len) break; // 無限ループや範囲外アクセス防止
        remaining_len -= dlen;
        current_e = (element_head *)((uint8_t *)current_e + dlen);
	}
	if (!is_rid_ssid) { // SSIDが "RID-" で始まらなかった場合
			return;
	}
	// 次にVendor Specific要素を探す
	current_e = e; // 最初から探し直し
	remaining_len = len;
	while(remaining_len > sizeof(element_head)){
        if (current_e->id == 221) { // Vendor Specific Element ID
            vendor_ie_data_t *vi = (vendor_ie_data_t *)current_e;
            if ((vi->vendor_oui[0] == 0xFA) &&
             (vi->vendor_oui[1] == 0x0B) &&
             (vi->vendor_oui[2] == 0xBC) &&
             (vi->vendor_oui_type == 0x0D)) { // ASTM OUI and RID type
                RID_Data *data = (RID_Data *)vi->payload;
                // データをRemoteIDDataManagerに追加
                // RIDとしては、ペイロード内のserial_noを使用するのが一般的
                String rid_from_payload = String(data->serial_no);
                // SSID由来のRIDとペイロード由来のRIDが異なる場合、どちらを優先するか、
                // あるいは両方記録するかなどのポリシーが必要。ここではペイロード優先。
                if (rid_from_payload.isEmpty()) {
                    rid_from_payload = rid_serial_from_ssid; // ペイロードが空ならSSIDから
                }
                if (rid_from_payload.isEmpty()) { // それでも空ならスキップ
                    M5.Log.printf("[WARNING] RID is empty. Skipping.\n");
                    return;
                }
                time_t current_time = time(NULL); // 現在時刻を取得 (RTC設定が必要)
                // M5.Rtc.getDateTime() などを使っても良い
                // 緯度経度は1e-7単位、高度は0.1m単位なので変換
                float latitude = (float)data->lat * 1e-7f;
                float longitude = (float)data->lng * 1e-7f;
                float pressure_alt = (float)data->Pressur_Altitude * 0.1f;
                float gps_alt = (float)data->Geodetic_Altitude * 0.1f;
                // セマフォを取得してdataManagerを保護
                if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) { // タイムアウトを短めに設定
                    dataManager.addData(
                        rid_from_payload,
                        ppkt->rx_ctrl.rssi,
                        current_time,
                        latitude,
                        longitude,
                        pressure_alt,
                        gps_alt
                    );
                    xSemaphoreGive(dataManagerSemaphore);
                    // M5.Log.printf("Data added for RID: %s, RSSI: %d\n", rid_from_payload.c_str(), ppkt->rx_ctrl.rssi); // デバッグ用
                } else {
                    M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in sniffer_cb\n");
                }
                return; // Vendor Specific処理完了
            }
        }
        int dlen = sizeof(element_head) + current_e->len;
        if (dlen == 0 || dlen > remaining_len) break;
        remaining_len -= dlen;
        current_e = (element_head *)((uint8_t *)current_e + dlen);
	}
}

void display_init(void) {
    auto cfg = M5.config();
    StickCP2.begin(cfg);
    M5.Display.fillScreen(BLACK); // M5Unifiedでは M5.Display を使う
    M5.Display.setTextColor(GREEN);
    M5.Display.setFont(&fonts::Font0); // 適切なフォントを選択
    M5.Display.setTextSize(1);
    M5.Display.setRotation(1);
    M5.Display.setTextWrap(false); // 右端で折り返さないようにする（手動で制御するため）
    M5.Display.setCursor(0, 0);
    M5.Log.printf("Display initialized.\n"); // printf で出力
}

void wifi_sniffer_init(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) ); /* set country for channel range [1, 13] */
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  // 全ての管理フレームをキャプチャするフィルター (ビーコン以外も含む場合)
  // wifi_promiscuous_filter_t filter = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT};
  // または、特定のサブタイプのみをフィルタリングすることも検討可能だが、まずはMGMT全体で。
  // Beaconフレームのみに限定する場合、より細かいフィルタリングが必要になることがある
  wifi_promiscuous_filter_t filter;
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT; // 管理フレームのみ
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  M5.Log.printf("Wi-Fi Sniffer initialized.\n");
}

void setup()
{
    display_init();
    wifi_sniffer_init();
    dataManagerSemaphore = xSemaphoreCreateMutex(); // セマフォを作成
    // RTCの時刻設定
    M5.Log.printf("[INFO] Setting up RTC and system time...\n");
    auto dt = M5.Rtc.getDateTime(); // dt の型はコンパイラによって推論される (おそらく lgfx::v1::lgfx_date_t やそれに類する型)
                                    // もしくは、M5Unified.h で定義された datetime_t
    if (dt.date.year < 2023) {
        M5.Log.printf("[INFO] RTC date is old (%04d-%02d-%02d), attempting to set to a default time.\n",
                      dt.date.year, dt.date.month, dt.date.date);
        M5.Log.printf("[INFO] Setting a default time for RTC.\n");
        // M5.Rtc.getDateTime() と同じ型を使用する
        // M5Unified.h や M5GFX.h をインクルードしていれば、
        // 以下のいずれかで datetime_t が利用できるはず。
        // まずは名前空間なしで試す。
        decltype(M5.Rtc.getDateTime()) default_datetime; // ★★★ dt と同じ型を使用 ★★★
        // もし上記でエラーが出る場合は、M5Unified.h や M5GFX.h の中で
        // datetime_t がどのように定義されているか確認が必要。
        // 例: lgfx::datetime_t や m5gfx::datetime_t など
        default_datetime.date.year = 2024;
        default_datetime.date.month = 5;
        default_datetime.date.date = 30;
        default_datetime.time.hours = 20;
        default_datetime.time.minutes = 50;
        default_datetime.time.seconds = 0;
        M5.Rtc.setDateTime(default_datetime);
        M5.Log.printf("[INFO] RTC set to default: %04d-%02d-%02d %02d:%02d:%02d\n",
                      default_datetime.date.year, default_datetime.date.month, default_datetime.date.date,
                      default_datetime.time.hours, default_datetime.time.minutes, default_datetime.time.seconds);
        dt = M5.Rtc.getDateTime(); // 設定後の時刻を再取得
    }
    // M5.Rtc.getDateTime() から time_t (UNIXエポック秒) を生成
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
    }
    struct timeval tv = { rtc_epoch, 0 };
    if (settimeofday(&tv, nullptr) == 0) {
        M5.Log.printf("[INFO] System time set successfully from RTC.\n");
    } else {
        M5.Log.printf("[ERROR] Failed to set system time (settimeofday).\n");
    }
    time_t current_system_time = time(NULL);
    M5.Log.printf("[INFO] Current system time (epoch): %ld\n", current_system_time);
    // M5.Log.printf("[INFO] Current system time (string): %s", ctime(¤t_system_time));
    M5.Log.printf("Setup completed. Starting RID sniffing...\n");
}

// 表示するRIDの最大数
const int MAX_RIDS_TO_DISPLAY = 5; // 画面サイズに合わせて調整

void loop()
{
    delay(WIFI_CHANNEL_SWITCH_INTERVAL); // チャンネル切り替え間隔
    // 次のチャンネルに切り替え
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
    // M5.Log.printf("Switched to Wi-Fi channel: %d\n", channel);
    channel = (channel % WIFI_CHANNEL_MAX) + 1;
    if (channel == 0) channel = 1; // 0チャンネルは無いため1から
    // 画面表示更新
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.printf("Ch: %2d | RIDs: %d | heep: %d\n", channel, dataManager.getRIDCount(), ESP.getFreeHeap()); // 現在のチャンネルと検出RID数を表示
    M5.Display.println("---------------------");
    // セマフォを取得してdataManagerからデータを安全に読み出す
    if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(50)) == pdTRUE) {
        int rid_count = dataManager.getRIDCount();
        int display_count = 0;
        if (rid_count > 0) {
            // RSSI降順でソートされたRIDのインデックスを取得して表示
            for (int i = 0; i < rid_count && display_count < MAX_RIDS_TO_DISPLAY; ++i) {
                String current_rid_str = dataManager.getRIDStringByIndex(i);
                if (!current_rid_str.isEmpty()) {
                    RemoteIDEntry latest_entry;
                    if (dataManager.getLatestEntryForRID(current_rid_str, latest_entry)) {
                        M5.Display.printf("%s (%d)\n", current_rid_str.substring(0,10).c_str(), latest_entry.rssi); // RID (先頭10文字)とRSSI
                        // 緯度経度なども表示する場合
                        M5.Display.printf("  Lat:%.4f Lon:%.4f\n", latest_entry.latitude, latest_entry.longitude);
                        M5.Display.printf("  PAlt:%.1fm GAlt:%.1fm\n", latest_entry.pressureAltitude, latest_entry.gpsAltitude);
                        time_t entry_time = latest_entry.timestamp;
                        struct tm *tm_info = localtime(&entry_time);
                        char time_buf[20];
                        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);
                        M5.Display.printf("  Time: %s\n", time_buf);
                        display_count++;
                        if (M5.Display.getCursorY() > M5.Display.height() - M5.Display.fontHeight()*2) break; // 画面からはみ出そうなら終了
                    }
                }
            }
        } else {
            M5.Display.println("No RID data yet.");
        }
        // セマフォを解放
        xSemaphoreGive(dataManagerSemaphore);
    } else {
        M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in loop for display.\n");
        M5.Display.println("Failed to get data...");
    }
    M5.update(); // ボタンの状態更新
}
