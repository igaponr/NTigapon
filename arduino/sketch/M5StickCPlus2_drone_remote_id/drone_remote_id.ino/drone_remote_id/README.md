# M5Stack Drone Remote ID Sniffer & Display

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

M5Stackデバイス (M5StickC Plus2 / M5GOなど) を使用して、Wi-Fi Beaconフレームをスキャンし、ASTM F3411-19規格に準拠したドローンのリモートID (RID) データを抽出・表示・記録するツールです。

![Demo Screenshot (Optional)](./docs/images/screenshot.png) <!-- もしスクリーンショットがあれば -->

## 概要

このプロジェクトは、ドローンから発信されるリモートID情報をリアルタイムで傍受し、以下の機能を提供します。

*   **リアルタイム表示:** 受信したリモートID情報 (機体ID、登録記号、位置情報、高度など) をM5StackデバイスのLCDに表示します。
*   **データ管理:** 受信したデータをデバイス内部に時系列で保存し、リングバッファとして管理します。特定のIDのデータを優先的に多く保持することも可能です。
*   **JSON出力:** 蓄積したデータをシリアル経由でJSON形式で出力し、PCなどでさらなる分析や記録に利用できます。
*   **チャンネルスキャン/固定:** Wi-Fiチャンネルを自動でスキャンするモードと、特定のチャンネルに固定するモードを切り替え可能です。

## 機能

*   ASTM F3411-19規格のリモートIDメッセージ（Basic ID, Location/Vector, Authentication等を含むメッセージパック）の解析。
*   複数のリモートIDを同時に追跡し、最新情報をRSSI（受信信号強度）順に表示。
*   M5StickC Plus2 および M5GO (メモリ状況によりキャンバスサイズ調整が必要な場合あり) に対応。
*   効率的なテキスト表示のためのカスタムディスプレイコントローラ (`M5CanvasTextDisplayController`) を使用し、ダブルバッファリングによるちらつきの少ない表示を実現 (メモリが許す限り)。
*   データ管理クラス (`RemoteIDDataManager`) による柔軟なデータ保持。
    *   特定のRIDを「ターゲットRID」として指定し、より多くのログを保持。
    *   他のRIDについても、最新の一定数のログを保持。
*   ボタン操作による機能切り替え:
    *   **ボタンA:** 蓄積データをJSON形式でシリアルポートに出力。
    *   **ボタンB:** Wi-Fiチャンネルスキャンモードとチャンネル固定モードをトグル。
    *   **ボタンC (M5StickC Plus2では電源ボタン):** デバイスをリセット。
*   JSON出力は、以下の2つのモードを選択可能 (コンパイル時設定):
    1.  最もRSSIが高いRIDのデータを送信。
    2.  事前に指定した登録記号を持つRIDのデータを送信。
*   LCD表示ヘッダに、現在のWi-Fiチャンネル、検出RID数、空きヒープメモリ、(Top RSSIの)データエントリ数を表示。

## ハードウェア要件

*   **M5Stackデバイス:**
    *   M5StickC Plus2 (開発および主要テスト環境)
    *   M5GO (動作確認済み、ただしLCDのキャンバスサイズはメモリ制約により調整が必要な場合があります)
    *   その他のESP32ベースのM5Stackデバイスでも、M5Unifiedライブラリが対応していれば動作する可能性がありますが、ピン配置やメモリ量の調整が必要になることがあります。
*   (オプション) リモートIDを発信するドローンまたはテスト用送信機。

## ソフトウェア要件・ライブラリ

