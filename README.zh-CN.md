# DeskPal 桌面虚拟伙伴

基于 **Seeed Wio Terminal + Edge Impulse + MQTT** 的本科大四物联网项目，作者 **陈哲 / Zhe Chen**。

获得宁波诺丁汉大学 **EEEE3124 Internet of Things（2024/25）Mini Project 的 Best IoT Project Award**，证书日期为 **2024 年 12 月**。

[English](README.md) · [安装与运行](docs/SETUP.md) · [演示步骤](docs/DEMO.md) · [资料清单](docs/MATERIALS.md)

![原型](media/deskpal-prototype.webp)

## 已有功能

设备通过三轴加速度数据进行 `idle / rise / wave` 分类，在 LCD 上显示表情、分类概率、好感度和进度条；通过 SHT40 和板载光传感器感知环境，并用 MQTT 收发环境读数、表情状态、提醒和用户输入的天气。NTP 用于同步时间。

这里的分类对象是**设备运动**。没有恢复出足以证明人体姿态识别准确率、健康改善效果或临床有效性的数据。原课程 PPT 中的商业收入、健康收益、安全设计及未来维护方案不等于固件已经实现的功能。

## 使用

1. 阅读 [安装说明](docs/SETUP.md)，配置 Wio Terminal 开发环境及传感器库。
2. 将本仓库的 `libraries/PoseDetection_inferencing` 安装到 Arduino 的 libraries 目录。
3. 将 `firmware/DeskPal/config.example.h` 复制为同目录下的 `config.h`，填写自己的 Wi-Fi 与局域网 MQTT Broker 地址。
4. 打开 `firmware/DeskPal/DeskPal.ino`，选择 Seeeduino Wio Terminal，编译并上传。
5. 按设备右侧 A 键，在 `weather/response` 主题发送 `{"city":"Ningbo","weather":"cloudy"}`，继续 [演示流程](docs/DEMO.md)。

`config.h` 已加入 Git 忽略规则。此次整理保留原始交互逻辑，仅将网络配置外置；未连接实物测试，也没有宣称恢复了原始编译环境。详见 [已知限制](docs/KNOWN_LIMITATIONS.md)。

## 恢复的材料

代码及模型库、15 页最终 PPT 与 PDF、商业分析笔记 DOCX、原始代码 ZIP、获奖证书、项目图片、MP3/SRT 和剪映工程已归档。完整清单见 [MATERIALS.md](docs/MATERIALS.md)。

**最终 MP4 和三段 MOV 仍未找到。** 原素材路径是 `D:\BaiduNetdiskDownload\iot\`。旧 SharePoint 链接于 2026-09-21 检查时返回 HTTP 404，不能据此判断文件永久删除。剪映工程及恢复线索保留在本地归档中。

原始课件包含学号、私人分享链接和第三方素材，因此与公开代码分开存放；本地归档入口为项目总目录的 `START_HERE.md`。

## 授权

应用代码及新整理说明采用 Apache-2.0。第三方模型运行库保留原许可证。项目照片、证书、学校及厂商标识、历史演示资料不纳入根目录软件许可证。原始文件未被修改。
