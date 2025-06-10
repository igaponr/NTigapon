/**
 * @file drone_remote_id.ino
 * @author igapon (igapon@gmail.com)
 * @brief M5StickCPlus2を使用してドローンのリモートID情報を取得し表示するプログラム
 * @version 0.3 (M5CanvasTextDisplayControllerで更新)
 * @date 2025-06-22
 * @Hardwares M5StickCPlus2
 * @Platform Version Arduino M5Stack Board Manager v2.1.4
 * @Dependent Library
 *  - M5GFX: https://github.com/m5stack/M5GFX
 *  - M5Unified: https://github.com/m5stack/M5Unified
 *  - M5StickCPlus2: https://github.com/m5stack/M5StickCPlus2
 * @note このプログラムはWi-Fi Beaconフレームをスキャンし、ASTM F3411-19規格に準拠した
 *       リモートID (RID) データを抽出・表示します。
 *       取得したデータはシリアル経由でJSON形式で出力することも可能です
 *       PlantUML Class図: https://www.plantuml.com/plantuml/uml/bHbjRzku-Rj_1US15pjpSjTkEmn65rmQ9sss92rYNtl0C0YqzDZcbIPr91N7Atg_VNn8cQPCMTVcGqpnUN-dg9-qeSfKQzxxard16eWkQGwSpdizmmm7SgcAYWb97c4j3P3R2NclvA-G6t96hZZLcamONIgwxlKivIUw17B_zwcnopisr-HpZzY_717j_fkRNyYhLnQWgDfUN7Ye-_iBqM0UfQ66IJ6m4iq_CaDKlJ6Cy3QiCAl1a8o9HOhsLy2MAzC2M33Efxa2y7nV_6WPvv9B5NDTHjAHXTSWPlEPphLHD3TVoFdvEVd-U_BEh42HDPYLBBJ3_D9hc7j5nHFLnomE20Fdc5EoN3p7AkKAg85fgPY1GQsujtv4Te_UQ5HuBYMFY0lW48XZhcMbLtkUVtrfcJoFo6wvFOAwK_MKw5ADI1TbSBcdh6g4rEKQp0oUpPJzpwb2j5sqm6-aC0w-i8iOZizl6q57bnmWV4H_fV1Om8JQ_7SI2borg725gk-JPx5qjen8s9dxdQycFVJL8-IcDHpxvMQ_jF4_xjL2qStKA2QMWrmAROX_-8vezziPky3WQaKZfIl1hcG1xQIZkbJI-6WmLvNIKdKashUKrfVGONPDAUamtjQuu2BWUmip2KnWLrEOHKGkQfpnJe1_hka7bOQ2pBTa2efHFhBrUtudbyDOiQafhieoqQklJJ4UFq5kf7erk-WJgX7rWzkmAirWNN9geA6l9ohWjmeYcWa-TvEiQVdgtwCFWUIUbjq49MLMhHUZsSj0yvwouuePjeRCEDIPNSuGSLl29MTKMymqKF_IKbYdM9y47X5aF9vi1Lsp71ycCg_M83ptUgCPj5OE-F_T7EUI9moFNqbxgHJT1bBt55F6DbrLsiWrLWJQaxHxrrAOjcxRNabXbEGSLDBtZsBwOV0TAJmWBfFVkmbZr3aicHWmmGoZVDTbNGySaNhtZWdug6ZfHyU8c5st6eU-DhADBTcgoNVJCepAT0WrHhewn1-HGavNSkEf6jJfVD11_IPLYcu5RJmcziQmYZiXMlfjbFcAgjD0bpQubB8nHx69358Tq6OqB3C9SYAs0zKYaRm30JlO4_OhNCofXWxSGbPp3dlq0dAsflmzfpdeutHST9kNeji-FoGdTP-NJw2uf8Kc2wbsAB6ZRS071vxVfUzW4t8uniWvK3LHT6FLgKzPm_OpOTvCy2DOHqTxIXxZlLvsOeMJvqQd1rU4MEeZ43UPVFfPyZ_4iOnQSKA9jKCxpyHdPpWVdwW1SW_hEIXjj_PGal6y61FVp-BjtCNZnPY4KsKAVNa4IdF3di03Zg0KFkgTECfwF9WR94kURfhQFmbWV_nCGE7WcK1ue-d6aBJzDWZdomvWsSY5H6ZPI8C0JcE8fwt36FeN2Uz2Y8i8lGYv0Xf3IMgOgyHBKJo21fDMP5DBT_3Cnwxp1MRHcvKsgifD_J9wExaMHcqRirLfpGxp68yhsj2rzRe_kJG69Ssbc0KCvzyV_nRZe39203zags39j56kFXxakEvz4GAt85CLC2OBBgc98L8iZu1A1LfN2YxvCU9bgOy2cvu93auXgfvNHdkdk64_qtlxHzPiZuE6edsD3Zz2wshn8xGdh4iQbilTSXZ7y8HCOC44aCVRoOGQYaSOQfyLaPNHh012HK7ci90AWjQ8U4y5NOAA4Y5b4US22ACOM9kYWtJ32nSHyfe-PpYjqfc5be8sMKYj5DIHOOcA0ojf6WcAOqIBmeV7-xgXx7VEdkqmkMX8VTJqGnlyS5nJXSsU5GVA8CL0iI89Vl35AAskqV6Y6ZLgQPIKqDUcs2NV0vRX8HchjnFzaPaLMd4hxgWszqnK1WPvfHG8WmhkbCPc4x-aUHqkcWmlEKTUDr9Prgr-mImAQOVnJJcd6TZG-4fMmhGFOvJzUdih2dWUCFoFED_AtwlQoYRjO2kgsorD6hrZV-UgmGdky14WVDnCxZQ6Vd1ubm-Iirrd80uIOIgLWGBJuVNsSJgz7HpwRFyMxDAo3lw1y_35q4gSoTBno35GpdK-x5mkqz2dFequFJQoVz3MPh-PjG_1bQ-oIDiqeHBaHA5cZt3gUJRVS0JqdILJhCYUAAyWqVgCVDQ66fP_8RFBnpVNiyp6ARk__2MRNCukT-xmHfD_aXSlV_YXZVRTxEtrOpVfZqSe7wwp-zk7dsVNsUr3Dhs-UlSmcTR4lZbuuiQXY1gQQIELjITSkNZ3Had3lo0dPBPYcWYv8GeMw2GZYLa1WMS3Ib3knrbmgoLua6IDywyVUVW1j862J1skzNEVQCWnLcG52hxwICVqZ2hV30v3p8gs4rmfome77QQj7q-_-kkLSNJF4kgwRNGDqokywISm-Ug6SMZUb1LA2iYK8yzOSSw4TDSojq9w4dijunUDs_GtNCuf9n-eOdJEGOy3v7TIxEVUk6qOngY7jofPQTIu-yRcj5tK56nIh5oAXELRAWgE2NL2RXJ0u-pTb9Xj2H67vB3fjqEwRjY2PTXxqDcD9ySMT9cvJMIhprSH2_lA7PvkAk4oHu-RxwjLsUYqNChcXcEj1LiiG6KOIZ3PoXcb1lDgCIPFaXMd8sTKhLsfv9hfl9ALpifFnYcMcQDCcM1s_CN9_1Pu2QfE-zqrB1DTkWpZZF_pxh1u3rhJ9MGhe4KAdS9lMMvu0l1orZJFLeNAJ0e73dXXwTYsmQU66clxWjDb2doRVSXedbUAvjiK-ZgRTa2nss4QryMLtcQ7cWcqhWyfA1Le44Qx9hIcJ8GBR-tmoG3hReWN8B1Fcbkpk_opBLMxKtgbGNTNHw16z9RYQx2dN4kI-yQfoKTsTiE8Yp3UwdEvPBbZi_8PuTsW0uF1cgfFKDY0cPNr4jLMNTzVT_tMMtUYKKRZDizVF56bKP2NwpFALVHUnnD_vtz21gY7MLMw_Xem94oZDmhaXAekBITF1w9mSS7kTs8JyyowWLDNVIjMQjpfTRgGI4lJVz6lFw2ElTS72XR3Fo0yHSAN_H0XI-lVsFNtSYDqDxLj61VrQF0olmRVNJa55PyOhVrpNj_gEVSKCA-MG_H6Iq8uqcI8Oi4pe-FHIzQqFBSyfapaq00mJIXPk_F8YBpdL9YVx-_m5h7Ys3aMbft1GiZbkfIQOIYGqVLpoLdE3D-sZWdlpKF5-cJSa3smMX5tr1bQbc_b1fvG2rze9Te0V47Osee2LBEFvlQ9LAI_I_m-KL62e6PtO4vSUg5TAALW2d8J6U0TMqivqLu3ruO9Mn01K40nifpc5NO60yBZEX-H0d9EbPE8ljELBRYSWJ0eUsRnIWLFJ5QQz8_wXVHRgpjrruPgfwjmfncKy20tX9bTVEOmxdLAEU1uw7zlEHFeY64lPUKJjNb02GbRKaNN8wB0L4eCK-AqAf3g4snhWIFYEEWT2tsSH_Z-5zXu4XUqJhBtb7qZaVzAvqcE8OMFRTreqP7lQp3zbxGQiPV4pWNrj9z48ym09eYxk4b94xjwJIP9d0xWANqlp4i4Tewru7-hew4Y1z_hIgE61rmIvwIq1nHf2_L39-NSy5dNUMZNzPiSmZnfIAfD7U7QVKOS7l5B86J2joE-9KqUsDLozxMKvC5pmnus_ZTALYN6Pe4ud2fcjcGWI-n1b5i-kh8_56-aek-jP_zPiQ80yTz2RiHPz0xHw_q4egZM_Fy0
 * 参考:
 *  - ESP32 Wi-Fi Sniffer: https://lang-ship.com/blog/work/esp32-wifi-sniffer/
 *  - M5StackでWi-Fiパケットキャプチャ: https://qiita.com/kobatan/items/dac5d4696d631003e037
 *  - Interface 2024年2月号 特集「ドローン新技術トライアル」内記事: https://interface.cqpub.co.jp/wp-content/uploads/if2402_041.pdf (主にリモートIDのデータ構造)
 */
