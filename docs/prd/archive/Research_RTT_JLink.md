# 技术调研报告: SEGGER RTT via J-Link SDK 集成方案

> 调研人: 集成开发工程师
> 日期: 2026-05-31
> 版本: v1.0
> 项目: EmbedDebug

---

## 一、调研目标

研究 SEGGER RTT (Real-Time Transfer) 通过 J-Link SDK DLL 实现的方案, 为 EmbedDebug 项目中 `RttConnection` 类的设计提供完整的技术依据。核心任务是让上位机通过 J-Link 调试器与目标 MCU 进行高速双向数据通信, 替代传统串口调试输出。

---

## 二、J-Link SDK DLL 概况

### 2.1 DLL 文件

J-Link 安装目录 `E:/Embedded/Tool/SEGGER_IOT/JLink_V932` 下提供两个核心 DLL:

| 文件 | 架构 | 说明 |
|------|------|------|
| `JLinkARM.dll` | 32-bit (x86) | 32位平台使用 |
| `JLink_x64.dll` | 64-bit (x64) | **本项目使用此DLL** |

EmbedDebug 是 64-bit 应用 (MinGW x64 + Qt 6.8.3), 因此使用 `JLink_x64.dll`。

### 2.2 调用约定

- DLL 中大部分函数使用 **`__cdecl`** 调用约定
- 部分 RTT/Trace 相关函数使用 **`__stdcall`** 调用约定 (Windows 平台)
- PyLink 源码中明确列出的 `__stdcall` 函数包括:
  - `JLINK_RTTERMINAL_Control`
  - `JLINK_RTTERMINAL_Read`
  - `JLINK_RTTERMINAL_Write`
  - `JLINK_Configure`
  - `JLINK_ExecCommand`
  - `JLINK_EraseChip`
  - `JLINK_SPI_Transfer`
  - 以及其他约 40 个函数 (完整列表见附录)

> **关键发现**: RTT 三大核心函数都是 `__stdcall` 约定, 这在 Qt `QLibrary` 动态加载时需要特别注意, `QLibrary::resolve()` 默认按 `__cdecl` 解析符号, 对 `__stdcall` 函数在 x64 平台上实际上没有区别 (x64 只有 `__fastcall` 一种调用约定), 但在 Win32 平台上需要额外处理。

**结论**: 因为本项目编译为 x64, x64 下所有调用约定统一为 Microsoft x64 calling convention, 无需区分 cdecl/stdcall。

### 2.3 SDK 头文件缺失

本机安装的 J-Link V932 **不包含** C/C++ 头文件 (`JLinkARM.h` 或 `JLink.h`), 仅包含:
- DLL 文件 (`JLink_x64.dll`, `JLinkARM.dll`)
- 文档 (`UM08001_JLink.pdf`, `UM08003_JFlash.pdf`)
- 少量示例脚本和 DCC 示例

这意味着我们必须通过 **QLibrary 动态加载** + **手动声明函数指针 typedef** 的方式使用 DLL。

---

## 三、DLL 核心函数签名 (Typedef 函数指针)

基于 PyLink 开源项目源码、JLinkARM.def 导出定义文件、以及 CSDN 社区文档, 以下是我们需要的所有函数签名。

### 3.1 连接管理函数

```cpp
// ============================================================
// DLL 生命周期
// ============================================================

// 获取 DLL 版本号 (BCD编码, 如 V932 = 0x00009320)
// 返回: DLL版本号
typedef int (*JLINK_GetDLLVersion_t)(void);

// 打开 J-Link 连接 (初始化 DLL 内部状态)
// 返回: 0=成功, <0=错误码
typedef int (*JLINK_Open_t)(void);

// 关闭 J-Link 连接, 释放资源
// 返回: 0=成功
typedef void (*JLINK_Close_t)(void);

// 检查 J-Link 是否已打开
// 返回: 1=已打开, 0=未打开
typedef int (*JLINK_IsOpen_t)(void);

// 执行 J-Link 命令字符串 (如 "device = STM32F407VG")
// 参数: sCmd - 命令字符串 (ANSI)
// 返回: 0=成功, <0=错误
typedef int (*JLINK_ExecCommand_t)(const char* sCmd);
```

### 3.2 仿真器选择函数

```cpp
// ============================================================
// 仿真器 (Emulator) 选择
// ============================================================

// 获取当前连接的 J-Link 设备数量
// 参数: aConnInfo - 输出数组, 指向 JLinkConnectInfo 结构
//       MaxInfo - 数组最大长度
// 返回: 实际找到的设备数
typedef int (*JLINK_EMU_GetList_t)(int HostIFs, void* aConnInfo, int MaxInfo);

// 通过 USB 索引选择 J-Link
// 参数: Index - J-Link 设备索引 (从0开始)
// 返回: 0=成功, <0=错误
typedef int (*JLINK_EMU_SelectByIndex_t)(int Index);

// 通过 USB 序列号选择 J-Link
// 参数: SN - J-Link 序列号
// 返回: 0=成功, <0=错误
typedef int (*JLINK_EMU_SelectByUSBSN_t)(uint32_t SN);

// 通过 IP 地址选择 J-Link
// 参数: sIPAddr - IP地址字符串 (如 "192.168.1.100")
//       Port - 端口号 (默认 19020)
// 返回: 0=成功, <0=错误
typedef int (*JLINK_EMU_SelectIP_t)(const char* sIPAddr, int Port);
```

