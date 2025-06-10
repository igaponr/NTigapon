/**
 * @file RemoteIDDataManager.cpp
 * @brief RemoteIDDataManagerクラスの実装ファイル
 * @details リモートIDデータの管理、格納、およびクエリ機能を提供します
 */
#include "RemoteIDDataManager.h"

/**
 * @brief RemoteIDDataManagerクラスのコンストラクタ
 * @param targetRid 特別扱いするRIDの文字列。このRIDは他のRIDよりも多くのデータエントリを保持します
 */
RemoteIDDataManager::RemoteIDDataManager(const String& targetRid)
    : _target_rid_value(targetRid) {
    // 必要であれば初期化処理をここに追加
}

/**
 * @brief 新しいリモートIDデータを追加します
 * @details 指定されたRIDのデータコンテナが存在しない場合は新規に作成します
 *          データはリングバッファ形式で保存され、古いデータは自動的に削除されます
 * @param rid データを送信したリモートIDの識別子
 * @param rssi RSSI値
 * @param timestamp 受信タイムスタンプ (UNIX秒)
 * @param beaconTimestamp ビーコンタイムスタンプ (マイクロ秒)
 * @param channel 受信Wi-Fiチャンネル
 * @param registrationNo 機体登録記号
 * @param lat 緯度
 * @param lon 経度
 * @param pAlt 気圧高度
 * @param gAlt GPS高度
 */
void RemoteIDDataManager::addData(const String& rid, int rssi, time_t timestamp, uint64_t beaconTimestamp, int channel, const String& registrationNo, float lat, float lon, float pAlt, float gAlt) {
    RemoteIDEntry new_entry(rssi, timestamp, beaconTimestamp, channel, registrationNo, lat, lon, pAlt, gAlt);
    auto it = _data_store.find(rid);
    if (it == _data_store.end()) {
        // 新しいRIDの場合、コンテナを作成してマップに追加
        // このRIDがターゲットRIDか否かで、保存するデータエントリの最大数を決定
        size_t max_size = isTargetRID(rid) ? TARGET_RID_MAX_DATA : OTHER_RID_MAX_DATA;
        // std::piecewise_construct を使用して、キー(String)と値(RIDDataContainer)を効率的に構築
        it = _data_store.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(rid),          // キー(String)のコンストラクタ引数
                                 std::forward_as_tuple(max_size)).first; // 値(RIDDataContainer)のコンストラクタ引数
    }
    // 既存または新規作成したコンテナに新しいデータエントリを追加
    it->second.addEntry(new_entry);
}

/**
 * @brief 指定時刻から過去1分以内にデータ記録があるRIDのリストを取得します
 * @param currentTime 現在時刻 (UNIX秒)。この時刻を基準に過去1分間を評価します
 * @return 過去1分以内にデータがあったRIDのString型リスト
 */
std::vector<String> RemoteIDDataManager::getRIDsWithDataInLastMinute(time_t currentTime) const {
    std::vector<String> result_rids;
    const time_t one_minute_ago = currentTime - ONE_MINUTE_IN_SECONDS;
    for (const auto& pair : _data_store) {
        const RIDDataContainer& container = pair.second;
        if (container.entries.empty()) {
            continue; // データが全くないコンテナはスキップ
        }
        // 最適化: 最新のデータが1分より古いか、または最古のデータが現在時刻より未来の場合
        // (リングバッファの特性上、latest_timestamp が one_minute_ago より前なら、他のエントリも古い可能性が高い)
        // (また、タイムスタンプの異常値チェックも兼ねる)
        if (container.latest_timestamp < one_minute_ago || container.entries.front().timestamp > currentTime) {
            continue;
        }
        // 上記の簡易フィルタを通過した場合、コンテナ内のエントリを実際に確認
        // 1つでも条件に合致するエントリがあれば、そのRIDを結果に追加して次のRIDへ
        for (const auto& entry : container.entries) {
            if (entry.timestamp >= one_minute_ago && entry.timestamp <= currentTime) {
                result_rids.push_back(pair.first);
                break; // このRIDは条件を満たしたので、このRIDに対するループは終了
            }
        }
    }
    return result_rids;
}

/**
 * @brief 指定されたRIDのすべてのデータエントリを時系列順（古いものから新しいもの）で取得します
 * @param rid データを取得したいRIDの識別子
 * @param max_entries 返すエントリの最大数。0の場合は全てのエントリを返します。指定された場合、最新の `max_entries` 件を返します
 * @return 指定されたRIDのデータエントリのベクター。RIDが存在しない場合は空のベクター
 */
