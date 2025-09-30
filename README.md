# e-Otomo えおとも

**オープンソースAIフォトフレーム - あなたの写真をe-paperディスプレイに美しく表示**

e-Otomo（えおとも）は、Bluetooth Low Energy (BLE) を使用してスマートフォンから写真を送信し、e-paperディスプレイに表示するオープンソースプロジェクトです。

## 特徴

- **WebBluetooth対応**: スマートフォンブラウザから直接写真を送信
- **e-paperディスプレイ**: 電子ペーパーで写真を美しく表現
- **複数の描画モード**: Floyd-Steinberg、Atkinson、Ordered、Thresholdディザリング
- **定期更新モード**: 複数の写真を自動で切り替え表示（3分間隔）
- **省電力設計**: CR2032電池で長時間動作
- **自動圧縮**: RLE圧縮により効率的なデータ転送

## ハードウェア構成

### 必要な部品

- **マイコン**: Seeed Studio XIAO nRF52840
- **ディスプレイ**: Waveshare 1.54inch e-Paper Module (3色: 白・黒・赤)
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
- **設計ツール**: KiCad 7.x
- **基板サイズ**: コンパクト設計
- **レイヤー**: 2層基板

### 筐体設計

- **3Dモデル**: `hardware/case/上部.stl`、`hardware/case/下部.stl`、`hardware/case/ボタン.stl`
- **材質**: PLA推奨
- **設計ツール**: Fusion 360
- **透明アクリル**: `hardware/case/透明アクリル1mm.dxf`（ディスプレイカバー用）

## ソフトウェア構成

### ファームウェア (Arduino)

- **開発環境**: Arduino IDE with Adafruit nRF52 BSP
- **ライブラリ**:
  - GxEPD2 (e-paper display)
  - Adafruit Bluefruit nRF52 libraries
- **機能**:
  - BLE通信
  - e-paperディスプレイ制御
  - 省電力管理
  - QRコード表示
  - RLE圧縮対応
  - 定期更新モード（3分間隔）

### Webアプリケーション

