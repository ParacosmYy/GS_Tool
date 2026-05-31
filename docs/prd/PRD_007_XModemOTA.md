# PRD-007: XMODEM协议传输实现

## 背景
OTA固件升级需要通过串口/TCP传输固件文件到MCU bootloader。XMODEM是最常用的STM32 bootloader内置协议。实现XMODEM(Checksum/CRC/1K三种模式)为OTA基础。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | XModemTransfer实现XMODEM协议(Sender: PC→MCU) | P0 | ota/protocols/ |
| R2 | 支持Checksum/CRC16/1K三种模式 | P0 | ota/protocols/ |
| R3 | OtaManager OTA调度管理器 | P0 | ota/ |
| R4 | OtaWidget OTA操作面板(文件选择/进度/速率/ETA) | P1 | ota/ |
| R5 | 导航树增加OTA节点 | P1 | core/ |

## 接口设计

```cpp
class XModemTransfer : public QObject {
    Q_OBJECT
public:
    enum Mode { Checksum, CRC, OneK };
    void setConnection(IConnection* conn);
    void setMode(Mode mode);
    void setFilePath(const QString& path);
    bool start();
    void cancel();
signals:
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();
    void transferError(const QString& reason);
};
```

## 设计模式
- **策略模式**: Checksum/CRC/1K三种校验策略
- **模板方法**: BaseTransfer定义传输骨架(start→sendBlocks→finish)
- **观察者**: 进度/完成/错误信号通知UI

## 依赖
- `IConnection` — 数据通道(串口/TCP)
- `CRC` — CRC8/XMODEM CRC16校验
- `IntelHexParser` — HEX→BIN转换
- `RingBuffer` — 接收缓冲

## 验收标准
1. XMODEM-Checksum模式: SOH + 128字节块 + Sum8校验
2. XMODEM-CRC模式: SOH + 128字节块 + CRC16校验
3. XMODEM-1K模式: STX + 1024字节块 + CRC16校验
4. 支持取消传输
5. 进度信号实时更新百分比和速率