### 3.3 目标接口与速度函数

```cpp
// ============================================================
// 目标接口 & 速度配置
// ============================================================

// 选择目标调试接口
// 参数: tif - 接口类型: 0=JTAG, 1=SWD, 3=FINE, 4=ICSP, 5=SPI, 6=C2
// 返回: 0=成功, <0=错误
typedef int (*JLINK_TIF_Select_t)(int tif);

// 获取目标接口支持列表
// 参数: pAvailable - 输出, 位掩码表示可用接口
// 返回: 0=成功
typedef int (*JLINK_TIF_GetAvailable_t)(void* pAvailable);

// 设置调试接口通信速度
// 参数: Speed - 速度 (kHz), 特殊值: -1=自动, 0=自适应
// 返回: 无
typedef void (*JLINK_SetSpeed_t)(int Speed);

// 获取当前速度
// 返回: 当前速度 (kHz)
typedef int (*JLINK_GetSpeed_t)(void);

// 设置最大速度
// 参数: Speed - 最大速度 (kHz)
typedef void (*JLINK_SetMaxSpeed_t)(int Speed);
```

### 3.4 目标连接函数

```cpp
// ============================================================
// 目标 MCU 连接
// ============================================================

// 连接到目标 MCU (需要先 SelectDevice 和 TIF_Select)
// 返回: 0=成功, <0=错误码
//   -1: 未指定设备
//   -256: EMU_NO_CONNECTION
//   -257: EMU_COMM_ERROR
//   -259: VCC_FAILURE (目标未供电)
//   -261: NO_CPU_FOUND
typedef int (*JLINK_Connect_t)(void);

// 检查是否已连接到目标
// 返回: 1=已连接, 0=未连接
typedef int (*JLINK_IsConnected_t)(void);

// 复位目标 MCU
// 参数: strategy - 复位策略 (0=Normal, 2=ResetPin, 3=ConnectUnderReset)
// 返回: 0=成功
typedef int (*JLINK_Reset_t)(void);

// 获取目标硬件状态 (电压, 引脚状态等)
// 参数: pStatus - 输出 JLinkHardwareStatus 结构
//   struct { uint16_t VTarget; uint8_t tck,tdi,tdo,tms,tres,trst; }
// 返回: 0=成功
typedef int (*JLINK_GetHWStatus_t)(void* pStatus);
```

### 3.5 RTT 核心函数

```cpp
// ============================================================
// RTT (Real-Time Transfer) 核心 API
// ============================================================

// RTT 控制命令 (启动/停止/查询状态)
// 参数: Cmd - 命令编号 (见下方 JLinkRTTCommand 枚举)
//       pConfig - 配置结构指针, 命令相关:
//         START  -> JLinkRTTerminalStart*
//         STOP   -> NULL
//         GETDESC -> JLinkRTTerminalBufDesc*
//         GETNUMBUF -> int*
//         GETSTAT -> JLinkRTTerminalStatus*
// 返回: 0=成功, <0=错误
//   -2: RTT Control Block 未找到
typedef int (*JLINK_RTTERMINAL_Control_t)(int Cmd, void* pConfig);

// 从 RTT 上行缓冲区读取数据 (Target -> Host)
// 参数: BufferIndex - 通道索引 (从0开始)
//       pBuffer - 接收缓冲区指针
//       BufferSize - 要读取的最大字节数
// 返回: 实际读取的字节数 (0=无数据, <0=错误)
typedef int (*JLINK_RTTERMINAL_Read_t)(int BufferIndex, char* pBuffer, int BufferSize);

// 向 RTT 下行缓冲区写入数据 (Host -> Target)
// 参数: BufferIndex - 通道索引 (从0开始)
//       pBuffer - 发送数据指针
//       BufferSize - 要写入的字节数
// 返回: 实际写入的字节数 (<BufferSize 可能是缓冲区满)
typedef int (*JLINK_RTTERMINAL_Write_t)(int BufferIndex, const char* pBuffer, int BufferSize);
```

### 3.6 RTT 数据结构