#include <Arduino.h>
#include <string.h>
#include <ctime>
#include <M5Unified.h>
#include "esp_wifi.h"          // ESP-IDF Wi-Fi Library
#include "esp_mac.h"           // ESP-IDF MAC Address Utilities
#include "nvs_flash.h"         // ESP-IDF Non-Volatile Storage (未使用だが標準的にインクルードされることあり)
#include "RemoteIDDataManager.h" // カスタムクラス: リモートIDデータを管理
#include "M5CanvasTextDisplayController.h" // カスタムクラス: M5GFXのCanvasを使ったテキスト表示制御

#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)  ///< Wi-Fiチャンネルを切り替える間隔 (ミリ秒)
#define WIFI_CHANNEL_MAX               (13)  ///< スキャンするWi-Fiチャンネルの最大数 (日本の一般的なチャンネルは1-13ch)
#define MAX_SSID_LEN                   (32)  ///< SSIDの最大長 (esp_wifi_types.hに基づく)
#define SEND_MODE_TOP_RSSI 1               ///< JSON送信モード制御フラグ。1: RSSI上位1件のデータを送信, 0: 指定登録記号のデータを送信
                                           // SEND_MODE_TOP_RSSI を 0 にすると指定登録記号モードになります

const char* TARGET_REG_NO_FOR_JSON = "JA.TEST012345"; ///< 指定登録記号モードの場合にJSON送信対象とする登録記号
const size_t MAX_ENTRIES_IN_JSON = 40; ///< 1つのRIDに対してJSONに含める履歴データの最大エントリ数 (メモリ使用量に影響)
RemoteIDDataManager dataManager(""); ///< リモートIDデータを管理するクラスのインスタンス
M5CanvasTextDisplayController* displayController_ptr = nullptr; ///< ディスプレイ表示を制御するクラスのポインタ

