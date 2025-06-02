#ifndef REMOTE_ID_DATA_MANAGER_H
#define REMOTE_ID_DATA_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <deque>
#include <map>
#include <algorithm> // std::sort
#include <climits>   // INT_MIN (C++11以降)
#include <ctime>     // time_t (C++ style)

// 個々のデータエントリを表す構造体
struct RemoteIDEntry {
    int rssi;
    time_t timestamp; // UNIXタイムスタンプ (秒)
    uint64_t beaconTimestamp; // ビーコンフレームのタイムスタンプ (マイクロ秒)
    int channel;            // 受信したWi-Fiチャンネル
    float latitude;
    float longitude;
    float pressureAltitude;
    float gpsAltitude;
    // デフォルトコンストラクタ
    RemoteIDEntry() : rssi(0), timestamp(0), beaconTimestamp(0), channel(0), latitude(0.0f), longitude(0.0f), pressureAltitude(0.0f), gpsAltitude(0.0f) {}
    // パラメータ付きコンストラクタ
    RemoteIDEntry(int r, time_t ts, uint64_t bTs, int ch, float lat, float lon, float pa, float ga)
        : rssi(r), timestamp(ts), beaconTimestamp(bTs), channel(ch), latitude(lat), longitude(lon), pressureAltitude(pa), gpsAltitude(ga) {}
};

class RemoteIDDataManager {
public:
    // コンストラクタ: target_rid (特別扱いするRID) を指定
    RemoteIDDataManager(const String& targetRid);
    // データ追加メソッド
    void addData(const String& rid, int rssi, time_t timestamp, uint64_t beaconTimestamp, int channel, float lat, float lon, float pAlt, float gAlt);
    // 指定時刻(currentTime)から過去1分以内にデータ記録があるRIDのリストを返す
    std::vector<String> getRIDsWithDataInLastMinute(time_t currentTime) const;
    // 指定RIDのすべてのデータを時系列順(古いものから新しいもの)で返す
    std::vector<RemoteIDEntry> getAllDataForRID(const String& rid) const;
    // 登録されているRIDの総数を返す
    int getRIDCount() const;
    // インデックスを指定して、該当するRIDの全データ(時系列順)を返す
    // インデックスは、全RIDを最新データのRSSI降順でソートした時の順位
    std::vector<RemoteIDEntry> getDataByIndex(int index) const;
    // (オプション) インデックスを指定してRID文字列自体を取得 (RSSIの降順)
    String getRIDStringByIndex(int index) const;
    // --- 使いやすさ向上のための追加メソッド案 ---
    // 特定のRIDがデータストアに存在するか確認
    bool hasRID(const String& rid) const;
    // 特定のRIDの最新データを取得 (取得できた場合trueを返す)
    bool getLatestEntryForRID(const String& rid, RemoteIDEntry& entry) const; // entryに出力
    // 全てのRIDデータをクリア
    void clearAllData();
    // 特定のRIDのデータをクリア
    void clearDataForRID(const String& rid);
    // RSSI降順でソートされたRIDのリスト {latest_rssi, rid_string} を取得するヘルパー
    std::vector<std::pair<int, String>> getSortedRIDsByRSSI() const;

private:
    // RIDごとのデータと設定を保持する内部構造体
    struct RIDDataContainer {
        std::deque<RemoteIDEntry> entries; // データ履歴 (リングバッファとして使用)
        size_t max_size;                   // このコンテナの最大データ保存数
        int latest_rssi;                   // 最新データのRSSI (ソート用)
        time_t latest_timestamp;           // 最新データのタイムスタンプ (フィルタ用)
        RIDDataContainer(size_t maxSize) : max_size(maxSize), latest_rssi(INT_MIN), latest_timestamp(0) {}
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
    String _target_rid_value; // 特別扱いするRID
    static const size_t TARGET_RID_MAX_DATA = 3600; // target_ridの最大保存数
    static const size_t OTHER_RID_MAX_DATA = 2;     // target_rid以外の最大保存数
    static const time_t ONE_MINUTE_IN_SECONDS = 60; // 1分の秒数
    // RID文字列をキーとして、RIDDataContainerを値とするマップ
    std::map<String, RIDDataContainer> _data_store;
    // target_ridかどうかを判定するヘルパー
    bool isTargetRID(const String& rid) const {
        return rid == _target_rid_value;
    }
};

#endif // REMOTE_ID_DATA_MANAGER_H