```cpp
// ============================================================
// RTT 相关结构体 (与 DLL 二进制布局必须一致)
// ============================================================

// RTT 启动配置
struct JLinkRTTerminalStart {
    uint32_t ConfigBlockAddress;    // RTT Control Block 地址 (0=自动搜索)
    uint32_t Reserved[3];           // 保留, 必须为0
};

// RTT 缓冲区描述符
struct JLinkRTTerminalBufDesc {
    int32_t  BufferIndex;           // 缓冲区索引
    uint32_t Direction;             // 方向: 0=Up(Target->Host), 1=Down(Host->Target)
    char     acName[32];            // 缓冲区名称 (如 "Terminal", "SystemView")
    uint32_t SizeOfBuffer;          // 缓冲区大小 (字节)
    uint32_t Flags;                 // 缓冲区标志
};

// RTT 运行状态
struct JLinkRTTerminalStatus {
    uint32_t NumBytesTransferred;   // 已传输字节数
    uint32_t NumBytesRead;          // 已读取字节数
    int32_t  HostOverflowCount;     // 主机溢出计数 (数据丢失指示)
    int32_t  IsRunning;             // RTT 是否正在运行 (1=运行中)
    int32_t  NumUpBuffers;          // 上行缓冲区数量
    int32_t  NumDownBuffers;        // 下行缓冲区数量
    uint32_t Reserved[2];           // 保留
};

// RTT 命令枚举
enum JLinkRTTCommand {
    RTT_CMD_START    = 0,           // 启动 RTT
    RTT_CMD_STOP     = 1,           // 停止 RTT
    RTT_CMD_GETDESC  = 2,           // 获取缓冲区描述
    RTT_CMD_GETNUMBUF = 3,          // 获取缓冲区数量
    RTT_CMD_GETSTAT  = 4,           // 获取 RTT 状态
};

// RTT 方向枚举
enum JLinkRTTDirection {
    RTT_DIR_UP   = 0,               // 上行: Target -> Host
    RTT_DIR_DOWN = 1,               // 下行: Host -> Target
};

// 调试接口类型枚举
enum JLinkInterface {
    TIF_JTAG = 0,
    TIF_SWD  = 1,
    TIF_FINE = 3,
    TIF_ICSP = 4,
    TIF_SPI  = 5,
    TIF_C2   = 6,
};
```

---

## 四、RTT 通道模型

### 4.1 Up / Down 双向通道

```
+-------------------+          JTAG/SWD          +-------------------+
|    Target MCU     | <=========================> |   J-Link Probe    |
|  (STM32, etc.)    |    Debug Interface          |                   |
+-------------------+                             +---------+---------+
                                                            | USB / IP
                                                            |
                                                  +---------v---------+
                                                  |  Host PC           |
                                                  |  EmbedDebug        |
                                                  |  (JLink_x64.dll)   |
                                                  +-------------------+
```

RTT 通道是双向的, 每个方向独立:

| 方向 | 含义 | 术语 | 用途 |
|------|------|------|------|
| **Up (上行)** | Target -> Host | `SEGGER_RTT_Write()` (MCU端) | MCU 输出日志/调试信息 |
| **Down (下行)** | Host -> Target | `SEGGER_RTT_Read()` (MCU端) | 上位机发送命令到 MCU |

### 4.2 通道索引约定

```
Channel 0 (默认):
  Up[0]:   "Terminal" - 主终端输出 (printf 重定向到这里)
  Down[0]: "Terminal" - 主终端输入

Channel 1..N (可选):
  Up[1]:   "SystemView" - SEGGER SystemView 事件追踪数据
  Up[2]:   "Data" - 自定义二进制数据通道
  Down[1]: "Commands" - 自定义命令通道
```

Channel 0 是 RTT 初始化时自动创建的默认通道, 不需要 MCU 端额外配置。其他通道需要 MCU 端调用 `SEGGER_RTT_ConfigUpBuffer()` / `SEGGER_RTT_ConfigDownBuffer()` 创建。

### 4.3 数据流原理

```
MCU RAM 中:
+------------------------------------------------------------------+
| RTT Control Block (16字节ID + BufferDescriptors[])               |
|   ID: "SEGGER RTT" (自动检测标识)                                 |
|   MaxNumUpBuffers, MaxNumDownBuffers                              |
|   UpDescriptors[0..N]: {sName, pBuffer, SizeOfBuffer, WrOff, RdOff, Flags} |
|   DownDescriptors[0..N]: {sName, pBuffer, SizeOfBuffer, WrOff, RdOff, Flags}|
+------------------------------------------------------------------+
| Up Buffer 0 (环形缓冲区, 通常 1KB)                                |
| Down Buffer 0 (环形缓冲区, 通常 16-32B)                           |
+------------------------------------------------------------------+

数据流 (Up方向):
  MCU: SEGGER_RTT_Write() -> 写入Up环形缓冲区 -> 更新WrOff
  J-Link: 后台读取WrOff vs RdOff -> 有新数据 -> DMA方式读到Host

数据流 (Down方向):
  Host: JLINK_RTTERMINAL_Write() -> J-Link写入Down环形缓冲区 -> 更新WrOff
  MCU: SEGGER_RTT_Read() -> 检查WrOff vs RdOff -> 有新数据 -> 读取
```

关键特性:
- **无锁设计**: Up方向的WrOff只由MCU写, RdOff只由J-Link写, 不存在竞争
- **零拷贝**: J-Link通过调试接口直接访问MCU RAM, 无需MCU参与
- **实时性**: Background模式下不影响MCU实时行为, 传输速度约 3.5 MB/s

---

## 五、JLinkBridge 类设计 (QLibrary 动态加载)

### 5.1 设计模式: 适配器模式 (Adapter)

`JLinkBridge` 是对 J-Link DLL 的 C++ 封装层, 使用 Qt 的 `QLibrary` 进行运行时动态加载, 避免链接期依赖 DLL。

```
+----------------+     uses      +------------------+
| RttConnection  | -----------> |  JLinkBridge     |
| (IConnection)  |              |  (DLL适配器)      |
+----------------+              +------------------+
                                        |
                                  QLibrary::resolve()
                                        |
                                +-------v---------+
                                | JLink_x64.dll   |
                                | (SEGGER SDK)     |
                                +-----------------+
```

