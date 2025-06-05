#include "RemoteIDDataManager.h"

RemoteIDDataManager::RemoteIDDataManager(const String& targetRid)
    : _target_rid_value(targetRid) {
    // 必要であれば初期化処理
}

void RemoteIDDataManager::addData(const String& rid, int rssi, time_t timestamp, uint64_t beaconTimestamp, int channel, const String& registrationNo, float lat, float lon, float pAlt, float gAlt) {
    RemoteIDEntry new_entry(rssi, timestamp, beaconTimestamp, channel, registrationNo, lat, lon, pAlt, gAlt);
    auto it = _data_store.find(rid);
    if (it == _data_store.end()) {
        // 新しいRIDの場合、コンテナを作成してマップに追加
        size_t max_size = isTargetRID(rid) ? TARGET_RID_MAX_DATA : OTHER_RID_MAX_DATA;
        // std::piecewise_construct を使うと、キーと値のコンストラクタ引数をタプルで渡せる
        it = _data_store.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(rid),          // キー(String)のコンストラクタ引数
                                 std::forward_as_tuple(max_size)).first; // 値(RIDDataContainer)のコンストラクタ引数
    }
    // 既存または新規作成したコンテナにデータを追加
    it->second.addEntry(new_entry);
}

std::vector<String> RemoteIDDataManager::getRIDsWithDataInLastMinute(time_t currentTime) const {
    std::vector<String> result_rids;
    const time_t one_minute_ago = currentTime - ONE_MINUTE_IN_SECONDS;
    for (const auto& pair : _data_store) {
        const RIDDataContainer& container = pair.second;
        if (container.entries.empty()) {
            continue; // データがない場合はスキップ
        }
        // 最新のデータが1分より古いか、最古のデータが現在時刻より未来の場合、このコンテナは対象外の可能性が高い
        // (リングバッファの特性上、latest_timestamp が one_minute_ago より前なら、他のエントリも古い)
        if (container.latest_timestamp < one_minute_ago || container.entries.front().timestamp > currentTime) {
            continue;
        }
        // 上記のフィルタを通過した場合、コンテナ内に該当期間のデータがあるか実際に確認する
        for (const auto& entry : container.entries) {
            if (entry.timestamp >= one_minute_ago && entry.timestamp <= currentTime) {
                result_rids.push_back(pair.first);
                break; // このRIDは条件を満たしたので、次のRIDのチェックへ
            }
        }
    }
    return result_rids;
}

std::vector<RemoteIDEntry> RemoteIDDataManager::getAllDataForRID(const String& rid, size_t max_entries) const {
    auto it = _data_store.find(rid);
    if (it != _data_store.end()) {
        const auto& deque_entries = it->second.entries;
        if (max_entries > 0 && deque_entries.size() > max_entries) {
            // dequeの末尾からmax_entries個の要素を取得
            std::vector<RemoteIDEntry> result_vec;
            result_vec.reserve(max_entries);
            auto start_it = deque_entries.end();
            // dequeのイテレータはランダムアクセスではないので、max_entries分戻る
            // ただし、dequeのサイズがmax_entriesより小さい場合も考慮
            size_t num_to_copy = std::min(deque_entries.size(), max_entries);
            if (num_to_copy > 0) {
                start_it = deque_entries.end() - num_to_copy;
            } else {
                return {}; // コピーするものがなければ空を返す
            }
            for (auto iter = start_it; iter != deque_entries.end(); ++iter) {
                result_vec.push_back(*iter);
            }
            return result_vec;
        } else {
            // 全件返すか、元々max_entries以下ならそのままコピー
            return std::vector<RemoteIDEntry>(deque_entries.begin(), deque_entries.end());
        }
    }
    return {}; // RIDが見つからない場合は空のベクター
}

int RemoteIDDataManager::getRIDCount() const {
    return _data_store.size();
}

std::vector<std::pair<int, String>> RemoteIDDataManager::getSortedRIDsByRSSI() const {
    std::vector<std::pair<int, String>> sorted_rids_list;
    if (_data_store.empty()) {
        return sorted_rids_list;
    }
    for (const auto& pair : _data_store) {
        if (!pair.second.entries.empty()) {
            sorted_rids_list.push_back({pair.second.latest_rssi, pair.first});
        }
    }
    std::sort(sorted_rids_list.begin(), sorted_rids_list.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) {
            return a.first > b.first; // RSSI降順
        }
        return a.second < b.second; // RID文字列昇順
    });
    return sorted_rids_list;
}

std::vector<RemoteIDEntry> RemoteIDDataManager::getDataByIndex(int index) const {
    if (index < 0) {
        return {};
    }
    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();
    if (static_cast<size_t>(index) >= sorted_rids.size()) {
        return {};
    }
    String selected_rid = sorted_rids[index].second;
    return getAllDataForRID(selected_rid, 0); // 0 を渡して全件取得
}

String RemoteIDDataManager::getRIDStringByIndex(int index) const {
    if (index < 0) {
        return "";
    }
    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();
    if (static_cast<size_t>(index) >= sorted_rids.size()) {
        return "";
    }
    return sorted_rids[index].second;
}

bool RemoteIDDataManager::hasRID(const String& rid) const {
    return _data_store.count(rid) > 0;
}

bool RemoteIDDataManager::getLatestEntryForRID(const String& rid, RemoteIDEntry& entry) const {
    auto it = _data_store.find(rid);
    if (it != _data_store.end() && !it->second.entries.empty()) {
        entry = it->second.entries.back();
        return true;
    }
    return false;
}

void RemoteIDDataManager::clearAllData() {
    _data_store.clear();
}

