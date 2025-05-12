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
#include "esp_event_loop.h"
#include "nvs_flash.h"

#define SERIAL_NO	"XYZ09876543210"	// RIDの製造番号
#define REG_SYMBOL	"JA.JU0987654321"	// 無人航空機の登録記号
#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)
#define WIFI_CHANNEL_MAX               (14)
#define MAX_STACK_SIZE 10       // スタックの最大サイズ
#define MAX_MESSAGE_LENGTH 100  // メッセージの最大長

// メモリ描画領域表示（スプライト）のインスタンスを作成(必要に応じて複数作成)
// M5Canvas canvas(&M5.Lcd);  // &M5.Lcd を &StickCP2.Display と書いても同じ
static wifi_country_t wifi_country = {.cc = "JP", .schan = 1, .nchan = 14};  // Most recent esp32 library struct
int channel = 1;
char messageStack[MAX_STACK_SIZE][MAX_MESSAGE_LENGTH]; // メッセージスタック
int stackTop = -1; // スタックポインタ
SemaphoreHandle_t stackSemaphore; // スタックアクセス用のセマフォ

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

esp_err_t event_handler(void *ctx, system_event_t *event) {
  return ESP_OK;
}

void wifi_sniffer_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
	int len,i;
	char ssid[MAX_SSID_LEN+1];
    wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t*)buf;
	wifi_mac_hdr *mac =(wifi_mac_hdr *)ppkt->payload;
	if (mac->fctl != 0x0080){  // Wifiビーコンの場合は、Typeが管理フレーム（０）で、Sub Typeが（８）
        return;
    }
	element_head *e =(element_head *)mac->payload;
	vendor_ie_data_t *vi;
	len = ppkt->rx_ctrl.sig_len - sizeof(wifi_mac_hdr);
	if(len <= 0) return;
	while(len > 4){
		switch (e->id) {
		case 0: // SSID
			if (e->len > MAX_SSID_LEN) break;
			strncpy(ssid, (char*)e->payload, e->len);
			if (strncmp(ssid, "RID-", 4) != 0){  // SSIDの先頭文字がRID-で無ければ処理終了
			    return;
			}
			ssid[e->len] = 0;
//			printf("RSSI=%3d,Ch=%2d,PACKET TYPE=%s,SSID=%s\n", ppkt->rx_ctrl.rssi, ppkt->rx_ctrl.channel, get_packet_name(type), ssid);
			break;
		case 221:	// Vender Specific
			vi = (vendor_ie_data_t *)e;
            if ((vi->vendor_oui[0] != 0xFA)
             || (vi->vendor_oui[1] != 0x0B)
             || (vi->vendor_oui[2] != 0xBC)
             || (vi->vendor_oui_type != 0x0D)){  // OUIとOUI Typeの値は、RIDの場合、固定の数値
                return;
            }
			{
			    RID_Data *data = (RID_Data *)vi->payload;
			    char message[MAX_MESSAGE_LENGTH];
			    snprintf(message, MAX_MESSAGE_LENGTH, "serial=%s,reg=%s,lat=%d,lng=%d,p-alt=%d,g-alt=%d",
			     data->serial_no, data->reg_no, data->lat, data->lng, data->Pressur_Altitude, data->Geodetic_Altitude);
                // セマフォを取得
                if (xSemaphoreTake(stackSemaphore, portMAX_DELAY) == pdTRUE) {
                    // スタックにメッセージを追加
                    if (stackTop < MAX_STACK_SIZE - 1) {
                        stackTop++;
                        strcpy(messageStack[stackTop], message);
                    } else {
                        // スタックがいっぱいの場合、古いメッセージを削除
                        for (int i = 0; i < MAX_STACK_SIZE - 1; i++) {
                            strcpy(messageStack[i], messageStack[i + 1]);
                        }
                        strcpy(messageStack[MAX_STACK_SIZE - 1], message);
                    }
                    // セマフォを解放
                    xSemaphoreGive(stackSemaphore);
                }
//                printf("serial=%s,reg=%s,lat=%d,lng=%d,p-alt=%d,g-alt=%d\n",
//                 data->serial_no, data->reg_no, data->lat, data->lng, data->Pressur_Altitude, data->Geodetic_Altitude);
			}
			break;
        default:
            return;  // DJIドローンは、[SSID]と[Vendor Specific]の２つだけ
		}
		int dlen = 2 + e->len;
		len -= dlen;
		e = (element_head *)((uint8_t *)e + dlen);
	}
}

void display_init(void) {
    auto cfg = M5.config();
    StickCP2.begin(cfg);
    StickCP2.Display.fillScreen(BLACK); // 背景色
    StickCP2.Display.setTextColor(GREEN);  //(文字色, 背景)
    StickCP2.Display.setFont(&fonts::Font0);
    StickCP2.Display.setTextSize(1);
    StickCP2.Display.setCursor(0, 0);
}

void wifi_sniffer_init(void) {
  nvs_flash_init();
  tcpip_adapter_init();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) ); /* set country for channel range [1, 13] */
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  wifi_promiscuous_filter_t filter = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT};
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
}

void setup()
{
//     Serial.begin(115200);
//     delay(10)
    display_init();
    wifi_sniffer_init();
    stackSemaphore = xSemaphoreCreateMutex();
}

void loop()
{
    delay(WIFI_CHANNEL_SWITCH_INTERVAL);
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
//    Serial.println("Changed channel:" + String(channel));
    channel = (channel % WIFI_CHANNEL_MAX) + 1;
    // セマフォを取得
    if (xSemaphoreTake(stackSemaphore, portMAX_DELAY) == pdTRUE) {
        // スタックからメッセージを取得して表示
        if (stackTop >= 0) {
            for(int i=0; i <= stackTop; i++){
                StickCP2.Display.println(messageStack[i]);
            }
            stackTop = -1; // スタックをクリア
        }
        // セマフォを解放
        xSemaphoreGive(stackSemaphore);
    }
}