### 5.2 类设计

```cpp
// ============================================================
// 文件: src/rtt/JLinkBridge.h
// 层次: 基础设施层 (不依赖任何上层模块)
// 模式: 适配器模式 - 将C函数DLL接口适配为C++对象
// ============================================================
#ifndef JLINKBRIDGE_H
#define JLINKBRIDGE_H

#include <QObject>
#include <QLibrary>
#include <QString>
#include <QVector>

// 前向声明 RTT 结构体 (与 DLL 二进制布局一致)
struct JLinkRTTerminalStart;
struct JLinkRTTerminalBufDesc;
struct JLinkRTTerminalStatus;

// RTT 命令枚举
enum JLinkRTTCommand {
    RTT_CMD_START    = 0,
    RTT_CMD_STOP     = 1,
    RTT_CMD_GETDESC  = 2,
    RTT_CMD_GETNUMBUF = 3,
    RTT_CMD_GETSTAT  = 4,
};

// 调试接口类型
enum JLinkInterface {
    TIF_JTAG = 0,
    TIF_SWD  = 1,
};

// J-Link 全局错误码
enum JLinkError {
    JLINK_OK                    = 0,
    JLINK_ERROR_UNSPECIFIED     = -1,
    JLINK_ERROR_NO_CONNECTION   = -256,
    JLINK_ERROR_COMM_ERROR      = -257,
    JLINK_ERROR_DLL_NOT_OPEN    = -258,
    JLINK_ERROR_VCC_FAILURE     = -259,
    JLINK_ERROR_NO_CPU_FOUND    = -261,
    JLINK_ERROR_RTT_CB_NOT_FOUND = -2,
};

class JLinkBridge : public QObject {
    Q_OBJECT

public:
    explicit JLinkBridge(QObject* parent = nullptr);
    ~JLinkBridge() override;

    JLinkBridge(const JLinkBridge&) = delete;
    JLinkBridge& operator=(const JLinkBridge&) = delete;

    // ---- DLL 加载/卸载 ----

    // 加载 JLink DLL, dllPath 为空则自动搜索
    bool loadLibrary(const QString& dllPath = QString());
    bool isLoaded() const;
    void unloadLibrary();

    // ---- 连接管理 ----
    int open();
    void close();
    int isOpen() const;
    int connect();
    int isConnected() const;
    int execCommand(const char* cmd);

    // ---- 仿真器选择 ----
    int emuSelectByIndex(int index);
    int emuSelectBySN(uint32_t sn);
    int emuSelectIP(const char* ip, int port);

    // ---- 接口 & 速度 ----
    int tifSelect(int tif);           // JLinkInterface 枚举
    void setSpeed(int speedKHz);      // -1=auto, 0=adaptive
    int getSpeed() const;

    // ---- RTT 操作 ----
    int rttStart(uint32_t configBlockAddr = 0);
    int rttStop();
    int rttRead(int bufferIndex, char* buf, int bufSize);
    int rttWrite(int bufferIndex, const char* buf, int bufSize);
    int rttGetStatus(JLinkRTTerminalStatus* status);
    int rttGetNumUpBuffers();
    int rttGetNumDownBuffers();
    int rttGetBufDesc(int bufferIndex, int direction, JLinkRTTerminalBufDesc* desc);

    // ---- 硬件状态 ----
    int getHWStatus(void* pStatus);
    int getDLLVersion() const;
    uint32_t getSN() const;
    int hasError() const;
    void clrError();

    // ---- 错误信息 ----
    QString lastErrorString() const;

signals:
    void errorOccurred(const QString& errorMsg);

private:
    // 解析所有 DLL 函数指针
    bool resolveFunctions();

    // ---- 函数指针 ----
    // 连接管理
    JLINK_Open_t            m_fnOpen = nullptr;
    JLINK_Close_t           m_fnClose = nullptr;
    JLINK_IsOpen_t          m_fnIsOpen = nullptr;
    JLINK_Connect_t         m_fnConnect = nullptr;
    JLINK_IsConnected_t     m_fnIsConnected = nullptr;
    JLINK_ExecCommand_t     m_fnExecCommand = nullptr;
    JLINK_GetDLLVersion_t   m_fnGetDLLVersion = nullptr;

    // 仿真器选择
    JLINK_EMU_SelectByIndex_t m_fnEmuSelectByIndex = nullptr;
    JLINK_EMU_SelectByUSBSN_t m_fnEmuSelectByUSBSN = nullptr;
    JLINK_EMU_SelectIP_t      m_fnEmuSelectIP = nullptr;

    // 接口 & 速度
    JLINK_TIF_Select_t      m_fnTifSelect = nullptr;
    JLINK_SetSpeed_t        m_fnSetSpeed = nullptr;
    JLINK_GetSpeed_t        m_fnGetSpeed = nullptr;

    // RTT
    JLINK_RTTERMINAL_Control_t m_fnRttControl = nullptr;
    JLINK_RTTERMINAL_Read_t    m_fnRttRead = nullptr;
    JLINK_RTTERMINAL_Write_t   m_fnRttWrite = nullptr;

    // 其他
    JLINK_GetHWStatus_t     m_fnGetHWStatus = nullptr;
    JLINK_Reset_t           m_fnReset = nullptr;
    JLINK_HasError_t        m_fnHasError = nullptr;    // typedef int (*JLINK_HasError_t)(void);
    JLINK_ClrError_t        m_fnClrError = nullptr;    // typedef void (*JLINK_ClrError_t)(void);
    JLINK_GetSN_t           m_fnGetSN = nullptr;       // typedef uint32_t (*JLINK_GetSN_t)(void);

    // ---- 成员变量 ----
    QLibrary    m_library;
    QString     m_lastError;
};

#endif // JLINKBRIDGE_H
```