/** @brief Wi-Fiの国設定 (日本) */
static wifi_country_t wifi_country = {.cc = "JP", .schan = 1, .nchan = WIFI_CHANNEL_MAX};
int channel = 1; ///< 現在スキャン中のWi-Fiチャンネル
SemaphoreHandle_t dataManagerSemaphore; ///< dataManagerへのアクセスを保護するためのセマフォ
const uint8_t ASTM_OUI[] = {0xFA, 0x0B, 0xBC};      ///< ASTM規格でリモートIDに使われるOUI (Organizationally Unique Identifier)
const uint8_t ASTM_OUI_TYPE_RID = 0x0D;           ///< ASTM OUI内のリモートIDを示すタイプ値
const int HEADER_LINES = 2;         ///< 画面表示のヘッダ情報が使用する行数 (例: "Ch: RIDs: Heap:", "-------")
const int LINES_PER_RID_ENTRY = 5;  ///< 1つのRID情報を表示するために必要な行数
int max_rids_to_display_calculated = 0; ///< 画面に表示可能な最大RIDエントリ数 (setup時に計算)

// --- チャンネル固定モード用変数 ---
bool channelLockModeActive = false; ///< チャンネル固定モードが有効かどうかのフラグ
int lockedChannel = -1;             ///< 固定中のチャンネル番号 (-1の場合は固定されていない)
unsigned long lastChannelLockCheck = 0; ///< チャンネル固定モード時にターゲットチャンネルを再確認した最後の時刻
const unsigned long CHANNEL_LOCK_CHECK_INTERVAL = 5000; ///< チャンネル固定モード時にターゲットチャンネルを再確認する間隔 (ミリ秒)

#pragma pack(push,1) // 構造体のパディングを無効にし、メンバーが密に配置されるようにする (メモリ節約とデータ構造の正確なマッピングのため)

/**
 * @struct Message_head
 * @brief ASTM F3411-19 リモートIDメッセージのヘッダ部分 (1バイト)
 */
typedef struct{
	uint8_t ver:4;			    ///< bit[3-0] Protocol version (プロトコルバージョン)
	uint8_t type:4;			    ///< bit[7-4] Message type (メッセージタイプ)
} Message_head;

/**
 * @struct Seq_ctl
 * @brief Wi-Fi MACヘッダのシーケンスコントロールフィールド (2バイト)
 */
typedef struct{
	uint16_t fragment_num:4;	///< bit[3-0]  Fragment Number (フラグメント番号)
	uint16_t sequence_num:12;	///< bit[15-4] Sequence Number (シーケンス番号)
} Seq_ctl;

/**
 * @struct wifi_mac_hdr_t
 * @brief Wi-Fi MACヘッダ構造体 (一部)
 * @note Beaconフレームの解析に必要な部分を定義
 */
typedef struct{
	uint16_t fctl;				///< Frame Control (フレーム制御フィールド)
	uint16_t duration;	        ///< Duration (デュレーション)
	uint8_t da[6];				///< Destination Address (宛先MACアドレス)
	uint8_t sa[6];				///< Source Address (送信元MACアドレス)
	uint8_t bssid[6];			///< BSSID
	Seq_ctl seqctl;				///< Sequence number (シーケンス番号)
    uint64_t timestamp;		    ///< Timestamp (ビーコンのタイムスタンプ, TSFタイマー値)
	uint16_t interval;			///< Beacon interval (ビーコン間隔)
	uint16_t cap;				///< Capability info (ケーパビリティ情報)
	uint8_t payload[0];         ///< ペイロード開始位置 (可変長のため0要素配列)
} wifi_mac_hdr_t;

/**
 * @struct element_head_t
 * @brief Wi-Fi Information Element (IE) のヘッダ構造体
 */
typedef struct{
	uint8_t id;					///< Element ID (エレメントID)
	uint8_t len;				///< Length (ペイロード長)
	uint8_t payload[0];         ///< ペイロード開始位置 (可変長)
} element_head_t;

/**
 * @struct Status_flag
 * @brief ASTM F3411-19 Location/Vector Message のステータスフラグ (1バイト)
 */
typedef struct{
	uint8_t speed_mul:1;	    ///< bit[0] Speed Multiplier (0: x0.25 m/s, 1: x0.75 m/s)
	uint8_t dir_seg:1;		    ///< bit[1] Direction Segment (0: Direction < 180 degrees, 1: Direction >= 180 degrees)
	uint8_t heght_type:1;	    ///< bit[2] Height Type (0: Height Above Takeoff, 1: Height Above Ground Level)
	uint8_t resv:1;			    ///< bit[3] Reserved (予約)
	uint8_t status:4;		    ///< bit[7-4] Operational Status (運用ステータス)
} Status_flag;

/**
 * @struct H_V_accuracy
 * @brief ASTM F3411-19 Location/Vector Message の水平・垂直精度 (1バイト)
 */
typedef struct{
	uint8_t horizontal:4;	    ///< bit[3-0] Horizontal Accuracy (水平精度)
	uint8_t vetical:4;		    ///< bit[7-4] Vertical Accuracy (垂直精度)
} H_V_accuracy;

/**
 * @struct B_S_accuracy
 * @brief ASTM F3411-19 Location/Vector Message の速度・気圧高度精度 (1バイト)
 */
typedef struct{
	uint8_t speed:4;		    ///< bit[3-0] Speed Accuracy (速度精度)
	uint8_t baro:4;			    ///< bit[7-4] Baro Altitude Accuracy (気圧高度精度)
}B_S_accuracy;

/**
 * @struct RID_Data
 * @brief ASTM F3411-19 に基づくリモートIDデータのペイロード構造 (一部を簡略化・固定化)
 * @note この構造体は、特定のメッセージパックフォーマット (ID, Location, Auth) を前提としています
 *       実際のASTM規格ではより柔軟なメッセージ構成が可能です
 */
