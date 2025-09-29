# Otomo-e おとも絵

**オープンソースAIフォトフレーム - あなたの写真をe-paperディスプレイに美しく表示**

Otomo-e（おとも絵）は、Bluetooth Low Energy (BLE) を使用してスマートフォンから写真を送信し、e-paperディスプレイに表示するオープンソースプロジェクトです。

## 特徴

- **WebBluetooth対応**: スマートフォンブラウザから直接写真を送信
- **e-paperディスプレイ**: 電子ペーパーで写真を美しく表現
- **複数の描画モード**: Floyd-Steinberg、Atkinson、Ordered、Thresholdディザリング
- **省電力設計**: CR2032電池で長時間動作
- **自動圧縮**: RLE圧縮により効率的なデータ転送

## ハードウェア構成

### 必要な部品

- **マイコン**: Seeed Studio XIAO nRF52840
- **ディスプレイ**: Waveshare 1.54inch e-Paper Module (3色: 白・黒・赤)
- **電源**: CR2032電池

### ピン配置

| nRF52840 | e-Paper Module | 説明 |
|----------|----------------|------|
| 3V3      | VCC           | 電源 |
| GND      | GND           | グランド |
| D6       | DIN (MOSI)    | SPI データ |
| D7       | CLK (SCK)     | SPI クロック |
| D3       | BUSY          | ビジー信号 |
| D2       | CS            | チップセレクト |
| D5       | DC            | データ/コマンド |
| D4       | RST           | リセット |
| D11      | -             | プッシュボタン（プルダウン抵抗付き）|

## ハードウェア設計

### PCB設計

- **設計ファイル**: `hardware/pcb/PhotoFrameMini_v3.*`
- **設計ツール**: KiCad 7.x
- **基板サイズ**: コンパクト設計
- **レイヤー**: 2層基板

### 筐体設計

- **3Dモデル**: `hardware/case/ai_photoframe_mini_case.step`
- **3Dプリント**: `hardware/case/case.stl`、`hardware/case/button.stl`
- **材質**: PLA推奨
- **設計ツール**: Fusion 360

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

### Webアプリケーション

- **技術**: HTML5, CSS3, JavaScript (WebBluetooth API)
- **リポジトリ**: [Otomo-e Web Interface](https://github.com/manomono-23/otomo-e-web)
- **ライブデモ**: [art-frame.manomono.net](http://art-frame.manomono.net/)
- **機能**:
  - 写真選択とプレビュー
  - 複数の描画モード
  - 自動圧縮最適化
  - iOS/Android対応

## セットアップガイド

### 1. ハードウェア組み立て

1. XIAO nRF52840とe-paperモジュールを上記ピン配置に従って接続
2. CR2032電池ホルダーを電源ラインに接続
3. プッシュボタンをD11ピンに接続（プルダウン抵抗必須）

### 2. ファームウェア書き込み

```bash
# Arduino IDEで以下のライブラリをインストール
- Adafruit nRF52 Arduino BSP
- GxEPD2 library
- Adafruit Bluefruit nRF52 libraries

# firmware/otomo-e.inoを開いてXIAO nRF52840に書き込み
```

### 3. Webアプリケーション

```bash
# 公開されているWebアプリを使用（推奨）
# QRコードから http://art-frame.manomono.net/ にアクセス

# または独自のWebサーバーでホスト
# git clone https://github.com/manomono-23/otomo-e-web.git
# cd otomo-e-web
# # HTTPSサーバーでindex.htmlをホスト（WebBluetoothにはHTTPS必須）
```

## 使い方

### 基本操作

1. **電源投入**: 電池を挿入してデバイスを起動
2. **長押し起動**: プッシュボタンを1.5秒以上長押しでQRコード表示
3. **Webアプリアクセス**: QRコードをスキャンしてWebアプリにアクセス
4. **写真選択**: 表示したい写真を選択
5. **デバイス接続**: 「デバイスに接続」ボタンをクリック
6. **送信**: 写真をe-paperディスプレイに送信

### 詳細機能

- **表示モード**: 2色・3色・4色モードから選択
- **描画アルゴリズム**: 4種類のディザリング方式
- **省電力**: 自動スリープ機能（30秒/90秒タイムアウト）

## 技術仕様

### 電力消費
- **動作時**: 約20mA
- **スリープ時**: 約10μA
- **電池寿命**: CR2032で約6ヶ月（1日5回使用想定）

### 通信仕様
- **プロトコル**: Bluetooth Low Energy 4.2+
- **転送速度**: 最大15KB/s（RLE圧縮時）
- **対応OS**: Android, iOS（Bluefy経由）, PC

### ディスプレイ仕様
- **サイズ**: 1.54インチ
- **解像度**: 200×200ピクセル
- **色数**: 3色（白・黒・赤）
- **更新時間**: 約2秒

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
git clone https://github.com/manomono-23/otomo-e.git
cd otomo-e

# ファームウェア開発
# Arduino IDEでfirmware/を開く

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

- **メインファームウェア**: [otomo-e](https://github.com/manomono-23/otomo-e) (このリポジトリ)
- **Webアプリケーション**: [otomo-e-web](https://github.com/manomono-23/otomo-e-web)
- **画像データ送信ツール**: [art-frame.manomono.net](http://art-frame.manomono.net/)

## リンク

- [プロジェクトホームページ](https://github.com/manomono-23/otomo-e)
- [問題報告・機能要求](https://github.com/manomono-23/otomo-e/issues)
- [ディスカッション](https://github.com/manomono-23/otomo-e/discussions)

---

**Otomo-e - あなたの大切な写真を、いつでも、どこでも、美しく。**