### 5.3 DLL 加载流程

```cpp
bool JLinkBridge::loadLibrary(const QString& dllPath) {
    // 1. 优先使用指定路径
    // 2. 否则尝试默认安装路径
    // 3. 最后尝试系统 PATH 搜索
    QStringList searchPaths = {
        dllPath,
        "E:/Embedded/Tool/SEGGER_IOT/JLink_V932/JLink_x64.dll",
        "C:/Program Files/SEGGER/JLink/JLink_x64.dll",
        "JLink_x64.dll"  // PATH 搜索
    };

    for (const auto& path : searchPaths) {
        if (path.isEmpty()) continue;
        m_library.setFileName(path);
        if (m_library.load()) {
            return resolveFunctions();
        }
    }
    m_lastError = "无法加载 JLink_x64.dll";
    return false;
}

bool JLinkBridge::resolveFunctions() {
    // QLibrary::resolve() 返回 void*, 需要 reinterpret_cast
    #define RESOLVE(name) \
        m_fn##name = reinterpret_cast<name##_t>( \
            m_library.resolve("JLINK_" #name)); \
        if (!m_fn##name) { \
            m_lastError = QString("无法解析函数: JLINK_" #name); \
            return false; \
        }

    RESOLVE(Open);
    RESOLVE(Close);
    RESOLVE(IsOpen);
    RESOLVE(Connect);
    RESOLVE(IsConnected);
    RESOLVE(ExecCommand);
    RESOLVE(GetDLLVersion);
    RESOLVE(TIF_Select);
    RESOLVE(SetSpeed);
    RESOLVE(GetSpeed);
    RESOLVE(RTTERMINAL_Control);
    RESOLVE(RTTERMINAL_Read);
    RESOLVE(RTTERMINAL_Write);

    // 可选函数 (旧版 DLL 可能没有)
    m_fnEmuSelectByIndex = reinterpret_cast<JLINK_EMU_SelectByIndex_t>(
        m_library.resolve("JLINK_EMU_SelectByIndex"));
    m_fnEmuSelectByUSBSN = reinterpret_cast<JLINK_EMU_SelectByUSBSN_t>(
        m_library.resolve("JLINK_EMU_SelectByUSBSN"));
    m_fnEmuSelectIP = reinterpret_cast<JLINK_EMU_SelectIP_t>(
        m_library.resolve("JLINK_EMU_SelectIP"));
    m_fnGetHWStatus = reinterpret_cast<JLINK_GetHWStatus_t>(
        m_library.resolve("JLINK_GetHWStatus"));
    m_fnGetSN = reinterpret_cast<JLINK_GetSN_t>(
        m_library.resolve("JLINK_GetSN"));

    #undef RESOLVE
    return true;
}
```

---

## 六、RttConnection 实现方案 (继承 IConnection)

### 6.1 类设计

```cpp
// ============================================================
// 文件: src/connection/RttConnection.h
// 层次: 基础设施层
// 模式: 策略模式 - IConnection 接口的具体实现
// ============================================================
#ifndef RTTCONNECTION_H
#define RTTCONNECTION_H

#include "connection/IConnection.h"
#include <QThread>
#include <QMutex>
#include <QAtomicBool>

class JLinkBridge;
class RttReadThread;

// RTT 连接实现 - 通过 J-Link 调试器与 MCU 进行 RTT 通信
// 复用 IConnection 抽象接口, 上层无需知道底层是 RTT
class RttConnection : public IConnection {
    Q_OBJECT

public:
    explicit RttConnection(QObject* parent = nullptr);
    ~RttConnection() override;

    // IConnection 接口实现
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;

    // 通过 QVariantMap 配置连接参数
    // 支持的 key:
    //   "dllPath"      (QString) JLink DLL路径, 空=自动搜索
    //   "deviceName"   (QString) 目标设备名, 如 "STM32F407VG"
    //   "interface"    (int)     调试接口: 0=JTAG, 1=SWD (默认SWD)
    //   "speed"        (int)     接口速度 kHz, -1=自动 (默认-1)
    //   "serialNumber" (uint)    J-Link 序列号, 0=自动选择
    //   "emuIndex"     (int)     J-Link 索引 (多J-Link时使用, 默认0)
    //   "rttChannel"   (int)     RTT 通道索引 (默认0)
    //   "configBlockAddr" (uint) RTT CB地址, 0=自动搜索 (默认0)
    void configure(const QVariantMap& params) override;

private slots:
    void onReadThreadData(const QByteArray& data);
    void onReadThreadError(const QString& error);

private:
    void updateState(ConnectionState newState);

    // 配置参数
    QString m_dllPath;
    QString m_deviceName;          // 如 "STM32F407VG"
    int     m_interface = 1;       // 默认 SWD
    int     m_speed = -1;          // 默认自动速度
    uint32_t m_serialNumber = 0;   // J-Link SN (0=自动)
    int     m_emuIndex = 0;        // J-Link 索引
    int     m_rttChannel = 0;      // RTT 通道索引
    uint32_t m_configBlockAddr = 0;// RTT CB 地址

    // 运行时状态
    JLinkBridge*    m_bridge = nullptr;
    RttReadThread*  m_readThread = nullptr;
    ConnectionState m_state = ConnectionState::Disconnected;
    mutable QMutex  m_stateMutex;
};

#endif // RTTCONNECTION_H
```