typedef struct
{
	uint8_t counter;		    ///< Counter (メッセージカウンター、通常0からインクリメント)
	Message_head msg;			///< Message Pack Header (0xF0) (メッセージタイプパック)
	uint8_t block_size;	        ///< Block Size (通常25バイト)
	uint8_t block_n;		    ///< Block Count (含まれるメッセージブロックの数、ここでは4と仮定)
	//--- Basic ID (Type 1: Serial Number) (25 bytes) --------------------------
	Message_head msg1;				///< Basic ID Message Header (メッセージタイプ 0x00: Basic ID)
	uint8_t UA_type1:4;				///< bit[3-0] UA (Unmanned Aircraft) Type (機体種別)
	uint8_t ID_type1:4;				///< bit[7-4] ID Type (0x01: Serial Number) (ID種別)
	char serial_no[20];				///< Serial Number (UASの製造番号)
	uint8_t resv1[3];               ///< Reserved (予約)
	//--- Basic ID (Type 2: Registration ID) (25 bytes) --------------------------
	Message_head msg2;				///< Basic ID Message Header (メッセージタイプ 0x00: Basic ID)
	uint8_t UA_type2:4;				///< bit[3-0] UA Type (機体種別)
	uint8_t ID_type2:4;				///< bit[7-4] ID Type (0x02: CAA Assigned Registration ID) (ID種別)
	char reg_no[20];				///< Registration Number (登録記号 例：JA.JU012345ABCDE)
	uint8_t resv2[3];               ///< Reserved (予約)
	//--- Location/Vector (25 bytes) --------------------------
	Message_head msg3;				///< Location/Vector Message Header (メッセージタイプ 0x01: Location/Vector)
	Status_flag status;				///< Status Flags (飛行ステータス、方向、高度タイプなど)
	uint8_t dir;					///< Direction (飛行方向, 0-359.9度をエンコード)
	uint8_t speed;					///< Horizontal Speed (水平速度)
	uint8_t Ver_speed;				///< Vertical Speed (垂直速度)
	int32_t lat;					///< Latitude (緯度, 1e-7 degree units)
	int32_t lng;					///< Longitude (経度, 1e-7 degree units)
	int16_t Pressur_Altitude;  	    ///< Pressure Altitude (気圧高度, 0.1m units)
	int16_t Geodetic_Altitude; 	    ///< Geodetic Altitude (ジオイド高度/GPS高度, 0.1m units)
	uint16_t Height;				///< Height Above Takeoff / Ground (離陸地点または地面からの高さ, 0.1m units)
	H_V_accuracy hv_Accuracy;		///< Horizontal and Vertical Accuracy (水平・垂直精度)
	B_S_accuracy bs_Accuracy;		///< Baro Altitude and Speed Accuracy (気圧高度・速度精度)
	uint16_t timestamp_detail;		///< Timestamp (現在時刻の分以下の秒数, 0.1s units, 0-5999)
	uint8_t T_Accuracy;				///< Time Accuracy (時刻精度, 0.1s units)
	uint8_t resv3;                  ///< Reserved (予約)
	//--- Authentication (Page 0) (25 bytes) -----------------------------
	Message_head msg4;				///< Authentication Message Header (メッセージタイプ 0x02: Authentication)
	uint8_t auth_type;		        ///< Authentication Type (認証タイプ, 例: 0x30 - ASTM specific)
	uint8_t page_count;			    ///< Page Count (認証メッセージのページ番号, ここでは0と仮定)
	uint8_t Length;   			    ///< Length of Authentication Data (認証データの長さ, ヘッダ含む)
	uint32_t timestamp_auth;		///< Authentication Timestamp (認証タイムスタンプ, 2019.1.1からの秒数)
	uint8_t auth_head; 	  	        ///< Authentication Data Header (認証データヘッダ, 例: 0x00 - AES-128-CCM)
	uint8_t auth_data[16];          ///< Authentication Data (認証データ, ここでは未処理)
} RID_Data;
#pragma pack(pop) // 構造体のパディング設定を元に戻す

/**
 * @brief Wi-Fiプロミスキャスモードで受信したパケットを処理するコールバック関数
 * @param buf 受信したパケットデータへのポインタ
 * @param type 受信したパケットのタイプ
 * @note この関数は Beacon フレームを解析し、ASTM F3411-19 規格のリモートID情報を抽出します
 */
