#ifndef REMOTE_ID_DATA_MANAGER_H
#define REMOTE_ID_DATA_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <deque>
#include <map>
#include <algorithm> // std::sort
#include <climits>   // INT_MIN (C++11以降)
#include <ctime>     // time_t (C++ style)
#include <ArduinoJson.h>

/**
 * @file RemoteIDDataManager.h
 * @brief リモートIDデータの管理を行うRemoteIDDataManagerクラスとその関連構造体の定義
 */

/// @brief 個々のリモートIDデータエントリを表す構造体
///
/// リモートIDビーコンから受信した情報を格納します
struct RemoteIDEntry {
    int rssi;                   ///< 受信信号強度インジケータ (RSSI)
    time_t timestamp;           ///< データ受信時のUNIXタイムスタンプ (秒単位)
    uint64_t beaconTimestamp;   ///< ビーコンフレーム自体のタイムスタンプ (マイクロ秒単位)
    int channel;                ///< 受信したWi-Fiチャンネル
    String registrationNo;      ///< 機体登録記号
    float latitude;             ///< 緯度 (度)
    float longitude;            ///< 経度 (度)
    float pressureAltitude;     ///< 気圧高度 (メートル)
    float gpsAltitude;          ///< GPS高度 (メートル)

    /// @brief デフォルトコンストラクタ
    /// メンバ変数をゼロまたは空の状態で初期化します
    RemoteIDEntry() : rssi(0), timestamp(0), beaconTimestamp(0), channel(0), registrationNo(""), latitude(0.0f), longitude(0.0f), pressureAltitude(0.0f), gpsAltitude(0.0f) {}

    /// @brief パラメータ付きコンストラクタ
    /// @param r RSSI値
    /// @param ts 受信タイムスタンプ (UNIX秒)
    /// @param bTs ビーコンタイムスタンプ (マイクロ秒)
    /// @param ch 受信Wi-Fiチャンネル
    /// @param regNo 機体登録記号
    /// @param lat 緯度
    /// @param lon 経度
    /// @param pa 気圧高度
    /// @param ga GPS高度
    RemoteIDEntry(int r, time_t ts, uint64_t bTs, int ch, const String& regNo, float lat, float lon, float pa, float ga)
        : rssi(r), timestamp(ts), beaconTimestamp(bTs), channel(ch), registrationNo(regNo), latitude(lat), longitude(lon), pressureAltitude(pa), gpsAltitude(ga) {}
};

/// @brief リモートIDデータを管理するクラス
///
/// 複数のリモートID (RID) からのデータを格納し、クエリ機能を提供します
/// 特定のRIDを「ターゲットRID」として指定し、より多くのデータを保持することができます
/// データはRIDごとに時系列でリングバッファに保存されます
class RemoteIDDataManager {
public:
    /// @brief コンストラクタ
    /// @param targetRid 特別扱いするRIDの文字列。このRIDは他のRIDよりも多くのデータエントリを保持します
    RemoteIDDataManager(const String& targetRid);

    /// @brief 新しいリモートIDデータを追加します
    /// @param rid データを送信したリモートIDの識別子
    /// @param rssi RSSI値
    /// @param timestamp 受信タイムスタンプ (UNIX秒)
    /// @param beaconTimestamp ビーコンタイムスタンプ (マイクロ秒)
    /// @param channel 受信Wi-Fiチャンネル
    /// @param registrationNo 機体登録記号
    /// @param lat 緯度
    /// @param lon 経度
    /// @param pAlt 気圧高度
    /// @param gAlt GPS高度
    void addData(const String& rid, int rssi, time_t timestamp, uint64_t beaconTimestamp, int channel, const String& registrationNo, float lat, float lon, float pAlt, float gAlt);

    /// @brief 指定時刻から過去1分以内にデータ記録があるRIDのリストを取得します
    /// @param currentTime 現在時刻 (UNIX秒)。この時刻を基準に過去1分間を評価します
    /// @return 過去1分以内にデータがあったRIDのString型リスト
    std::vector<String> getRIDsWithDataInLastMinute(time_t currentTime) const;

    /// @brief 指定されたRIDのすべてのデータエントリを時系列順（古いものから新しいもの）で取得します
    /// @param rid データを取得したいRIDの識別子
    /// @param max_entries 返すエントリの最大数。0の場合は全てのエントリを返します。指定された場合、最新の `max_entries` 件を返します
    /// @return 指定されたRIDのデータエントリのベクター。RIDが存在しない場合は空のベクター
    std::vector<RemoteIDEntry> getAllDataForRID(const String& rid, size_t max_entries = 0) const;

    /// @brief 現在データストアに登録されているRIDの総数を返します
    /// @return RIDの総数
    int getRIDCount() const;

    /// @brief インデックスを指定して、該当するRIDの全データ（時系列順）を取得します
    ///        インデックスは、全RIDを最新データのRSSI降順でソートした時の順位に基づきます
    /// @param index 取得したいRIDのインデックス (0から始まる)
    /// @return 指定されたインデックスのRIDのデータエントリのベクター。インデックスが無効な場合は空のベクター
    std::vector<RemoteIDEntry> getDataByIndex(int index) const;

    /// @brief インデックスを指定して、該当するRIDの文字列自体を取得します
    ///        インデックスは、全RIDを最新データのRSSI降順でソートした時の順位に基づきます
    /// @param index 取得したいRIDのインデックス (0から始まる)
    /// @return 指定されたインデックスのRID文字列。インデックスが無効な場合は空文字列
    String getRIDStringByIndex(int index) const;

    /// @brief 特定のRIDがデータストアに存在するかどうかを確認します
    /// @param rid 確認したいRIDの識別子
    /// @return RIDが存在すればtrue、存在しなければfalse
    bool hasRID(const String& rid) const;