### 6.2 连接流程 (open 方法)

```cpp
bool RttConnection::open() {
    QMutexLocker locker(&m_stateMutex);
    if (m_state == ConnectionState::Connected) return true;

    updateState(ConnectionState::Connecting);

    // Step 1: 加载 DLL
    m_bridge = new JLinkBridge(this);
    if (!m_bridge->loadLibrary(m_dllPath)) {
        emit errorOccurred(m_bridge->lastErrorString());
        updateState(ConnectionState::Error);
        return false;
    }

    // Step 2: 打开 J-Link
    if (m_bridge->open() < 0) {
        emit errorOccurred("JLINK_Open 失败");
        updateState(ConnectionState::Error);
        return false;
    }

    // Step 3: 选择仿真器
    if (m_serialNumber > 0) {
        m_bridge->emuSelectBySN(m_serialNumber);
    } else {
        m_bridge->emuSelectByIndex(m_emuIndex);
    }

    // Step 4: 配置目标设备
    if (!m_deviceName.isEmpty()) {
        QString cmd = QString("device = %1").arg(m_deviceName);
        m_bridge->execCommand(cmd.toUtf8().constData());
    }

    // Step 5: 选择调试接口
    if (m_bridge->tifSelect(m_interface) < 0) {
        emit errorOccurred("TIF_Select 失败, 可能接口不支持");
        updateState(ConnectionState::Error);
        return false;
    }

    // Step 6: 设置速度
    m_bridge->setSpeed(m_speed);

    // Step 7: 连接目标 MCU
    if (m_bridge->connect() < 0) {
        emit errorOccurred("无法连接目标 MCU, 请检查: "
                          "1. J-Link已连接 2. 目标已供电 3. 设备名正确");
        updateState(ConnectionState::Error);
        return false;
    }

    // Step 8: 启动 RTT
    if (m_bridge->rttStart(m_configBlockAddr) < 0) {
        emit errorOccurred("RTT启动失败, 请确认目标程序已包含 RTT 初始化");
        updateState(ConnectionState::Error);
        return false;
    }

    // Step 9: 启动读取线程
    m_readThread = new RttReadThread(m_bridge, m_rttChannel, this);
    connect(m_readThread, &RttReadThread::dataReceived,
            this, &RttConnection::onReadThreadData);
    connect(m_readThread, &RttReadThread::errorOccurred,
            this, &RttConnection::onReadThreadError);
    m_readThread->start();

    updateState(ConnectionState::Connected);
    return true;
}
```

---

## 七、RTT 数据读取的线程模型

### 7.1 线程架构

```
主线程 (UI Thread)
  |
  +-- RttConnection (IConnection)
        |
        +-- JLinkBridge (DLL封装, 非线程安全)
        |     |
        |     +-- JLINK_RTTERMINAL_Read()  <-- 从读取线程调用
        |     +-- JLINK_RTTERMINAL_Write() <-- 从主线程调用
        |
        +-- RttReadThread (QThread)
              |
              +-- 轮询循环 (1ms interval)
              |     |
              |     +-- JLINK_RTTERMINAL_Read(UpChannel, buf, 4096)
              |     |
              |     +-- 有数据? -> emit dataReceived(QByteArray)
              |     |
              |     +-- 无数据? -> QThread::usleep(1000)
              |
              +-- 健康检查 (每100次轮询)
                    |
                    +-- rttGetStatus() -> 检查 IsRunning
```

### 7.2 读取线程实现