std::vector<RemoteIDEntry> RemoteIDDataManager::getAllDataForRID(const String& rid, size_t max_entries) const {
    auto it = _data_store.find(rid);
    if (it != _data_store.end()) {
        const auto& deque_entries = it->second.entries;
        if (max_entries > 0 && deque_entries.size() > max_entries) {
            // 指定された最大エントリ数に基づき、最新のデータのみを抽出
            std::vector<RemoteIDEntry> result_vec;
            result_vec.reserve(max_entries); // 事前にメモリを確保

            // dequeの末尾からmax_entries個の要素を取得するための開始イテレータを計算
            // dequeのイテレータはランダムアクセスではないので、 `end() - N` でアクセス
            auto start_it = deque_entries.end();
            // 実際にコピーする要素数。dequeのサイズがmax_entriesより小さい場合も考慮
            size_t num_to_copy = std::min(deque_entries.size(), max_entries);
            if (num_to_copy > 0) {
                start_it = deque_entries.end() - num_to_copy; // dequeの末尾からnum_to_copy個手前が開始位置
            } else {
                return {}; // コピーする要素がない場合は空のベクターを返す
            }
            // 計算した開始イテレータから末尾までをコピー
            for (auto iter = start_it; iter != deque_entries.end(); ++iter) {
                result_vec.push_back(*iter);
            }
            return result_vec;
        } else {
            // 全件返すか、元々max_entries以下の場合はそのまま全件コピー
            return std::vector<RemoteIDEntry>(deque_entries.begin(), deque_entries.end());
        }
    }
    return {}; // RIDが見つからない場合は空のベクターを返す
}

/**
 * @brief 現在データストアに登録されているRIDの総数を返します
 * @return RIDの総数
 */
int RemoteIDDataManager::getRIDCount() const {
    return _data_store.size();
}

/**
 * @brief RSSIの降順でソートされたRIDのリストを取得するヘルパーメソッド
 *        リストの各要素は {最新RSSI, RID文字列} のペアです
 *        RSSIが同じ場合はRID文字列の昇順でソートされます
 * @return RSSI降順、その後RID文字列昇順でソートされたペアのベクター
 */
std::vector<std::pair<int, String>> RemoteIDDataManager::getSortedRIDsByRSSI() const {
    std::vector<std::pair<int, String>> sorted_rids_list;
    if (_data_store.empty()) {
        return sorted_rids_list; // データストアが空なら空リストを返す
    }
    // データストア内の各RIDについて、最新のRSSIとRID文字列をペアにしてリストに追加
    for (const auto& pair : _data_store) {
        if (!pair.second.entries.empty()) { // エントリが空でないことを確認
            sorted_rids_list.push_back({pair.second.latest_rssi, pair.first});
        }
    }
    // リストをソート
    // 優先順位1: RSSIの降順
    // 優先順位2: RID文字列の昇順 (RSSIが同じ場合)
    std::sort(sorted_rids_list.begin(), sorted_rids_list.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) {
            return a.first > b.first; // RSSI降順
        }
        return a.second < b.second; // RSSIが同じ場合はRID文字列で昇順
    });
    return sorted_rids_list;
}

/**
 * @brief インデックスを指定して、該当するRIDの全データ（時系列順）を取得します
 *        インデックスは、全RIDを最新データのRSSI降順でソートした時の順位に基づきます
 * @param index 取得したいRIDのインデックス (0から始まる)
 * @return 指定されたインデックスのRIDのデータエントリのベクター。インデックスが無効な場合は空のベクター
 */
std::vector<RemoteIDEntry> RemoteIDDataManager::getDataByIndex(int index) const {
    if (index < 0) {
        return {}; // 負のインデックスは無効
    }
    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();
    if (static_cast<size_t>(index) >= sorted_rids.size()) {
        return {}; // インデックスが範囲外の場合は空のベクターを返す
    }
    String selected_rid = sorted_rids[index].second; // インデックスに対応するRID文字列を取得
    return getAllDataForRID(selected_rid, 0); // 0を渡して、そのRIDの全データを取得
}

/**
 * @brief インデックスを指定して、該当するRIDの文字列自体を取得します
 *        インデックスは、全RIDを最新データのRSSI降順でソートした時の順位に基づきます
 * @param index 取得したいRIDのインデックス (0から始まる)
 * @return 指定されたインデックスのRID文字列。インデックスが無効な場合は空文字列
 */
