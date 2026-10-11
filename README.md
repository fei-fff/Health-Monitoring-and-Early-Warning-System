# 基于 STM32F407 的嵌入式设备健康监测与预警终端

一个 STM32 端 + PC 端联动的嵌入式健康监测项目。MCU 负责采集芯片内部温度与内部参考电压、估算供电电压、做滤波和阈值告警，并通过 LCD 实时显示、通过串口周期上报；PC 端用 Python 接收数据、落盘 CSV，并做滚动窗口异常检测，异常时回发命令形成闭环控制。

## 功能特性

- **ADC 采集**：读取内部温度传感器和 `VREFINT`，估算供电电压 `VDD`
- **数据滤波**：一阶低通滤波，抑制采样噪声
- **阈值告警**：温度上限、供电电压下限，越限产生事件计数并上报告警帧
- **LCD 显示**：FSMC 16-bit 8080 接口驱动，三页面（实时数据 / 阈值 / 事件）
- **周期调度**：TIM6 产生 1 ms 时基，1 秒非阻塞采样节拍
- **串口通信**：USART1 `115200-8-N-1` 行协议，1 Hz 上报采样数据
- **命令控制**：支持页面切换、阈值下发、LED 控制，并回传 ACK
- **PC 端异常检测**：滚动窗口鲁棒 z-score（MAD），检测温升 / 欠压异常并回发命令

## 技术栈

- MCU：STM32F407ZGT6（正点原子探索者 F407）
- 固件：C、STM32CubeF4 HAL 库、Keil MDK v5、ST-Link V2
- 上位机：Python 3、pyserial

## 目录结构

```text
.
├── pc/                     # PC 端 Python 工具
│   ├── protocol.py         # 帧解析 / 组帧
│   ├── pc_monitor.py       # 串口接收 + CSV 记录
│   ├── live_monitor.py     # 实时监控 + 异常检测 + 回发命令
│   ├── ai_anomaly.py       # 滚动窗口异常检测算法
│   ├── analyze_csv.py      # 离线 CSV 分析
│   └── requirements.txt
├── project1/               # Keil / CubeMX 工程
│   ├── App/                # 应用层（健康状态、UI）
│   ├── Protocol/           # 通信协议
│   ├── User/               # main.c 等
│   ├── Drivers/BSP/        # ADC / LCD / LED / Timer 驱动
│   ├── Drivers/SYSTEM/     # 正点原子基础层（时钟 / 延时 / 串口）
│   └── Projects/MDK-ARM/   # Keil 工程文件
└── .gitignore
```

## 快速开始

### 1. 编译烧录 MCU

用 Keil MDK v5 打开：

```text
project1/Projects/MDK-ARM/atk_f407.uvprojx
```

编译后通过 ST-Link V2 下载到开发板。烧录成功后 LCD 每 1 秒刷新一次实时数据，UART 每 1 秒发送一帧采样数据。

### 2. 运行 PC 端

```powershell
cd pc
python -m pip install -r requirements.txt

# 查看串口
python pc_monitor.py --list-ports

# 串口采集 + CSV 记录
python pc_monitor.py --port COM7 --csv samples.csv

# 实时监控 + 异常检测
python live_monitor.py --port COM7 --window 30 --threshold 3.5
```

## 通信协议

所有帧以 `\r\n` 结尾。

**上行采样帧**

```text
S,<uptime_ms>,<temp_x10>,<vdd_mv>,<event_count>,<alarm_flags>
```

**上行告警帧**

```text
A,<code>,<uptime_ms>,<value>
```

**下行命令**

```text
CMD,PAGE,<0|1|2>        # 切换页面
CMD,TH,TEMP,<value>     # 设置温度上限，单位 0.1℃
CMD,TH,VDD,<value>      # 设置 VDD 下限，单位 mV
CMD,LED,<0|1>           # 控制 LED，1 点亮，0 熄灭
```

**下行应答**

```text
ACK,<cmd>,<code>        # code: 0 成功，1 未知命令，2 参数越界
```

**告警码**

| code | 含义 |
| --- | --- |
| 1 | TEMP_HIGH |
| 2 | VDD_LOW |

默认阈值：温度上限 `65.0 ℃`，VDD 下限 `2800.0 mV`。
