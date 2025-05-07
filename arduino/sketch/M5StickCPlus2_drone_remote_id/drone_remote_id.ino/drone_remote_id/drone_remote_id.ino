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
#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_mac.h"

#define SERIAL_NO	"XYZ09876543210"		// RIDの製造番号
#define REG_SYMBOL	"JA.JU0987654321"		// 無人航空機の登録記号

#pragma pack(push,1)			// データを詰めて配置
typedef struct{
	uint8_t ver:4;			// bit[3-0] Protocol version
	uint8_t type:4;			// bit[7-4] message type
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
	uint8_t speed_mul:1;	// bit[0] 0:x0.25, 1:x0.75
	uint8_t dir_seg:1;		// bit[1] 0:<180, 1:>=180
	uint8_t heght_type:1;	// bit[2] 0:Abave Takeoff, 1:AGL
	uint8_t resv:1;			// bit[3] reserved
	uint8_t status:4;		// bit[7-4] status
} Status_flag;

typedef struct{
	uint8_t horizontal:4;	// bit[3-0]
	uint8_t vetical:4;		// bit[7-4]
} H_V_accuracy;				// 水平垂直の正確さ

typedef struct{
	uint8_t speed:4;		// bit[3-0]
	uint8_t baro:4;			// bit[7-4]
}B_S_accuracy;				// 速度方位の正確さ

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


#define maxCh 14				 // max Channel -> US = 11, EU = 13, Japan = 14
int curChannel = 1;

const char *get_packet_name(wifi_promiscuous_pkt_type_t type)
{
  switch(type) {
  	case WIFI_PKT_MGMT: return "MGMT";
  	case WIFI_PKT_DATA: return "DATA";
	case WIFI_PKT_CTRL: return "CTRL";
  	case WIFI_PKT_MISC: return "MISC";
		default:		
			return "";
			break;
  }
}

void rx_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
	int len,i;
	char ssid[MAX_SSID_LEN+1];
    wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t*)buf;
	wifi_mac_hdr *mac =(wifi_mac_hdr *)ppkt->payload;
	element_head *e =(element_head *)mac->payload;
	vendor_ie_data_t *vi;
	if (mac->fctl != 0x0080){  // Wifiビーコンの場合は、Typeが管理フレーム（０）で、Sub Typeが（８）
        return;
    }
	// printf("PACKET TYPE = [ %s ], CHAN = %d, RSSI = %d\n", get_packet_name(type), ppkt->rx_ctrl.channel, ppkt->rx_ctrl.rssi);
	// printf("Frame Ctl = %04X\n",mac->fctl);
	// printf("Duration = %04X\n",mac->duration);
	// printf("DA    = %02X:%02X:%02X:%02X:%02X:%02X\n", mac->da[0],mac->da[1],mac->da[2],mac->da[3],mac->da[4],mac->da[5]);
	// printf("SA    = %02X:%02X:%02X:%02X:%02X:%02X\n",	mac->sa[0],mac->sa[1],mac->sa[2],mac->sa[3],mac->sa[4],mac->sa[5]);
	// printf("BSSID = %02X:%02X:%02X:%02X:%02X:%02X\n", mac->bssid[0],mac->bssid[1],mac->bssid[2],mac->bssid[3],mac->bssid[4],mac->bssid[5]);
	// printf("Seq No. = %d, Frg No. = %d\n", mac->seqctl.sequence_num, mac->seqctl.fragment_num);
	// printf("Timestamp = %d\n",mac->timestamp);
	// printf("Interval = %dTU\n", mac->interval);	
	// printf("Cpability = %04X\n", mac->cap);		
	len = ppkt->rx_ctrl.sig_len - sizeof(wifi_mac_hdr);
	// printf("Payload length = %d\n", len);
	if(len <= 0) return;
	while(len > 4){
		printf("eid = %3d len = %3d ", e->id, e->len);
		switch (e->id) {
		case 0: // SSID
			if(e->len > MAX_SSID_LEN) break;
			strncpy(ssid, (char*)e->payload, e->len);
			ssid[e->len] = 0;
			printf("SSID = %s\n", ssid);
			break;
		case 221:	// Vender Specific
			vi = (vendor_ie_data_t *)e;
            if ((vi->vendor_oui[0] != 0xFA) || (vi->vendor_oui[1] != 0x0B) || (vi->vendor_oui[2] != 0xBC) || (vi->vendor_oui_type != 0x0D)){  // OUIとOUI Typeの値は、RIDの場合、固定の数値
                return;
            }
			printf("Vendor OUI = %02X:%02X:%02X OUI Type = %2d data = ",vi->vendor_oui[0],vi->vendor_oui[1],vi->vendor_oui[2],vi->vendor_oui_type);
			for(i=0; i < vi->length; i++){
				printf("%02X ",vi->payload[i]);
			}
			printf("\n");
			{
			    RID_Data *data = (RID_Data *)vi->payload;
                printf("serial=%s,reg=%s,lat=%d,lng=%d\n", data->serial_no, data->reg_no, data->lat, data->lng);
			}
			break;
        default:
            return;  // DJIドローンは、[SSID]と[Vendor Specific]の２つだけ
            printf("data = ");
            for(i=0;i < e->len; i++){
				printf("%02X ", e->payload[i]);
			}
			printf("\n");
			break;
		}
		int dlen = 2 + e->len;
		len -= dlen;
		e = (element_head *)((uint8_t *)e + dlen);
	}
	uint8_t *fcs = (uint8_t *)e;
	printf("FCS = %02X %02X %02X %02X\n\n",fcs[0],fcs[1],fcs[2],fcs[3]);
	WiFi.scanDelete();	// 最後のスキャン結果をメモリから削除します。
}

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
    // WiFi.disconnect();
    esp_wifi_set_promiscuous(true);		        // プロミスキャスモード（無差別モード）に設定する。
    esp_wifi_set_promiscuous_rx_cb(&rx_callback);	// 受信したときの割り込み先設定
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(100);
    StickCP2.Display.println("Setup done");
}

