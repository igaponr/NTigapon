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

std::vector<RemoteIDEntry> RemoteIDDataManager::getAllDataForRID(const String& rid) const {
    auto it = _data_store.find(rid);
    if (it != _data_store.end()) {
        // dequeからvectorにコピーして返す
        return std::vector<RemoteIDEntry>(it->second.entries.begin(), it->second.entries.end());
    }
    // 見つからない場合は空のベクターを返す
    return {};
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
    return getAllDataForRID(selected_rid);
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