String RemoteIDDataManager::getRIDStringByIndex(int index) const {
    if (index < 0) {
        return ""; // 負のインデックスは無効
    }
    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();
    if (static_cast<size_t>(index) >= sorted_rids.size()) {
        return ""; // インデックスが範囲外の場合は空文字列を返す
    }
    return sorted_rids[index].second; // インデックスに対応するRID文字列を返す
}

/**
 * @brief 特定のRIDがデータストアに存在するかどうかを確認します
 * @param rid 確認したいRIDの識別子
 * @return RIDが存在すればtrue、存在しなければfalse
 */
bool RemoteIDDataManager::hasRID(const String& rid) const {
    return _data_store.count(rid) > 0; // std::map::count は要素数を返す (0 or 1)
}

/**
 * @brief 特定のRIDの最新データエントリを取得します
 * @param rid データを取得したいRIDの識別子
 * @param[out] entry 取得した最新データエントリを格納する参照
 * @return データが取得できた場合はtrue、RIDが存在しないかデータがない場合はfalse
 */
bool RemoteIDDataManager::getLatestEntryForRID(const String& rid, RemoteIDEntry& entry) const {
    auto it = _data_store.find(rid);
    if (it != _data_store.end() && !it->second.entries.empty()) {
        entry = it->second.entries.back(); // dequeの最後の要素が最新
        return true;
    }
    return false; // RIDが見つからないか、エントリがない場合
}

/**
 * @brief データストア内の全てのRIDデータをクリアします
 */
void RemoteIDDataManager::clearAllData() {
    _data_store.clear(); // マップをクリアすることで全データが削除される
}

/**
 * @brief 特定のRIDに関連する全てのデータをクリアします
 * @param rid データをクリアしたいRIDの識別子
 */
void RemoteIDDataManager::clearDataForRID(const String& rid) {
    _data_store.erase(rid); // 指定されたキーのエントリをマップから削除
}

/**
 * @brief RemoteIDEntryの内容をJsonObjectに格納するプライベートヘルパーメソッド
 *        JSONのキー名は短縮形を使用します
 * @param jsonObj 格納先のJsonObject
 * @param entry 格納するRemoteIDEntryデータ
 */
void RemoteIDDataManager::_populateJsonEntry(JsonObject jsonObj, const RemoteIDEntry& entry) const {
    jsonObj["rssi"] = entry.rssi;
    jsonObj["ts"] = entry.timestamp;           // "timestamp" -> "ts" (短縮)
    // beaconTimestamp (uint64_t) は非常に大きな数値になる可能性があるため、文字列として格納
    // snprintfで安全に文字列化する
    char beaconTsStr[21]; // uint64_tの最大桁数(20桁) + NUL終端文字
    snprintf(beaconTsStr, sizeof(beaconTsStr), "%llu", entry.beaconTimestamp);
    jsonObj["bTs"] = beaconTsStr;              // "beaconTimestamp" -> "bTs" (短縮)
    jsonObj["ch"] = entry.channel;
    if (!entry.registrationNo.isEmpty()) {     // registrationNoが空文字列でなければJSONに追加
        jsonObj["reg"] = entry.registrationNo; // "registrationNo" -> "reg" (短縮)
    }
    jsonObj["lat"] = entry.latitude;
    jsonObj["lon"] = entry.longitude;
    jsonObj["pAlt"] = entry.pressureAltitude;
    jsonObj["gAlt"] = entry.gpsAltitude;
}

/**
 * @brief RSSIが最も高い上位 `count` 件のRIDデータを、受け取ったストリームに出力します
 *        現状の実装では `count` は実質1として動作し、最もRSSIが高い1つのRIDのデータを返します
 * @param count 取得する上位RIDの数 (現在は1に固定して利用されることを想定)
 * @param max_log_entries 1つのRIDに対してJSONに含めるデータエントリの最大数
 * @param output_stream 出力ストリームを受け取る
 */