void wifi_sniffer_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    // MGMTフレーム以外は無視
    if (type != WIFI_PKT_MGMT) {
        return;
    }
    wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t*)buf;
    wifi_mac_hdr_t *mac_hdr = (wifi_mac_hdr_t *)ppkt->payload;
    // Beaconフレーム (Type 0, SubType 8, Frame Control = 0x8000) 以外は無視
    // 0x0080 はリトルエンディアンでの 0x8000 (Beacon)
    if (mac_hdr->fctl != 0x0080) {
        return;
    }
    // MACヘッダ以降のペイロード長を計算
    int payload_len = ppkt->rx_ctrl.sig_len - sizeof(wifi_mac_hdr_t);
    if (payload_len <= 0) return; // ペイロードがなければ処理終了
    element_head_t *element = (element_head_t *)mac_hdr->payload;
    int remaining_len = payload_len;
    bool is_rid_ssid_found = false;
    String rid_serial_from_ssid = ""; // SSIDから抽出したRIDシリアルナンバー
    char ssid_str_buf[MAX_SSID_LEN + 1]; // SSID文字列格納用バッファ
    // --- Pass 1: SSID Information Element (IE) を探し、RID関連か確認 ---
    element_head_t *current_element = element;
    int current_remaining_len = remaining_len;
    while(current_remaining_len > sizeof(element_head_t)){
        if (current_element->id == 0) { // Element ID 0 は SSID
            if (current_element->len > 0 && current_element->len <= MAX_SSID_LEN) {
                strncpy(ssid_str_buf, (char*)current_element->payload, current_element->len);
                ssid_str_buf[current_element->len] = '\0'; // ヌル終端
                String temp_ssid = String(ssid_str_buf);
                if (temp_ssid.startsWith("RID-")) { // SSIDが "RID-" で始まるか
                    is_rid_ssid_found = true;
                    if (temp_ssid.length() > 4) { // "RID-" の後にシリアルナンバーがあるか
                        rid_serial_from_ssid = temp_ssid.substring(4);
                        rid_serial_from_ssid.trim(); // 前後の空白を除去
                    }
                    break; // RID関連SSIDが見つかったのでPass 1終了
                } else {
                    return; // RID関連SSIDでなければこのパケットは無視
                }
            }
        }
        // 次のIEへポインタを進める
        int element_total_len = sizeof(element_head_t) + current_element->len;
        if (element_total_len == 0 || element_total_len > current_remaining_len) break; // 不正な長さならループ中断
        current_remaining_len -= element_total_len;
        current_element = (element_head_t *)((uint8_t *)current_element + element_total_len);
    }
    // RID関連SSIDが見つからなかった場合は処理終了
    if (!is_rid_ssid_found) {
        return;
    }
    // --- Pass 2: Vendor Specific IE を探し、ASTM RIDデータを抽出 ---
    current_element = element; // 再度先頭からIEをスキャン
    current_remaining_len = remaining_len;
    while(current_remaining_len > sizeof(element_head_t)){
        const size_t min_vendor_ie_len = sizeof(ASTM_OUI) + 1; // OUI(3 bytes) + Type(1 byte)
        // Element ID 221 は Vendor Specific IE
        if (current_element->id == 221 && current_element->len >= min_vendor_ie_len) {
            const uint8_t* vendor_payload_ptr = (const uint8_t*)current_element->payload;
            // OUIとTypeがASTM RIDのものと一致するか確認
            if (memcmp(vendor_payload_ptr, ASTM_OUI, sizeof(ASTM_OUI)) == 0 &&
                vendor_payload_ptr[sizeof(ASTM_OUI)] == ASTM_OUI_TYPE_RID) {
                const size_t oui_header_size = sizeof(ASTM_OUI) + 1; // OUI + Typeのサイズ
                // RID_Data構造体のサイズ以上のデータがあるか確認
                if (current_element->len >= oui_header_size + sizeof(RID_Data)) {
                    RID_Data *data = (RID_Data *)(vendor_payload_ptr + oui_header_size);
                    // ペイロードからシリアルナンバーを抽出
                    char sn_buf[sizeof(data->serial_no) + 1];
                    memcpy(sn_buf, data->serial_no, sizeof(data->serial_no));
                    sn_buf[sizeof(data->serial_no)] = '\0';
                    String rid_from_payload_temp = String(sn_buf);
                    rid_from_payload_temp.trim();
                    String rid_from_payload = rid_from_payload_temp;
                    // ペイロードのシリアルナンバーが空なら、SSIDから取得したものを使用
                    if (rid_from_payload.isEmpty()) {
                        rid_from_payload = rid_serial_from_ssid;
                    }
                    // それでも空なら、このデータはスキップ
                    if (rid_from_payload.isEmpty()) {
                        M5.Log.printf("[WARNING] RID is empty after checking payload and SSID. Skipping.\n");
                        return;
                    }
                    time_t current_time_sec = time(NULL); // 現在時刻 (秒)
                    // 各データを適切な型・単位に変換
                    float latitude = static_cast<float>(data->lat) * 1e-7f;
                    float longitude = static_cast<float>(data->lng) * 1e-7f;
                    float pressure_alt = static_cast<float>(data->Pressur_Altitude) * 0.1f;
                    float gps_alt = static_cast<float>(data->Geodetic_Altitude) * 0.1f;
                    // ペイロードから登録記号を抽出
                    char reg_no_buf[sizeof(data->reg_no) + 1];
                    memcpy(reg_no_buf, data->reg_no, sizeof(data->reg_no));
                    reg_no_buf[sizeof(data->reg_no)] = '\0'; // ヌル終端
                    String registration_number_from_payload = String(reg_no_buf);
                    registration_number_from_payload.trim();
                    // セマフォで保護しながらdataManagerにデータを追加
                    if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
                        dataManager.addData(
                            rid_from_payload,                   // RID (シリアルナンバー等)
                            ppkt->rx_ctrl.rssi,                 // RSSI値
                            current_time_sec,                   // M5StickCのシステム時刻
                            mac_hdr->timestamp,                 // BeaconフレームのTSFタイムスタンプ
                            channel,                            // 受信チャンネル
                            registration_number_from_payload, // 登録記号
                            latitude,                           // 緯度
                            longitude,                          // 経度
                            pressure_alt,                       // 気圧高度
                            gps_alt                             // GPS高度
                        );
                        xSemaphoreGive(dataManagerSemaphore);
                        // デバッグログ (必要に応じてコメント解除)
                        // M5.Log.printf("RID: %s, Ch: %d, RSSI: %d, BcnTS: %llu, Lat: %.4f, Lon: %.4f, PAlt: %.1f, GAlt: %.1f\n",
                        //    rid_from_payload.c_str(), channel, ppkt->rx_ctrl.rssi, mac_hdr->timestamp, latitude, longitude, pressure_alt, gps_alt);
                    } else {
                        M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in sniffer_cb\n");
                    }
                    return; // RIDデータ処理完了
                } else {
                     M5.Log.printf("[DEBUG] Vendor IE for ASTM RID too short. Len: %u, Expected Min: %u\n",
                                  current_element->len, oui_header_size + sizeof(RID_Data));
                }
            }
        }
        // 次のIEへポインタを進める
        int element_total_len = sizeof(element_head_t) + current_element->len;
        if (element_total_len == 0 || element_total_len > current_remaining_len) {
             M5.Log.printf("[ERROR] Invalid element length: %d. Remaining: %d\n", current_element->len, current_remaining_len);
             break; // 不正な長さならループ中断
        }
        current_remaining_len -= element_total_len;
        current_element = (element_head_t *)((uint8_t *)current_element + element_total_len);
    }
}

/**
 * @brief Wi-Fiスニッファを初期化し、プロミスキャスモードを開始します
 */
void wifi_sniffer_init(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // デフォルトのWi-Fi初期化設定
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) ); // Wi-Fiドライバ初期化
  ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) ); // 国コード設定 (JP)
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) ); // Wi-Fi設定をRAMに保存 (NVS不使用)
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) ); // Wi-FiモードをNULL (StationでもAPでもない) に設定
  ESP_ERROR_CHECK( esp_wifi_start() ); // Wi-Fi開始
  // プロミスキャスモードのフィルタ設定 (MGMTフレームのみをキャプチャ)
  wifi_promiscuous_filter_t filter;
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  // プロミスキャスモードのコールバック関数を登録
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
  // プロミスキャスモードを有効化
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  // 初期チャンネルを設定
  ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
  M5.Log.printf("Wi-Fi Sniffer initialized on Channel %d.\n", channel);
}

