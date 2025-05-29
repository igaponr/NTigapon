// RemoteIDDataManager.cpp (実装ファイル)
// ヘッダーでインクルードガードしているので、ここでは#include "RemoteIDDataManager.h"は必須ではないが、
// 分割コンパイルする場合は必要。Arduino IDEでは.inoファイルに直接書くか、タブで追加する。

// C++17未満では、static const メンバー変数の定義がクラス外で必要になることがあります。
// ヘッダーファイル内で初期化済みであれば、この行は不要な場合もあります。
// (Arduino環境のコンパイラバージョンによります)
// const size_t RemoteIDDataManager::TARGET_RID_MAX_DATA; (ヘッダで初期化済み)
// const size_t RemoteIDDataManager::OTHER_RID_MAX_DATA; (ヘッダで初期化済み)


RemoteIDDataManager::RemoteIDDataManager(const String& targetRid)
    : _target_rid_value(targetRid) {
    // 必要であれば初期化処理
}

void RemoteIDDataManager::addData(const String& rid, int rssi, time_t timestamp, float lat, float lon, float pAlt, float gAlt) {
    RemoteIDEntry new_entry(rssi, timestamp, lat, lon, pAlt, gAlt);

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
    time_t one_minute_ago = currentTime - 60; // 1分前の時刻

    for (const auto& pair : _data_store) { // pairは {const String& rid, const RIDDataContainer& container}
        const RIDDataContainer& container = pair.second;
        if (!container.entries.empty()) {
            // コンテナ内のいずれかのデータが1分以内かチェック
            // entriesは時系列順なので、効率化のため末尾からチェックも可能だが、
            // データ数が少ない(特にOTHER_RID)ため、全チェックでも大きな問題はない。
            for (const auto& entry : container.entries) {
                if (entry.timestamp >= one_minute_ago && entry.timestamp <= currentTime) {
                    result_rids.push_back(pair.first); // 条件を満たすRIDを追加
                    break; // このRIDは条件を満たしたので、次のRIDのチェックへ
                }
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
    return {}; // C++11以降のリスト初期化、または std::vector<RemoteIDEntry>()
}

int RemoteIDDataManager::getRIDCount() const {
    return _data_store.size();
}

// ヘルパー関数: RSSI降順でソートされたRIDのリスト {latest_rssi, rid_string} を取得
std::vector<std::pair<int, String>> RemoteIDDataManager::getSortedRIDsByRSSI() const {
    std::vector<std::pair<int, String>> sorted_rids_list;
    if (_data_store.empty()) {
        return sorted_rids_list;
    }

    for (const auto& pair : _data_store) {
        if (!pair.second.entries.empty()) {
            // RIDDataContainerにキャッシュされた最新RSSIを使用
            sorted_rids_list.push_back({pair.second.latest_rssi, pair.first});
        }
    }

    // RSSIの降順でソート。RSSIが同じ場合はRID文字列で昇順ソート（結果の安定性のため）
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
        return {}; // 不正なインデックス
    }

    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();

    if (index >= sorted_rids.size()) {
        return {}; // インデックス範囲外
    }

    String selected_rid = sorted_rids[index].second;
    return getAllDataForRID(selected_rid);
}

String RemoteIDDataManager::getRIDStringByIndex(int index) const {
    if (index < 0) {
        return ""; // 不正なインデックス
    }

    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();

    if (index >= sorted_rids.size()) {
        return ""; // インデックス範囲外
    }
    
    return sorted_rids[index].second;
}

// --- 使いやすさ向上のための追加メソッドの実装 ---
bool RemoteIDDataManager::hasRID(const String& rid) const {
    return _data_store.count(rid) > 0; // countは要素数を返す(0か1)
}

bool RemoteIDDataManager::getLatestEntryForRID(const String& rid, RemoteIDEntry& entry) const {
    auto it = _data_store.find(rid);
    if (it != _data_store.end() && !it->second.entries.empty()) {
        entry = it->second.entries.back(); // dequeの最後の要素が最新
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
