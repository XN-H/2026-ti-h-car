# 2026 电赛 H 题：车载平衡滚球运动控制系统

本仓库整理了电赛小车项目的主控源码、设计报告与报告制作文件。

## 内容

- [`report_work/source_package_20260801/TI主控源码_MSPM0G3507`](report_work/source_package_20260801/TI主控源码_MSPM0G3507)：MSPM0G3507 主控工程，包含循迹、编码器、速度控制、IMU、OLED 和任务逻辑。
- [`output/report`](output/report)：H 题设计报告 PDF。
- [`report_work`](report_work)：报告排版脚本和自制图示。脚本保留了当时使用的本机路径，需要按自己的环境调整；原报告模板及部分参考电路图不在仓库中。

`tmp`、报告排版过程中的备份与 QA 导出、下载的参考资料均未纳入版本库。报告 PDF 放在 `output/report`。

仓库收录的是现有文件中的主控小车控制工程和最终报告 PDF。openmv操控与识别未收录

## 使用说明

主控工程使用 TI MSPM0 DriverLib、SysConfig 与 Keil 项目文件，具体器件和接口配置以源码为准。

TI 生成文件保留原版权和许可声明。本仓库未附加统一的开源许可证。