void RemoteIDDataManager::getJsonForTopRSSI(int count, size_t max_log_entries, Print& output_stream) const {
    const size_t jsonDocSize = JSON_OBJECT_SIZE(2) +
                               JSON_OBJECT_SIZE(8) * max_log_entries +
                               JSON_ARRAY_SIZE(max_log_entries) +
                               1024;
    DynamicJsonDocument doc(jsonDocSize); // ここは毎回確保のまま（断片化リスクは残る）
                                          // より良くするにはdocもメンバ変数化してclear & 再利用
    JsonObject root = doc.to<JsonObject>();
    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();
    if (sorted_rids.empty() || count < 1) {
        output_stream.print("{}");
        output_stream.println();
        return;
    }
    String rid_str = sorted_rids[0].second;
    std::vector<RemoteIDEntry> entries_for_rid = getAllDataForRID(rid_str, max_log_entries);
    if (!entries_for_rid.empty()) {
        root["rid"] = rid_str;
        JsonArray entriesArray = root.createNestedArray("entries");
        for (const auto& entry_item : entries_for_rid) {
            JsonObject entryObj = entriesArray.createNestedObject();
            _populateJsonEntry(entryObj, entry_item);
        }
        serializeJson(doc, output_stream);
        output_stream.println();
    } else {
        output_stream.print("{}");
        output_stream.println();
    }
}

/**
 * @brief 指定された登録記号を持つRIDのデータを、受け取ったストリームに出力します
 *        最初に見つかった登録記号に合致するRIDのデータを返します
 * @param regNo 検索する機体登録記号
 * @param max_log_entries JSONに含めるデータエントリの最大数
 * @param output_stream 出力ストリームを受け取る
 */
void RemoteIDDataManager::getJsonForRegistrationNo(const String& regNo, size_t max_log_entries, Print& output_stream) const {
    const size_t jsonDocSize = JSON_OBJECT_SIZE(2) +
                               JSON_OBJECT_SIZE(8) * max_log_entries +
                               JSON_ARRAY_SIZE(max_log_entries) +
                               1024;
    DynamicJsonDocument doc(jsonDocSize);
    JsonObject root = doc.to<JsonObject>();
    if (regNo.isEmpty()) {
        output_stream.print("{}");
        output_stream.println();
        return;
    }
    bool found = false;
    for (const auto& pair : _data_store) { // 'pair' を使ってイテレート
        const String& rid_str_from_pair = pair.first; // RID文字列を取得
        const RIDDataContainer& container = pair.second; // 'container' を宣言
        if (!container.entries.empty() && container.entries.back().registrationNo == regNo) {
            std::vector<RemoteIDEntry> entries_for_rid = getAllDataForRID(rid_str_from_pair, max_log_entries); // 'entries_for_rid' を宣言・初期化

            if (!entries_for_rid.empty()) {
                root["rid"] = rid_str_from_pair;
                JsonArray entriesArray = root.createNestedArray("entries");
                for (const auto& entry_item : entries_for_rid) {
                    JsonObject entryObj = entriesArray.createNestedObject();
                    _populateJsonEntry(entryObj, entry_item);
                }
                serializeJson(doc, output_stream);
                output_stream.println();
                found = true;
                break;
            }
        }
    }
    if (!found) {
        output_stream.print("{}");
        output_stream.println();
    }
}

/**
 * @brief RSSIが最も高いRIDの最新データが受信されたWi-Fiチャンネルを取得します
 * @return 最新のWi-Fiチャンネル番号。該当データがない場合は-1
 */
int RemoteIDDataManager::getLatestChannelForTopRSSI() const {
    std::vector<std::pair<int, String>> sorted_rids = getSortedRIDsByRSSI();
    if (!sorted_rids.empty()) {
        String top_rid_str = sorted_rids[0].second; // RSSIが最も高いRID
        RemoteIDEntry latest_entry;
        if (getLatestEntryForRID(top_rid_str, latest_entry)) {
            return latest_entry.channel; // そのRIDの最新エントリのチャンネルを返す
        }
    }
    return -1; // 該当するRIDがない、またはRIDにデータがない場合
}

/**
 * @brief 指定された登録記号を持つRIDの最新データが受信されたWi-Fiチャンネルを取得します
 * @param regNo 検索する機体登録記号
 * @return 最新のWi-Fiチャンネル番号。該当データがない場合は-1
 */
int RemoteIDDataManager::getLatestChannelForRegistrationNo(const String& regNo) const {
    if (regNo.isEmpty()) {
        return -1; // 登録記号が空の場合は-1
    }
    for (const auto& pair : _data_store) {
        const RIDDataContainer& container = pair.second;
        // 最新エントリの登録記号をチェック
        if (!container.entries.empty() && container.entries.back().registrationNo == regNo) {
            return container.entries.back().channel; // 合致したRIDの最新エントリのチャンネルを返す
        }
    }
    return -1; // 指定された登録記号のRIDが見つからない、またはデータがない場合
}
