# KBD-USB2PS2

中文 | [English](README-en.md)

![kbd-usb2ps2](kbd-usb2ps2.png)

转化 USB HID 键盘协议为 PS/2 键盘协议 (scan code set 2)

这个项目的最初想法来源于 [usb2ps2](https://github.com/limao693/usb2ps2)，但是这个项目没有实现 PS/2 键盘协议的 **启动初始化**，**输入速率** 和 **输入延迟**，还有就是使用的 *USB Host Shield* 库太旧了，这个新的项目实现了上提到的这些问题。

## 硬件需求

* Arduino Uno Rev3
* USB Host Shield V2.0

## 如何使用

1. 安装 [Arduino IDE](https://www.arduino.cc/en/Main/Software)
2. 在 Arduino IDE 中安装 [USB Host Shield Library 2.0](https://github.com/felis/USB_Host_Shield_2.0)
3. 将这个项目 (kbd-usb2ps2.ino) 写入 Arduino Uno Rev3
4. 将 USB Host Shield V2.0 组装在 Arduino Uno Rev3 上
5. 将 PS/2 的 **数据线** 和 **时钟线** 分别插在 Arduino 的 (**4**, **2**) 上

:warning: 注意: 在电脑启动前，你就应该先把 PS/2 接口插在电脑上，因为 PS/2 接口不支持 [热插拔](https://en.wikipedia.org/wiki/PS/2_port#Hotplugging).

## 键盘兼容性

:heavy_check_mark: : 支持, :x: : 不支持 

* HHKB Professional Type-S :x:
* CHERRY G80-3494LWCEU-0 :heavy_check_mark:
* ...

USB Host Shield V2.0 好像[不支持自带 Hub 口的键盘](https://github.com/felis/USB_Host_Shield_2.0/issues/518)，所以如果你的键盘自带 Hub 口，那插在这个系统上是不能正常使用的。

:rose: 欢迎提交 PR 来完善这个键盘支持列表。

## TODO

* [ ] 支持 Num Lock
* [x] BIOS 启动时识别为 PS/2 键盘设备

## Reference

* [The PS/2 Mouse/Keyboard Protocol](https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf), Adam Chapweske
* [USB HID to PS/2 Scan Code Translation Table](https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/translate.pdf), Microsoft
