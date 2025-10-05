# e-Otomo えおとも

**オープンソースAIフォトフレーム - あなたの写真をe-paperディスプレイに美しく表示**

e-Otomo（えおとも）は、nRF52840を使用したデバイスで、Bluetooth Low Energy (BLE) を使用してスマートフォン等から写真を送信し、e-paperディスプレイに表示するオープンソースプロジェクトです。

## 特徴

- **WebBluetooth対応**: スマートフォンブラウザから直接写真を送信
- **e-paperディスプレイ**: 電子ペーパーで写真を美しく表現
- **複数の描画モード**: Floyd-Steinberg、Atkinson、Ordered、Thresholdディザリング
- **省電力設計**: CR2032電池で長時間動作
- **自動圧縮**: RLE圧縮により効率的なデータ転送

## ハードウェア構成

### 必要な部品

- **マイコン**: Seeed Studio XIAO nRF52840
- **ディスプレイ**:
  - 3色（白・黒・赤）: [Waveshare 1.54inch e-Paper Module B](https://www.waveshare.com/1.54inch-e-Paper-B.htm)
  - 2色（白・黒）: [1.54" 200x200 E-Ink Display Panel #-765](https://buyepaper.com/products/-765)
- **電源**: CR2032電池

### ピン配置

| nRF52840 | 接続先 | 説明 |
|----------|--------|------|
| 3V3      | e-Paper VCC | 電源 |
| GND      | e-Paper GND | グランド |
| D6       | e-Paper DIN (MOSI) | SPI データ |
| D7       | e-Paper CLK (SCK) | SPI クロック |
| D3       | e-Paper BUSY | ビジー信号 |
| D1       | e-Paper CS | チップセレクト |
| D5       | e-Paper DC | データ/コマンド |
| D4       | e-Paper RST | リセット |
| D2       | プッシュボタン | ウェイクアップ（外部プルダウン抵抗付き）|
| D0       | 赤色LED | ステータス表示 |
| LED_BLUE | 青色LED（内蔵） | ディープスリープ表示 |

## ハードウェア設計

### PCB設計

- **設計ファイル**: `hardware/pcb/PhotoFrameMini_v3.*`
- **設計ツール**: KiCad 8.x
- **レイヤー**: 2層基板

### 筐体設計

- **3Dモデル**: `hardware/case/*`
- **材質**: PLA推奨
- **透明アクリル**: `hardware/case/Middle_Acrylic_2mm.svg`（ディスプレイカバー用）
- **透明アクリル**: `hardware/case/Panel_Acrylic_1mm.svg`（ケース中間層用）

## ソフトウェア構成

### ファームウェア (Arduino)

- **開発環境**: Arduino IDE
- **ライブラリ**:
  - GxEPD2 (e-paper display)
  - Adafruit Bluefruit nRF52 libraries
- **機能**:
  - BLE通信
  - e-paperディスプレイ制御
  - RLE圧縮対応

### Webアプリケーション

- **技術**: HTML5, CSS3, JavaScript (WebBluetooth API)
- **リポジトリ**: [e-Otomo Web Interface](https://github.com/manomono-23/e-otomo-web)
- **ライブデモ**: [e-otomo.manomono.net](http://e-otomo.manomono.net/)
- **機能**:
  - 写真選択とプレビュー
  - 複数の描画モード
  - 自動圧縮最適化
  - iOS/Android対応

## 使い方

### 初回セットアップ

1. **電池を挿入**: CR2032電池をデバイスに挿入
2. **長押し起動**: プッシュボタンを1.5秒以上長押し
3. **QRコード確認**: ディスプレイにQRコードとデバイス名が表示されます
4. **Webアプリアクセス**: スマートフォンでQRコードをスキャンしてWebアプリにアクセス

### 単発送信モード - 1枚の写真を送信

1. **Webアプリを開く**: ブラウザで http://e-otomo.manomono.net/ にアクセス
2. **モード選択**: 「今すぐ1枚送信」タブを選択
3. **写真を選択**: 「写真を選択」ボタンをクリックして表示したい写真を選ぶ
4. **描画モード選択**: 4種類の描画モードから好みのスタイルを選択
   - **Floyd-Steinberg**: 高品質なディザリング（デフォルト）
   - **Atkinson**: 柔らかな印象のディザリング
   - **Ordered**: パターン化されたディザリング
   - **Threshold**: シンプルな2値化
5. **色モード選択**: 2色/3色/4色モードから選択
   - **2色**: 白・黒
   - **3色**: 白・黒・赤
6. **プレビュー確認**: 選択した描画モードでリアルタイムプレビューを確認
7. **デバイスに接続**: 「デバイスに接続」ボタンをクリック
8. **デバイス選択**: Bluetoothダイアログで "e-otomo" で始まるデバイスを選択
9. **送信**: 「送信」ボタンをクリックして写真を転送
10. **表示完了**: e-paperディスプレイに写真が表示されます（約15秒）

### プッシュスイッチの動作

- **短押し（1.5秒未満）**: ディスプレイ更新なしでBluetooth接続開始
  - 電力を節約したい場合に使用
  - QRコードは表示されません
- **長押し（1.5秒以上）**: QRコード表示 + Bluetooth接続開始
  - 初回セットアップ時や、新しいデバイスで接続する場合に使用
  - ディスプレイにQRコードとデバイス名が表示されます

### LED ステータス表示

#### 赤色LED（D0）

| 状態 | LEDパターン | 説明 |
|------|------------|------|
| 起動時 | 10ms点灯・100ms消灯 × 5回 | デバイス起動完了 |
| 動作中 | 10ms点灯・2000ms消灯の繰り返し | Bluetooth接続待機・データ処理中 |
| スリープ前 | 10ms点灯・100ms消灯 × 3回 | 省電力モード移行 |

### 省電力モード

デバイスは以下の条件で自動的にディープスリープに入ります：

- **接続なしタイムアウト**: 起動後30秒以内にBluetooth接続がない場合
- **接続後タイムアウト**: Bluetooth接続後90秒間活動がない場合

ディープスリープ状態では：
- 消費電力が約10μAに削減
- プッシュボタンを押すことで起動
- 青色LEDが常時点灯（ディープスリープ表示）

### トラブルシューティング

#### デバイスが見つからない

1. プッシュボタンを押してBluetooth接続を開始
2. デバイスのLEDが点滅していることを確認
3. スマートフォンのBluetooth設定でデバイスが検出されることを確認
4. ブラウザをリロードして再試行

#### 写真が正しく表示されない

1. 描画モードを変更して再送信
2. 写真のコントラストや明るさを調整
3. 色モードがディスプレイタイプと一致しているか確認

#### 転送が途中で止まる

1. スマートフォンとデバイスの距離を近づける（1m以内推奨）
2. 他のBluetooth機器との干渉を避ける
3. デバイスを再起動（電池を抜き差し）

## 技術仕様

### 電力消費
- **動作時**: 約20mA
- **スリープ時**: 約10μA
- **電池寿命**: CR2032で約6ヶ月（1日1回使用想定）

### 通信仕様
- **プロトコル**: Bluetooth Low Energy 4.2+
- **転送速度**: 最大15KB/s（RLE圧縮時）
- **対応OS**: Android, iOS（Bluefy経由）, PC

### ディスプレイ仕様
- **サイズ**: 1.54インチ
- **解像度**: 200×200ピクセル
- **色数**: 3色（白・黒・赤） / 2色（白・黒）

### データ圧縮
- **アルゴリズム**: Run-Length Encoding (RLE)
- **圧縮率**: 平均60-90%（画像内容による）
- **ピクセル値形式**: [length, value]ペア

## 開発への貢献

このプロジェクトはオープンソースです。貢献を歓迎します！

## ライセンス

このプロジェクトは MIT License の下でライセンスされています。詳細は [LICENSE](LICENSE) ファイルを参照してください。

## 謝辞

- [Adafruit](https://www.adafruit.com/) - nRF52ライブラリ
- [Waveshare](https://www.waveshare.com/) - e-paperディスプレイ
- [Seeed Studio](https://www.seeedstudio.com/) - XIAO nRF52840

## 関連リポジトリ

- **メインファームウェア**: [e-Otomo](https://github.com/manomono-23/e-otomo) (このリポジトリ)
- **Webアプリケーション**: [e-Otomo-web](https://github.com/manomono-23/e-otomo-web)
- **画像データ送信ツール**: [e-otomo.manomono.net](http://e-otomo.manomono.net/)

## リンク

- [プロジェクトホームページ](https://kawashimadaichi.tokyo/e-otomo)