/**
 * @brief Arduinoのセットアップ関数。起動時に一度だけ実行されます
 *        ハードウェア初期化、ディスプレイ設定、Wi-Fiスニッファ初期化などを行います
 */
void setup()
{
    M5.begin(); // M5Unifiedの標準的な初期化 (ディスプレイ初期化より先に行うのが一般的)
    Serial.begin(115200);
    // M5CanvasTextDisplayController の初期化
    displayController_ptr = new M5CanvasTextDisplayController(M5.Display);
    if (!displayController_ptr) {
        M5.Log.printf("[FATAL] Failed to allocate Display Controller!\n");
        while(1); // 致命的エラーなので停止
    }
    // 初期設定: 文字サイズ1, 行ラップ無効, 文字色GREEN, 背景色BLACK, 画面回転1 (横向き)
    if (!displayController_ptr->begin(1, false, GREEN, BLACK, 1)) {
        M5.Log.printf("[FATAL] Failed to initialize Display Controller!\n");
        while(1); // 致命的エラーなので停止
    }
    M5CanvasTextDisplayController& dc = *displayController_ptr; // 以降、dc経由でアクセスするためのエイリアス
    // 画面に表示可能な最大RID数を計算 (ヘッダ行数を考慮)
    int available_rows_for_rids = dc.getRows() - HEADER_LINES;
    if (available_rows_for_rids < 0) available_rows_for_rids = 0;
    max_rids_to_display_calculated = available_rows_for_rids / LINES_PER_RID_ENTRY;
    if (max_rids_to_display_calculated < 0) max_rids_to_display_calculated = 0;
    M5.Log.printf("[INFO] Calculated max RIDs to display: %d\n", max_rids_to_display_calculated);
    // dataManagerアクセス用セマフォの作成
    dataManagerSemaphore = xSemaphoreCreateMutex();
    if (dataManagerSemaphore == NULL) {
        M5.Log.printf("[FATAL] Failed to create dataManagerSemaphore!\n");
        dc.fillScreen(RED); // 画面にエラー表示
        dc.setTextColor(WHITE);
        dc.setCursor(0,0);
        dc.println("Semaphore FAIL");
        dc.show();
        while(1); // 致命的エラーなので停止
    }
    M5.Log.printf("[INFO] Setting up RTC and system time...\n");
    // RTCから時刻を取得し、システム時刻に設定
    auto dt = M5.Rtc.getDateTime();
    // RTCの時刻が古すぎる場合 (未設定など)、デフォルト時刻に設定
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
        dt = M5.Rtc.getDateTime(); // 設定後の時刻を再取得
    }
    // RTC時刻をtime_t (Epoch秒) に変換
    struct tm tminfo;
    tminfo.tm_year = dt.date.year - 1900; // tm_yearは1900年からの年数
    tminfo.tm_mon  = dt.date.month - 1;   // tm_monは0-11月
    tminfo.tm_mday = dt.date.date;
    tminfo.tm_hour = dt.time.hours;
    tminfo.tm_min  = dt.time.minutes;
    tminfo.tm_sec  = dt.time.seconds;
    tminfo.tm_isdst = -1; // 夏時間を自動判別
    time_t rtc_epoch = mktime(&tminfo);
    if (rtc_epoch == -1) {
        M5.Log.printf("[ERROR] Failed to convert RTC datetime to epoch. System time may be incorrect.\n");
    } else {
        M5.Log.printf("[INFO] RTC Epoch: %ld\n", rtc_epoch);
        // システム時刻 (timeval構造体) を設定
        struct timeval tv = { rtc_epoch, 0 };
        if (settimeofday(&tv, nullptr) == 0) {
            M5.Log.printf("[INFO] System time set successfully from RTC.\n");
        } else {
            M5.Log.printf("[ERROR] Failed to set system time (settimeofday).\n");
        }
    }
    time_t current_system_time = time(NULL);
    M5.Log.printf("[INFO] Current system time (epoch): %ld\n", current_system_time);
    // Wi-Fiスニッファ初期化
    wifi_sniffer_init();
    M5.Log.printf("Setup completed. Starting RID sniffing...\n");
    dc.clearDrawingCanvas(); // 既存のメッセージをクリア
    dc.setCursor(0, dc.getRows() / 3); // 画面中央やや上にカーソル移動
    dc.println("Setup Complete!");
    dc.println("A: Send JSON");
    dc.println("B: Ch Lock");
    dc.println("C: Reset Device");
    dc.show(); // 描画内容をLCDに反映
    delay(3000); // 初期メッセージ表示時間
}

/**
 * @brief Arduinoのメインループ関数。setup()後に繰り返し実行されます
 *        ボタン入力処理、チャンネル制御、画面表示更新などを行います
 */