- **技術**: HTML5, CSS3, JavaScript (WebBluetooth API)
- **リポジトリ**: [e-Otomo Web Interface](https://github.com/manomono-23/e-otomo-web)
- **ライブデモ**: [art-frame.manomono.net](http://art-frame.manomono.net/)
- **機能**:
  - 写真選択とプレビュー
  - 複数の描画モード
  - 単発送信モード・定期更新モード
  - 自動圧縮最適化
  - iOS/Android対応

## セットアップガイド

### 1. ハードウェア組み立て

1. XIAO nRF52840とe-paperモジュールを上記ピン配置に従って接続
2. CR2032電池ホルダーを電源ラインに接続
3. プッシュボタンをD2ピンに接続（プルダウン抵抗必須）
4. 赤色LEDをD0ピンに接続（電流制限抵抗必須）

### 2. ファームウェア書き込み

```bash
# Arduino IDEで以下のライブラリをインストール
- Adafruit nRF52 Arduino BSP
- GxEPD2 library
- Adafruit Bluefruit nRF52 libraries

# firmware/e-Otomo/e-Otomo.inoを開く
# ディスプレイタイプを選択（DISPLAY_2COLOR または DISPLAY_3COLOR）
# デバイスIDを設定（DEVICE_ID）
# XIAO nRF52840に書き込み
```

### 3. Webアプリケーション

```bash
# 公開されているWebアプリを使用（推奨）
# QRコードから http://art-frame.manomono.net/ にアクセス

# または独自のWebサーバーでホスト
# git clone https://github.com/manomono-23/e-otomo-web.git
# cd e-otomo-web
# # HTTPSサーバーでindex.htmlをホスト（WebBluetoothにはHTTPS必須）
```

## 使い方

### 初回セットアップ

1. **電池を挿入**: CR2032電池をデバイスに挿入
2. **長押し起動**: プッシュボタンを1.5秒以上長押し
3. **QRコード確認**: ディスプレイにQRコードとデバイス名が表示されます
4. **Webアプリアクセス**: スマートフォンでQRコードをスキャンしてWebアプリにアクセス

### 単発送信モード - 1枚の写真を送信

1. **Webアプリを開く**: ブラウザで http://art-frame.manomono.net/ にアクセス
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
   - **4色**: 白・黒・赤・黄（対応ディスプレイのみ）
6. **プレビュー確認**: 選択した描画モードでリアルタイムプレビューを確認
7. **デバイスに接続**: 「デバイスに接続」ボタンをクリック
8. **デバイス選択**: Bluetoothダイアログで "e-otomo" で始まるデバイスを選択
9. **送信**: 「送信」ボタンをクリックして写真を転送
10. **表示完了**: e-paperディスプレイに写真が表示されます（約15秒）

### 定期更新モード（AUTO） - 複数の写真を自動切り替え

1. **Webアプリを開く**: ブラウザで http://art-frame.manomono.net/ にアクセス
2. **モード選択**: 「写真を定期更新」（AUTO）タブを選択
3. **画像スロット設定**: 最大3枚の写真を設定
   - **スロット1**: 「画像を選択」から1枚目の写真を選択
   - **スロット2**: 「画像を選択」から2枚目の写真を選択
   - **スロット3**: 「画像を選択」から3枚目の写真を選択
4. **描画モード選択**: 各スロットごとに描画モードを個別に設定可能
5. **プレビュー確認**: 各スロットのプレビューを確認
6. **デバイスに接続**: 「デバイスに接続」ボタンをクリック
7. **デバイス選択**: Bluetoothダイアログで "e-otomo" で始まるデバイスを選択
8. **送信**: 「画像スロットを送信」ボタンをクリック
9. **転送完了待機**: 3枚分の写真データが順次転送されます（約45秒）
10. **自動切り替え開始**: デバイスは3分間隔で自動的に写真を切り替え表示します

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

#### 青色LED（内蔵）

| 状態 | LEDパターン | 説明 |
|------|------------|------|
| スリープ中 | 常時点灯 | ディープスリープ状態 |
| 動作中 | 消灯 | 通常動作中 |

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

1. プッシュボタンを長押ししてBluetooth接続を開始
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

#### 定期更新が動作しない

1. 全てのスロットに画像が正しく送信されたか確認
2. デバイスログで「Periodic update enabled」が表示されているか確認
3. 3分間待機して切り替わるか確認

## 技術仕様

### 電力消費
- **動作時**: 約20mA
- **スリープ時**: 約10μA
- **電池寿命**: CR2032で約6ヶ月（1日5回使用想定）

### 通信仕様
- **プロトコル**: Bluetooth Low Energy 4.2+
- **転送速度**: 最大15KB/s（RLE圧縮時）
- **対応OS**: Android, iOS（Bluefy経由）, PC
- **最大転送サイズ**: 単発モード 10KB、定期更新モード 30KB（3スロット）

### ディスプレイ仕様
- **サイズ**: 1.54インチ
- **解像度**: 200×200ピクセル
- **色数**: 3色（白・黒・赤）
- **更新時間**: 約2秒
- **定期更新間隔**: 3分（固定）

### データ圧縮
- **アルゴリズム**: Run-Length Encoding (RLE)
- **圧縮率**: 平均60-90%（画像内容による）
- **ピクセル値形式**: [length, value]ペア

## 開発への貢献

このプロジェクトはオープンソースです。貢献を歓迎します！

### 貢献方法

1. このリポジトリをフォーク
2. フィーチャーブランチを作成 (`git checkout -b feature/amazing-feature`)
3. 変更をコミット (`git commit -m 'Add amazing feature'`)
4. ブランチにプッシュ (`git push origin feature/amazing-feature`)
5. プルリクエストを作成

### 開発環境

```bash
# リポジトリをクローン
git clone https://github.com/manomono-23/e-otomo.git
cd e-otomo

# ファームウェア開発
# Arduino IDEでfirmware/e-Otomo/を開く

# Webアプリ開発
# web/index.htmlをブラウザで開く
```

## ライセンス

このプロジェクトは MIT License の下でライセンスされています。詳細は [LICENSE](LICENSE) ファイルを参照してください。

## 作者

- **manomono** - [GitHub](https://github.com/manomono-23)

## 謝辞

- [Adafruit](https://www.adafruit.com/) - nRF52ライブラリ
- [Waveshare](https://www.waveshare.com/) - e-paperディスプレイ
- [Seeed Studio](https://www.seeedstudio.com/) - XIAO nRF52840

## 関連リポジトリ

- **メインファームウェア**: [e-Otomo](https://github.com/manomono-23/e-otomo) (このリポジトリ)
- **Webアプリケーション**: [e-Otomo-web](https://github.com/manomono-23/e-otomo-web)
- **画像データ送信ツール**: [art-frame.manomono.net](http://art-frame.manomono.net/)

## リンク

- [プロジェクトホームページ](https://github.com/manomono-23/e-otomo)
- [問題報告・機能要求](https://github.com/manomono-23/e-otomo/issues)
- [ディスカッション](https://github.com/manomono-23/e-otomo/discussions)

---

**e-Otomo（えおとも）- あなたの大切な写真を、いつでも、どこでも、美しく。**