*   Arduino IDE または PlatformIO
*   **M5Unified Library:** M5Stackデバイス共通ライブラリ (バージョン `0.2.7` 時点で開発)
    *   [https://github.com/m5stack/M5Unified](https://github.com/m5stack/M5Unified)
*   **M5GFX Library:** M5Unifiedが依存するグラフィックライブラリ (バージョン `0.2.9` 時点で開発)
    *   [https://github.com/m5stack/M5GFX](https://github.com/m5stack/M5GFX)
*   **ArduinoJson Library:** JSONデータの生成に使用 (バージョン `7.4.1` 時点で開発)
    *   [https://arduinojson.org/](https://arduinojson.org/)
*   ESP32ボードマネージャ (M5Stack提供のもの、バージョン `3.2.1` 時点で開発)

## セットアップ

1.  **ライブラリのインストール:**
    Arduino IDEのライブラリマネージャから、上記の `M5Unified`, `M5GFX`, `ArduinoJson` をインストールします。
2.  **ボード設定:**
    *   **M5StickC Plus2の場合:** Arduino IDEのボードメニューから「M5StickCPlus2」を選択します。
    *   **M5GOの場合:** Arduino IDEのボードメニューから「M5Stack-Core-ESP32」(または該当するM5GOのモデル、例: M5Stack-FIRE) を選択します。
3.  **コードの書き込み:**
    このリポジトリのスケッチ (`drone_remote_id.ino` および関連する `.h`, `.cpp` ファイル) をArduino IDEで開き、M5Stackデバイスに書き込みます。
4.  **(オプション) 設定の変更:**
    `drone_remote_id.ino` ファイルの先頭付近にある以下の定数を必要に応じて変更してください。
    *   `SEND_MODE_TOP_RSSI`: `1` でTop RSSIモード、`0` で指定登録記号モード。
    *   `TARGET_REG_NO_FOR_JSON`: 指定登録記号モードの場合のターゲット登録記号。
    *   `MAX_ENTRIES_IN_JSON`: JSON出力時の最大ログエントリ数。
    *   `RemoteIDDataManager dataManager("YOUR_TARGET_RID");`: ターゲットRIDを指定。

## 使い方

1.  プログラムを書き込んだM5Stackデバイスの電源を入れます。
2.  デバイスが自動的にWi-Fiスキャンを開始し、リモートID情報を検出すればLCDに表示します。
3.  **ボタン操作:**
    *   **ボタンA (M5GOでは左ボタン):** 押すと、現在最もRSSIが高いRID（または設定されたターゲットRID）の蓄積データをシリアルポートにJSON形式で出力します。シリアルモニタをPCで開いて確認してください (ボーレート: 115200)。
    *   **ボタンB (M5GOでは中央ボタン):** 押すと、Wi-Fiチャンネルのスキャンモードと、最も信号の強いRIDが検出されたチャンネルに固定するモードを切り替えます。
    *   **ボタンC (M5GOでは右ボタン、M5StickC Plus2では電源ボタン長押しでメニュー):** 押すとデバイスがリセットされます。(M5StickC Plus2の電源ボタンはESP.restart()を直接トリガーします)
4.  **LCD表示:**
    *   ヘッダ: 現在のチャンネル、検出RID数、ヒープメモリ残量、Top RIDのエントリ数を表示。
    *   メインエリア: 検出されたRIDの情報をRSSI降順でリスト表示（機体ID、登録記号、緯度経度、高度、受信時刻など）。

## 既知の課題・今後の改善点

*   **緯度・経度情報:** ドローンがアーム状態になかったり、受信するGNSS(GPS)が不足してアイコンがグリーンにならないと、受信データに有効な緯度・経度が含まれません。
*   **GPS高度の変動:** `gAlt` の値が不安定な場合があります。
*   **メモリ管理:** M5GOなどのデバイスでフル解像度のダブルバッファリングを行うにはメモリが厳しく、キャンバスサイズの調整やシングルバッファ化、PSRAMの活用などの検討が必要です。
*   **セマフォ競合:** 高頻度でデータを受信する場合やJSON出力データ量が多い場合に、スニッファコールバックとメインループの間でセマフォの競合が発生し、警告ログが出力されることがあります。データロスの可能性もゼロではありません。
*   **エラーハンドリング:** より堅牢なエラーハンドリングとユーザーへのフィードバック。

## 貢献

バグ報告、機能提案、プルリクエストを歓迎します！

## ライセンス

このプロジェクトは MIT License の下で公開されています。詳細は `LICENSE` ファイルをご覧ください。

## 参考資料

*   [ESP32 Wi-Fi Sniffer](https://lang-ship.com/blog/work/esp32-wifi-sniffer/) (参考にした技術ブログなど)
*   [M5StackでWi-Fiパケットキャプチャ](https://qiita.com/kobatan/items/dac5d4696d631003e037) (参考にした技術ブログなど)
*   [国土交通省 無人航空機の登録制度](https://www.mlit.go.jp/koku/koku_ua_registration.html)
*   [5.2.周辺機体の発信情報を確認する](https://www.dips-reg.mlit.go.jp/app/page/manual_5_2.html)
*   [StickC-Plus2](https://docs.m5stack.com/ja/core/M5StickC%20PLUS2)ドキュメント
*   [M5GO IoT Kit](https://docs.m5stack.com/ja/core/m5go)ドキュメント
*   [M5Unified](https://github.com/m5stack/M5Unified)ドキュメント
*   [M5GFX](https://github.com/m5stack/M5GFX)ドキュメント
*   [ArduinoJson](https://arduinojson.org/)ドキュメント