void loop()
{
    M5.update(); // M5Unifiedのボタン状態などを更新
    if (!displayController_ptr) return; // displayControllerが初期化失敗していたら何もしない
    M5CanvasTextDisplayController& dc = *displayController_ptr; // エイリアス
    // --- ボタンA: JSONデータをシリアル送信 ---
    if (M5.BtnA.wasPressed()) {
        M5.Log.println("Button A pressed. Preparing JSON data...");
        dc.clearDrawingCanvas();
        dc.setCursor(0,0);
        dc.println("BTN_A: Sending JSON...");
        dc.show();
        if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
#           if SEND_MODE_TOP_RSSI == 1
                M5.Log.printf("Mode: Top RSSI, Max Entries: %u\n", MAX_ENTRIES_IN_JSON);
                dataManager.getJsonForTopRSSI(1, MAX_ENTRIES_IN_JSON, Serial);
#           else
                M5.Log.printf("Mode: Reg No '%s', Max Entries: %u\n", TARGET_REG_NO_FOR_JSON, MAX_ENTRIES_IN_JSON);
                dataManager.getJsonForRegistrationNo(String(TARGET_REG_NO_FOR_JSON), MAX_ENTRIES_IN_JSON, Serial);
#           endif
            xSemaphoreGive(dataManagerSemaphore);
            M5.Log.println("JSON data streamed to Serial.");
            dc.clearDrawingCanvas();
            dc.setCursor(0,0);
            dc.println("JSON Sent (Streamed)");
            dc.println("Check PC.");
            dc.show();
            delay(2500);
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
    // --- ボタンB: チャンネル固定モード切り替え ---
    if (M5.BtnB.wasPressed()) {
        channelLockModeActive = !channelLockModeActive; // モードをトグル
        if (channelLockModeActive) {
            M5.Log.println("Channel lock mode ACTIVATED.");
            lockedChannel = -1; // 最初は固定チャンネル未定
            lastChannelLockCheck = millis(); // チェックタイマーリセット
        } else {
            M5.Log.println("Channel lock mode DEACTIVATED. Resuming scan.");
            lockedChannel = -1; // 固定解除
            // 次の通常のチャンネル切り替えタイミングでスキャンが再開される
        }
        // 即座に画面更新するために、表示更新タイマーをリセットすることも検討可能
        // last_display_update = 0; // これにより次のloopで即座に画面更新
    }
    // --- ボタンC (M5StickCPlus2では電源ボタン): リセット ---
    if (M5.BtnPWR.wasPressed()) { // M5.BtnC は M5StickCPlusではサイドボタン。M5StickCPlus2の物理ボタンCに該当するのはBtnPWR
        M5.Log.println("Button C (PWR) pressed. Resetting...");
        dc.clearDrawingCanvas();
        dc.setCursor(0, dc.getRows() / 2 -1);
        dc.println("Resetting...");
        dc.show();
        delay(1000); // メッセージ表示のための短い遅延
        ESP.restart(); // ESP32を再起動
    }
    // --- 定期的な画面表示更新処理 (WIFI_CHANNEL_SWITCH_INTERVALごと) ---
    static unsigned long last_display_update = 0;
    if (millis() - last_display_update > WIFI_CHANNEL_SWITCH_INTERVAL) {
        last_display_update = millis();
        // --- チャンネル制御ロジック ---
        if (channelLockModeActive) {
            // チャンネル固定モードが有効な場合
            if (lockedChannel == -1 || (millis() - lastChannelLockCheck > CHANNEL_LOCK_CHECK_INTERVAL)) {
                // 固定チャンネルが未設定、または定期的なターゲット再確認のタイミング
                lastChannelLockCheck = millis();
                int targetChannel = -1; // 固定対象とするチャンネル
                // セマフォで保護しながらdataManagerから情報を取得
                if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(50)) == pdTRUE) {
#                   if SEND_MODE_TOP_RSSI == 1
                        // RSSIが最も強いRIDの最新チャンネルをターゲットとする
                        std::vector<std::pair<int, String>> sorted_rids = dataManager.getSortedRIDsByRSSI();
                        if (!sorted_rids.empty()) {
                            RemoteIDEntry latest_entry;
                            if (dataManager.getLatestEntryForRID(sorted_rids[0].second, latest_entry)) {
                                targetChannel = latest_entry.channel;
                            }
                        }
#                   else
                        // 指定された登録記号のRIDの最新チャンネルをターゲットとする
                        // 注: RemoteIDDataManagerにこのための直接的なヘルパー関数がない場合、
                        //     _data_storeへのアクセスが必要になるが、これはカプセル化の観点からは非推奨。
                        //     理想的にはdataManagerに `getChannelForRegistrationNo` のような関数を追加する。
                        //     ここでは簡易的に内部データ構造を参照する例を示す（要RemoteIDDataManager.hの実装確認）。
                        RemoteIDEntry latest_entry;
                        if (dataManager.getLatestEntryForRegistrationNo(String(TARGET_REG_NO_FOR_JSON), latest_entry)) {
                            targetChannel = latest_entry.channel;
                        }
#                   endif
                    xSemaphoreGive(dataManagerSemaphore);
                } else {
                    M5.Log.println("[WARNING] Failed to take semaphore for channel lock target check.");
                }
                if (targetChannel != -1 && targetChannel >= 1 && targetChannel <= WIFI_CHANNEL_MAX) {
                    // 有効なターゲットチャンネルが見つかった場合
                    if (lockedChannel != targetChannel) { // 現在の固定チャンネルと異なる場合のみ設定変更
                        esp_err_t err = esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);
                        if (err == ESP_OK) {
                            M5.Log.printf("Channel locked to: %d\n", targetChannel);
                            lockedChannel = targetChannel; // 固定チャンネルを更新
                            channel = lockedChannel;       // 表示用のグローバル変数も更新
                        } else {
                            M5.Log.printf("[ERROR] Failed to lock channel to %d: %s\n", targetChannel, esp_err_to_name(err));
                            // lockedChannel = -1; // 設定失敗した場合、ロック解除するかどうかはポリシーによる
                        }
                    }
                } else {
                    // ターゲットが見つからない場合。lockedChannelは変更しない（以前の値または-1を維持）。
                    // M5.Log.println("Target for channel lock not found or invalid. Maintaining current state.");
                }
            }
            // lockedChannelが有効なら、そのチャンネルを維持（esp_wifi_set_channelの頻繁な呼び出しを避ける）。
            // 表示用の 'channel' 変数はlockedChannelの値を使用。
            if (lockedChannel != -1) {
                 channel = lockedChannel; // 表示用チャンネルを固定チャンネルに合わせる
            } else {
                 // ターゲットが見つからず、まだロックできていない場合は、通常のチャンネルスキャンを1ステップ進める
                 channel = (channel % WIFI_CHANNEL_MAX) + 1;
                 esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            }
        } else {
            // 通常のチャンネルスキャンモード
            channel = (channel % WIFI_CHANNEL_MAX) + 1; // チャンネルを1からWIFI_CHANNEL_MAXまで巡回
            esp_err_t err = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            if (err != ESP_OK) {
                M5.Log.printf("[ERROR] Failed to set channel to %d: %s\n", channel, esp_err_to_name(err));
            }
            lockedChannel = -1; // 固定モードではないので-1にリセット
        }
        // --- 画面表示更新 ---
        dc.clearDrawingCanvas(); // 描画キャンバスをクリア
        dc.setCursor(0, 0);      // カーソルを左上にリセット
        int current_rid_count = 0;
        // セマフォで保護しながらdataManagerからRID数を取得
        if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
            current_rid_count = dataManager.getRIDCount();
            xSemaphoreGive(dataManagerSemaphore);
        } else {
            M5.Log.println("[WARNING] Failed to take dataManagerSemaphore in loop for RID count.");
        }
        // ヘッダ情報表示
        char header_buf[80];
        if (channelLockModeActive) {
            if (lockedChannel != -1) {
                snprintf(header_buf, sizeof(header_buf), "Ch:%2d(L) RIDs:%d H:%u", lockedChannel, current_rid_count, ESP.getFreeHeap());
            } else {
                snprintf(header_buf, sizeof(header_buf), "Ch:Lock? RIDs:%d H:%u", current_rid_count, ESP.getFreeHeap()); // 固定試行中だが未確定
            }
        } else {
            snprintf(header_buf, sizeof(header_buf), "Ch:%2d(S) RIDs:%d H:%u", channel, current_rid_count, ESP.getFreeHeap()); // (S)can mode
        }
        dc.println(header_buf);
        // 区切り線表示
        String separator = "";
        for (int i = 0; i < dc.getCols(); ++i) {
            separator += "-";
        }
        dc.println(separator);
        // セマフォで保護しながらdataManagerから表示用データを取得し描画
        if (xSemaphoreTake(dataManagerSemaphore, pdMS_TO_TICKS(50)) == pdTRUE) {
            // RSSI降順でソートされたRIDのリストを取得 (RSSI値, RID文字列 のペア)
            std::vector<std::pair<int, String>> sorted_rids_info = dataManager.getSortedRIDsByRSSI();
            int rid_count_available = sorted_rids_info.size();
            int displayed_count = 0;
            // 実際に表示するRID数を決定 (利用可能なRID数と画面に表示可能な最大数のうち小さい方)
            int rids_to_actually_display = min(rid_count_available, max_rids_to_display_calculated);
            if (rid_count_available > 0) {
                for (int i = 0; i < rids_to_actually_display && displayed_count < max_rids_to_display_calculated; ++i) {
                    // 次のRID情報を表示するための行数が画面内に収まるかチェック
                    if (dc.getPrintCursorRow() + LINES_PER_RID_ENTRY > dc.getRows()) {
                        break; // 画面からはみ出るならループ中断
                    }
                    String current_rid_str = sorted_rids_info[i].second;
                    if (!current_rid_str.isEmpty()) {
                        RemoteIDEntry latest_entry; // 最新の受信データを格納する構造体
                        if (dataManager.getLatestEntryForRID(current_rid_str, latest_entry)) {
                            char line_buf[128]; // 各行表示用バッファ
                            // 1行目: RID (RSSI, Ch)
                            String rid_to_display = current_rid_str;
                            // " (RSSI,Ch:XX)" のために約15文字消費 + マージンを考慮
                            int max_rid_len_for_line = dc.getCols() - 16;
                            if (max_rid_len_for_line < 1) max_rid_len_for_line = 1;
                            if (rid_to_display.length() > (unsigned int)max_rid_len_for_line) {
                                rid_to_display = rid_to_display.substring(0, max_rid_len_for_line);
                            }
                            snprintf(line_buf, sizeof(line_buf), "%s (%d,Ch:%d)", rid_to_display.c_str(), latest_entry.rssi, latest_entry.channel);
                            dc.println(line_buf);
                            // 2行目: 登録記号
                            if (!latest_entry.registrationNo.isEmpty()) {
                                String reg_no_to_display = latest_entry.registrationNo;
                                // "Reg:" のために4文字消費
                                int max_reg_len = dc.getCols() - 4;
                                if (max_reg_len < 1) max_reg_len = 1;
                                if (reg_no_to_display.length() > (unsigned int)max_reg_len) {
                                    reg_no_to_display = reg_no_to_display.substring(0, max_reg_len);
                                }
                                snprintf(line_buf, sizeof(line_buf), "Reg:%s", reg_no_to_display.c_str());
                                dc.println(line_buf);
                            } else {
                                dc.println("Reg:N/A");
                            }
                            // 3行目: 緯度/経度
                            snprintf(line_buf, sizeof(line_buf), "L:%.3f Lo:%.3f", latest_entry.latitude, latest_entry.longitude);
                            dc.println(line_buf);
                            // 4行目: 高度情報と受信時刻 (M5StickC時刻)
                            char time_str[10] = "N/A Time";
                            time_t entry_time = latest_entry.timestamp; // これはM5StickCのシステム時刻
                            struct tm *tm_info = localtime(&entry_time);
                            if (tm_info) {
                                 snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
                            }
                            snprintf(line_buf, sizeof(line_buf), "P:%.0fm G:%.0fm %s", latest_entry.pressureAltitude, latest_entry.gpsAltitude, time_str);
                            dc.println(line_buf);
                            // 5行目: Beacon TSF タイムスタンプ (下位桁のみ表示)
                            snprintf(line_buf, sizeof(line_buf), "BcnTS: ..%03lu.%06lu",
                                     (unsigned long)((latest_entry.beaconTimestamp / 1000000ULL) % 1000), // 秒部分の下3桁 (TSFはマイクロ秒単位)
                                     (unsigned long)(latest_entry.beaconTimestamp % 1000000ULL));       // マイクロ秒部分6桁
                            dc.println(line_buf);
                            displayed_count++;
                        }
                    }
                }
            }
            // 表示するデータがなかった場合のメッセージ
            if (displayed_count == 0 && rid_count_available == 0) {
                 if (dc.getPrintCursorRow() < dc.getRows()) { // 画面に空き行があれば
                    dc.println("No RID data yet.");
                 }
            } else if (displayed_count == 0 && rid_count_available > 0) { // データはあるが表示スペースがなかった場合
                if (dc.getPrintCursorRow() < dc.getRows()) {
                    dc.println("No space to show RIDs");
                }
            }
            xSemaphoreGive(dataManagerSemaphore);
        } else {
            M5.Log.printf("[WARNING] Failed to take dataManagerSemaphore in loop for display.\n");
            if (dc.getPrintCursorRow() < dc.getRows()) { // 画面に空き行があれば
                dc.println("Failed to get data...");
            }
        }
        dc.show(); // 全ての描画が終わったら、描画キャンバスの内容をLCDに転送
    }
}
