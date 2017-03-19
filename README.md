## NESBot-mini

ESP32(DevkitC)でファミコンのコントローラを自動制御します。4021をシミュレートします。

TASのプレイデータは、FCEUXからLuaで出力したものを fceux-txt2c.rb で変換して使用します。

### ピンアサイン

| ESP32側     | pin | ファミコン側 |
|:------------|:---:|:---------|
| RESET       | IO23 | ファミコンCPU (~RESET端子) |
| NMI         | IO04 | ファミコンCPU (NMI) |
| GND         | GND  | ファミコン外部端子01 (GND) |
| IO02        | IO02 | ファミコン外部端子12 (Strobe) |
| IO18        | IO18 | ファミコン外部端子13 (Joypad 1 D1) |
| IO05        | IO05 | ファミコン外部端子14 (~OE) |

無保証です。