    /// @brief 特定のRIDの最新データエントリを取得します
    /// @param rid データを取得したいRIDの識別子
    /// @param[out] entry 取得した最新データエントリを格納する参照
    /// @return データが取得できた場合はtrue、RIDが存在しないかデータがない場合はfalse
    bool getLatestEntryForRID(const String& rid, RemoteIDEntry& entry) const; // entryに出力

    /// @brief データストア内の全てのRIDデータをクリアします
    void clearAllData();

    /// @brief 特定のRIDに関連する全てのデータをクリアします
    /// @param rid データをクリアしたいRIDの識別子
    void clearDataForRID(const String& rid);

    /// @brief RSSIの降順でソートされたRIDのリストを取得するヘルパーメソッド
    ///        リストの各要素は {最新RSSI, RID文字列} のペアです
    /// @return RSSI降順、その後RID文字列昇順でソートされたペアのベクター
    std::vector<std::pair<int, String>> getSortedRIDsByRSSI() const;

    /// @brief RSSIが最も高い上位 `count` 件のRIDデータを、受け取ったストリームに出力します
    ///        現状の実装では `count` は実質1として動作し、最もRSSIが高い1つのRIDのデータを返します
    /// @param count 取得する上位RIDの数 (現在は1に固定して利用されることを想定)
    /// @param max_log_entries 1つのRIDに対してJSONに含めるデータエントリの最大数
    /// @param output_stream 出力ストリームを受け取る
    void getJsonForTopRSSI(int count, size_t max_log_entries, Print& output_stream) const;

    /// @brief 指定された登録記号を持つRIDのデータを、受け取ったストリームに出力します
    ///        最初に見つかった登録記号に合致するRIDのデータを返します
    /// @param regNo 検索する機体登録記号
    /// @param max_log_entries JSONに含めるデータエントリの最大数
    /// @param output_stream 出力ストリームを受け取る
    void getJsonForRegistrationNo(const String& regNo, size_t max_log_entries, Print& output_stream) const;

    /// @brief RSSIが最も高いRIDの最新データが受信されたWi-Fiチャンネルを取得します
    /// @return 最新のWi-Fiチャンネル番号。該当データがない場合は-1
    int getLatestChannelForTopRSSI() const;

    /// @brief 指定された登録記号を持つRIDの最新データが受信されたWi-Fiチャンネルを取得します
    /// @param regNo 検索する機体登録記号
    /// @return 最新のWi-Fiチャンネル番号。該当データがない場合は-1
    int getLatestChannelForRegistrationNo(const String& regNo) const;

private:
    /// @brief RIDごとのデータと設定を保持する内部構造体
    ///
    /// 各RIDのデータエントリ履歴をリングバッファ形式で格納し、
    /// 最新のRSSIとタイムスタンプも保持して効率的なアクセスを可能にします
    struct RIDDataContainer {
        std::deque<RemoteIDEntry> entries; ///< データエントリの履歴 (リングバッファとして使用)。古いものから順に格納
        size_t max_size;                   ///< このコンテナが保持できるデータエントリの最大数
        int latest_rssi;                   ///< 最新データエントリのRSSI値 (ソート用)
        time_t latest_timestamp;           ///< 最新データエントリのタイムスタンプ (フィルタリング用)

        /// @brief RIDDataContainerのコンストラクタ
        /// @param maxSize このコンテナが保持するエントリの最大数
        RIDDataContainer(size_t maxSize) : max_size(maxSize), latest_rssi(INT_MIN), latest_timestamp(0) {}

        /// @brief 新しいデータエントリをコンテナに追加します
        ///        エントリ数が `max_size` を超える場合は、最も古いエントリが削除されます
        /// @param entry 追加するRemoteIDEntryオブジェクト
        void addEntry(const RemoteIDEntry& entry) {
            entries.push_back(entry);
            if (entries.size() > max_size) {
                entries.pop_front(); // 古いデータを削除
            }
            // 最新情報を更新 (entriesが空になることはaddEntry直後はないはず)
            if (!entries.empty()) {
                latest_rssi = entries.back().rssi;
                latest_timestamp = entries.back().timestamp;
            }
        }
    };

    String _target_rid_value; ///< 特別扱いするRIDの識別子。このRIDはより多くのデータを保持します

    static const size_t TARGET_RID_MAX_DATA = 1200; ///< `_target_rid_value` に指定されたRIDが保持するデータエントリの最大数
    static const size_t OTHER_RID_MAX_DATA = 10;   ///< `_target_rid_value` 以外のRIDが保持するデータエントリの最大数
    static const time_t ONE_MINUTE_IN_SECONDS = 60; ///< 1分間の秒数 (定数)

    /// @brief RID文字列をキーとして、RIDDataContainerを値とするマップ
    ///        これが主要なデータストアとなります
    std::map<String, RIDDataContainer> _data_store;

    /// @brief 指定されたRIDが `_target_rid_value` と一致するかどうかを判定するヘルパーメソッド
    /// @param rid 判定するRIDの識別子
    /// @return `_target_rid_value` と一致すればtrue、そうでなければfalse
    bool isTargetRID(const String& rid) const {
        return rid == _target_rid_value;
    }

    /// @brief RemoteIDEntryの内容をJsonObjectに格納するプライベートヘルパーメソッド
    ///        JSONのキー名は短縮形を使用します
    /// @param jsonObj 格納先のJsonObject
    /// @param entry 格納するRemoteIDEntryデータ
    void _populateJsonEntry(JsonObject jsonObj, const RemoteIDEntry& entry) const;
};

#endif // REMOTE_ID_DATA_MANAGER_H