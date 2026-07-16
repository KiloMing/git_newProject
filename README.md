# STM32H743 Hazardous Gas Monitoring System

## 项目简介

本项目复现并改进基于云平台的化工厂有害气体监测系统。

原项目主控制器为 STM32F429IGT6，本项目迁移到 STM32H743IIT6 阿波罗开发板。

## 计划功能

- 气体数据采集
- LoRa 无线通信
- STM32H743 数据处理
- LCD/LVGL 显示
- 本地阈值判断
- 声光报警
- SD 卡数据存储
- ESP8266 MQTT 通信
- 云平台监控
- Web 端远程控制

## 当前进度

- [x] 建立 Git 仓库
- [ ] SDRAM 测试
- [ ] LCD 基础显示
- [ ] 模拟气体数据
- [ ] 阈值判断
- [ ] RT-Thread
- [ ] LoRa
- [ ] MQTT
- [ ] 云平台

## 硬件平台

- STM32H743IIT6
- 正点原子阿波罗开发板
- 4.3 寸 LCD
- 外部 SDRAM