```cpp
// ============================================================
// 文件: src/rtt/RttReadThread.h
// 内部类, 不暴露给上层
// ============================================================
class RttReadThread : public QThread {
    Q_OBJECT

public:
    RttReadThread(JLinkBridge* bridge, int channel, QObject* parent = nullptr)
        : QThread(parent)
        , m_bridge(bridge)
        , m_channel(channel)
        , m_running(false)
    {}

    void stop() {
        m_running = false;
    }

signals:
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& error);

protected:
    void run() override {
        m_running = true;
        char buf[4096];  // 读取缓冲区
        int pollCount = 0;

        while (m_running) {
            // 从 RTT Up 通道读取数据
            int bytesRead = m_bridge->rttRead(m_channel, buf, sizeof(buf));

            if (bytesRead > 0) {
                // 有数据, 发送到主线程
                emit dataReceived(QByteArray(buf, bytesRead));
            }

            // 健康检查: 每100次轮询检查一次 RTT 状态
            pollCount++;
            if (pollCount >= 100) {
                pollCount = 0;
                JLinkRTTerminalStatus status;
                if (m_bridge->rttGetStatus(&status) == 0) {
                    if (!status.IsRunning) {
                        emit errorOccurred("RTT 会话已断开");
                        m_running = false;
                        break;
                    }
                    // 溢出告警 (非致命)
                    if (status.HostOverflowCount > 0) {
                        // 可以发出告警信号, 但不中断
                    }
                }
            }

            // 控制轮询频率: 有数据时不休眠, 无数据时休眠1ms
            if (bytesRead <= 0) {
                usleep(1000);  // 1ms
            }
            // 有数据时立即继续读取, 直到缓冲区排空
        }
    }

private:
    JLinkBridge*  m_bridge;
    int           m_channel;
    QAtomicBool   m_running;
};
```

### 7.3 线程安全考虑

| 问题 | 解决方案 |
|------|---------|
| Read/Write 并发 | J-Link DLL 内部已处理线程安全, Read 和 Write 可以在不同线程调用 |
| 状态查询并发 | RTT Status 查询是只读操作, 不会与 Read/Write 冲突 |
| 连接/断开并发 | open()/close() 仅在主线程调用, 通过 QMutex 保护状态 |
| 数据传递 | 通过 Qt 信号/槽 (自动队列连接), 线程间安全传递 QByteArray |

> **重要**: J-Link DLL 是 **单实例、单目标** 设计 (每个进程只能连接一个 J-Link 和一个目标 MCU)。不支持多个 JLinkBridge 实例同时操作不同的 J-Link。

---

## 八、错误处理和连接状态管理

### 8.1 状态转换图

```
                    open()
  Disconnected -----------> Connecting
      ^                        |
      |                        | (成功)
      |                        v
      | close()           Connected <------+
      |                        |            |
      |                        | (错误)     | (恢复)
      |                        v            |
      +------------------- Error -----------+
                                |
                                | close()
                                v
                           Disconnected
```

### 8.2 错误码映射

| J-Link 错误码 | 值 | 含义 | 用户提示 |
|--------------|-----|------|---------|
| JLINK_OK | 0 | 成功 | - |
| JLINK_ERROR_RTT_CB_NOT_FOUND | -2 | RTT控制块未找到 | "请确认目标程序已初始化RTT (调用SEGGER_RTT_Init())" |
| JLINK_ERROR_NO_CONNECTION | -256 | 无J-Link连接 | "请连接J-Link调试器" |
| JLINK_ERROR_COMM_ERROR | -257 | 通信错误 | "J-Link通信异常, 请检查USB连接" |
| JLINK_ERROR_VCC_FAILURE | -259 | 目标未供电 | "目标MCU未检测到电压, 请检查供电" |
| JLINK_ERROR_NO_CPU_FOUND | -261 | 未找到CPU | "未检测到目标CPU, 请检查: 1.设备名正确 2.接线正确 3.接口选择正确" |

### 8.3 自动重连策略

```cpp
// 在读取线程中检测到断开时的处理
void RttConnection::onReadThreadError(const QString& error) {
    qWarning() << "RTT read error:" << error;
    updateState(ConnectionState::Error);
    emit errorOccurred(error);
    // 不自动重连, 交给用户手动操作
    // 理由: RTT断开可能是目标MCU复位/断电, 需要用户确认后重连
}
```

---

## 九、设备选择和配置参数

### 9.1 配置面板参数

| 参数 | Key | 类型 | 默认值 | 说明 |
|------|-----|------|--------|------|
| DLL路径 | dllPath | QString | 自动搜索 | JLink_x64.dll 完整路径 |
| 设备名 | deviceName | QString | "" | SEGGER设备名 (如 STM32F407VG) |
| 调试接口 | interface | int | 1 (SWD) | 0=JTAG, 1=SWD |
| 接口速度 | speed | int | -1 | kHz, -1=自动, 0=自适应 |
| 序列号 | serialNumber | uint | 0 | J-Link SN, 0=第一个 |
| 仿真器索引 | emuIndex | int | 0 | 多J-Link时选择 |
| RTT通道 | rttChannel | int | 0 | 0=默认Terminal |
| CB地址 | configBlockAddr | uint | 0 | RTT Control Block地址, 0=自动 |

### 9.2 常用 STM32 设备名参考

| MCU | 设备名 |
|-----|--------|
| STM32F103C8 | STM32F103C8 |
| STM32F103CB | STM32F103CB |
| STM32F407VG | STM32F407VG |
| STM32F407ZE | STM32F407ZE |
| STM32H743VI | STM32H743VI |
| STM32G431KB | STM32G431KB |
| STM32F401CC | STM32F401CC |

完整设备列表可通过 `JLINK_ExecCommand("Devices")` 获取, 或查看 J-Link 安装目录下的设备数据库。

### 9.3 ConnectionFactory 集成

当前 `ConnectionFactory::create()` 中 `ConnectionType::Rtt` 分支返回 `nullptr`, 集成后:

```cpp
// ConnectionFactory.cpp 中添加:
#include "connection/RttConnection.h"

case ConnectionType::Rtt:
    return new RttConnection(parent);
```

---

## 十、依赖和构建配置

### 10.1 CMake 配置

```cmake
# RTT 模块不需要链接 JLink DLL (运行时动态加载)
# 只需添加源文件:
set(RTT_SOURCES
    src/rtt/JLinkBridge.h
    src/rtt/JLinkBridge.cpp
    src/rtt/RttReadThread.h
    src/connection/RttConnection.h
    src/connection/RttConnection.cpp
)
```

### 10.2 运行时 DLL 部署

方案一: 在 EmbedDebug.bat 中添加 DLL 路径到 PATH:
```bat
set PATH=E:\Embedded\Tool\SEGGER_IOT\JLink_V932;%PATH%
```

方案二: 用户在 RTT 配置面板中手动指定 DLL 路径。

方案三: 将 `JLink_x64.dll` 复制到 build 输出目录 (推荐, windeployqt 后):

### 10.3 项目目录结构

```
src/
├── rtt/                         # 基础设施层: RTT
│   ├── JLinkBridge.h            # DLL适配器 (适配器模式)
│   ├── JLinkBridge.cpp
│   └── RttReadThread.h          # 内部读取线程
├── connection/
│   ├── IConnection.h            # 已有 - 连接抽象接口
│   ├── SerialConnection.h/cpp   # 已有
│   ├── TcpConnection.h/cpp      # 已有
│   ├── UdpConnection.h/cpp      # 已有
│   └── RttConnection.h/cpp      # 新增 - RTT连接实现
```

---

## 十一、风险和限制

| 风险项 | 影响 | 缓解措施 |
|--------|------|---------|
| J-Link SDK 无公开头文件 | 需要手动维护函数签名, 版本更新可能不兼容 | 限制SDK版本, 充分测试 |
| DLL 单实例限制 | 同一进程只能连接一个J-Link | 文档说明限制, 不支持多实例 |
| RTT CB 搜索超时 | 大RAM目标 (>1MB) 搜索可能耗时数秒 | 支持手动指定CB地址 |
| 读取线程 CPU 占用 | 1ms 轮询可能占用约1-2% CPU | 空闲时自适应降频, 有数据时立即响应 |
| 目标 MCU 复位 | MCU复位后RTT需要重新初始化 | 检测状态并提示用户 |
| 许可证 | J-Link SDK 部分功能需要许可 | RTT是免费功能, 不需要额外许可 |

---

## 十二、参考资源

- [SEGGER RTT 知识库](https://kb.segger.com/RTT) - RTT 技术原理和API文档
- [PyLink 源码 (square/pylink)](https://github.com/square/pylink) - Python J-Link DLL 封装, 最完整的公开API参考
- [J-Link SDK 官方页面](https://www.segger.com/products/debug-probes/j-link/tools/j-link-sdk/) - SDK 下载 (需注册)
- [J-Link User Guide (UM08001)](https://www.segger.com/downloads/jlink/UM08001_JLink.pdf) - 官方用户手册
- [JLinkARM.def 导出定义](https://github.com/fantomgs/xvcd-jlink/blob/master/JLinkARM.def) - DLL 导出函数列表
- [JLink SDK API C# 版本](https://www.cnblogs.com/209jkjkjk/p/18619835) - C# DllImport 声明参考

---

## 附录 A: J-Link DLL __stdcall 函数列表

以下函数在 Win32 平台使用 `__stdcall` 调用约定 (x64 平台无区别):

```
JLINK_Configure
JLINK_DownloadFile
JLINK_GetAvailableLicense
JLINK_GetPCode
JLINK_PrintConfig
JLINK_EraseChip
JLINK_SPI_Transfer
JLINK_GetpFunc
JLINK_GetMemZones
JLINK_ReadMemZonedEx
JLINK_WriteMemZonedEx
JLINK_SetHookUnsecureDialog
JLINK_DIALOG_Configure
JLINK_DIALOG_ConfigureEx
JLINK_EMU_GPIO_GetProps
JLINK_EMU_GPIO_GetState
JLINK_EMU_GPIO_SetState
JLINK_EMU_AddLicense
JLINK_EMU_EraseLicenses
JLINK_EMU_GetLicenses
JLINK_HSS_GetCaps
JLINK_HSS_Start
JLINK_HSS_Stop
JLINK_HSS_Read
JLINK_POWERTRACE_Control
JLINK_POWERTRACE_Read
JLINK_RTTERMINAL_Control
JLINK_RTTERMINAL_Read
JLINK_RTTERMINAL_Write
JLINK_STRACE_Config
JLINK_STRACE_Control
JLINK_STRACE_Read
JLINK_STRACE_Start
JLINK_STRACE_Stop
JLINK_SWD_GetData
JLINK_SWD_GetU8
JLINK_SWD_GetU16
JLINK_SWD_GetU32
JLINK_SWD_StoreGetRaw
JLINK_SWD_StoreRaw
JLINK_SWD_SyncBits
JLINK_SWD_SyncBytes
JLINK_SetFlashProgProgressCallback
JLINKARM_BeginDownload
JLINKARM_EndDownload
JLINKARM_WriteMem
```