void loop()
{
    // int n = WiFi.scanNetworks();
    // int vol = StickCP2.Power.getBatteryVoltage();
    // StickCP2.Display.clear();
    // StickCP2.Display.setCursor(0, 0);
    // StickCP2.Display.printf("BAT: %dmv,", vol);
    // if (n == 0) {
    //     StickCP2.Display.println("no networks found");
    // } else {
    //     StickCP2.Display.print(n);
    //     StickCP2.Display.println(" networks found");
    //     // StickCP2.Display.println("No|RSSI|CH|Encryption|SSID|");
    //     StickCP2.Display.println("|RSSI|CH|Encrypt|SSID|");
    //     for (int i = 0; i < n; ++i) {
    //         if (i >= 6) {
    //             break;
    //         }
    //         // Print SSID and RSSI for each network found
    //         // StickCP2.Display.printf("%2d",i + 1);
    //         // StickCP2.Display.print("|");
    //         StickCP2.Display.printf("%4d", WiFi.RSSI(i));
    //         StickCP2.Display.print("|");
    //         StickCP2.Display.printf("%2d", WiFi.channel(i));
    //         StickCP2.Display.print("|");
    //         switch (WiFi.encryptionType(i))
    //         {
    //         case WIFI_AUTH_OPEN:
    //             StickCP2.Display.print("open");
    //             break;
    //         case WIFI_AUTH_WEP:
    //             StickCP2.Display.print("WEP");
    //             break;
    //         case WIFI_AUTH_WPA_PSK:
    //             StickCP2.Display.print("WPA");
    //             break;
    //         case WIFI_AUTH_WPA2_PSK:
    //             StickCP2.Display.print("WPA2");
    //             break;
    //         case WIFI_AUTH_WPA_WPA2_PSK:
    //             StickCP2.Display.print("WPA+WPA2");
    //             break;
    //         case WIFI_AUTH_WPA2_ENTERPRISE:
    //             StickCP2.Display.print("WPA2-EAP");
    //             break;
    //         case WIFI_AUTH_WPA3_PSK:
    //             StickCP2.Display.print("WPA3");
    //             break;
    //         case WIFI_AUTH_WPA2_WPA3_PSK:
    //             StickCP2.Display.print("WPA2+WPA3");
    //             break;
    //         case WIFI_AUTH_WAPI_PSK:
    //             StickCP2.Display.print("WAPI");
    //             break;
    //         default:
    //             StickCP2.Display.print("unknown");
    //         }
    //         StickCP2.Display.print("|");
    //         // StickCP2.Display.printf("%-32.32s", WiFi.SSID(i).c_str());
    //         StickCP2.Display.printf("%s", WiFi.SSID(i).c_str());
    //         StickCP2.Display.println("");
    //     }
    // }
    // // StickCP2.Display.println("");
    // // Delete the scan result to free memory for code below.
    // WiFi.scanDelete();
    // // Wait a bit before scanning again.
    // delay(5000);
    
    int vol = StickCP2.Power.getBatteryVoltage();
    StickCP2.Display.clear();
    StickCP2.Display.setCursor(0, 0);
    StickCP2.Display.printf("BAT: %dmv,", vol);
    ESP_ERROR_CHECK(esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE));
    Serial.println("Changed channel:" + String(curChannel));	
    delay(1000);
    curChannel++;	
    if(curChannel > maxCh) curChannel = 1;
}