void RemoteIDDataManager::clearDataForRID(const String& rid) {
    _data_store.erase(rid);
}

// JSONエントリーを生成するプライベートヘルパーメソッド
void RemoteIDDataManager::_populateJsonEntry(JsonObject jsonObj, const RemoteIDEntry& entry) const {
    jsonObj["rssi"] = entry.rssi;
    jsonObj["ts"] = entry.timestamp; // 短縮名 "timestamp" -> "ts"
    // beaconTimestampは非常に大きな数値になる可能性があるため、文字列として格納するか、
    // 上位と下位32ビットに分割することも検討できます。ここでは文字列としてみます。
    char beaconTsStr[21]; // uint64_tの最大桁数は20桁 + NUL
    snprintf(beaconTsStr, sizeof(beaconTsStr), "%llu", entry.beaconTimestamp);
    jsonObj["bTs"] = beaconTsStr; // "beaconTimestamp" -> "bTs"
    jsonObj["ch"] = entry.channel;
    if (!entry.registrationNo.isEmpty()) { // registrationNoが空でなければ追加
        jsonObj["reg"] = entry.registrationNo; // "registrationNo" -> "reg"
    }
    jsonObj["lat"] = entry.latitude;
    jsonObj["lon"] = entry.longitude;
    jsonObj["pAlt"] = entry.pressureAltitude;
    jsonObj["gAlt"] = entry.gpsAltitude;
}

// RSSI上位N件のRIDデータをJSONで取得 (今回はN=1で利用)
String RemoteIDDataManager::getJsonForTopRSSI(int count, size_t max_log_entries) const {
    // JSONドキュメントのサイズ。max_log_entriesに応じて調整。
    // 1エントリあたり約150-200バイトと仮定。ヘッダ等で+1KB。
    const size_t jsonDocSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(8) * max_log_entries + JSON_ARRAY_SIZE(max_log_entries) + 1024;
    DynamicJsonDocument doc(jsonDocSize);
    JsonObject root = doc.to<JsonObject>(); // ルートは単一のRID情報オブジェクト
    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();
    if (sorted_rids.empty() || count < 1) {
        String output;
        // serializeJson(doc, output); // 空の {} を返す
        // より明確に空であることを示すために空文字列を返すか、エラーを示すJSONを返すこともできる
        return "{}"; // 空のJSONオブジェクト
    }
    String rid_str = sorted_rids[0].second; // 常に最初の1件
    std::vector<RemoteIDEntry> entries_for_rid = getAllDataForRID(rid_str, max_log_entries);
    if (!entries_for_rid.empty()) {
        root["rid"] = rid_str;
        JsonArray entriesArray = root.createNestedArray("entries");
        for (const auto& entry_item : entries_for_rid) {
            JsonObject entryObj = entriesArray.createNestedObject();
            _populateJsonEntry(entryObj, entry_item);
        }
    } else {
        // データはあるが、ログエントリが0件だった場合 (通常は起こりにくい)
        // root["rid"] = rid_str; // RIDだけでも返すか、やはり空にするか
        // root.createNestedArray("entries");
        return "{}"; // 空のJSONオブジェクト
    }
    String output;
    serializeJson(doc, output);
    return output;
}

// 指定登録記号のRIDデータをJSONで取得
String RemoteIDDataManager::getJsonForRegistrationNo(const String& regNo, size_t max_log_entries) const {
    const size_t jsonDocSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(8) * max_log_entries + JSON_ARRAY_SIZE(max_log_entries) + 1024;
    DynamicJsonDocument doc(jsonDocSize);
    JsonObject root = doc.to<JsonObject>();
    if (regNo.isEmpty()) {
        return "{}";
    }
    bool found = false;
    for (const auto& pair : _data_store) {
        const RIDDataContainer& container = pair.second;
        // 各RIDの最新エントリの登録記号をチェック (通常、RIDごとの登録記号は不変のはず)
        if (!container.entries.empty() && container.entries.back().registrationNo == regNo) {
            String rid_str = pair.first;
            std::vector<RemoteIDEntry> entries_for_rid = getAllDataForRID(rid_str, max_log_entries);

            if (!entries_for_rid.empty()) {
                root["rid"] = rid_str;
                JsonArray entriesArray = root.createNestedArray("entries");
                for (const auto& entry_item : entries_for_rid) {
                    JsonObject entryObj = entriesArray.createNestedObject();
                    _populateJsonEntry(entryObj, entry_item);
                }
                found = true;
                break; // 最初に見つかったものだけを対象とする
            }
        }
    }
    if (!found) {
        return "{}"; // 見つからなかった場合は空のJSONオブジェクト
    }
    String output;
    serializeJson(doc, output);
    return output;
}

// オプションのヘルパーメソッドの実装
int RemoteIDDataManager::getLatestChannelForTopRSSI() const {
    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();
    if (!sorted_rids.empty()) {
        String top_rid_str = sorted_rids[0].second;
        RemoteIDEntry latest_entry;
        if (getLatestEntryForRID(top_rid_str, latest_entry)) {
            return latest_entry.channel;
        }
    }
    return -1; // 見つからないかデータなし
}

int RemoteIDDataManager::getLatestChannelForRegistrationNo(const String& regNo) const {
    if (regNo.isEmpty()) return -1;
    for (const auto& pair : _data_store) {
        const RIDDataContainer& container = pair.second;
        if (!container.entries.empty() && container.entries.back().registrationNo == regNo) {
            return container.entries.back().channel; // 最新エントリのチャンネル
        }
    }
    return -1; // 見つからないかデータなし
}
