# PRD-009: ZMODEM协议传输实现

## 背景
ZMODEM是XMODEM/YMODEM的后继协议，支持CRC32校验、1024字节数据帧、连续发送(无需每块等待ACK)、错误自动重传、断点续传。常用于Linux sz/rz工具和高级嵌入式bootloader。实现ZMODEM完善OTA协议栈。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | ZModemTransfer实现ZMODEM协议(Sender: PC→MCU) | P0 | ota/protocols/ |
| R2 | 支持ZDATA数据帧(1024字节块+CRC32) | P0 | ota/protocols/ |
| R3 | 支持ZFILE文件信息和ZFIN结束握手 | P0 | ota/protocols/ |
| R4 | OtaManager/OtaWidget集成ZMODEM选项 | P0 | ota/ |

## 接口设计

```cpp
class ZModemTransfer : public QObject {
    Q_OBJECT
public:
    void setConnection(IConnection* conn);
    void setFilePath(const QString& path);
    bool start();
    void cancel();
};
```

## 设计模式
- **策略模式**: ZMODEM与XMODEM/YMODEM同为OTA协议策略

## 依赖
- `IConnection` — 数据通道
- `CRC` — CRC32校验

## 验收标准
1. ZRQINIT握手正确发起
2. ZFILE帧携带文件名和大小
3. ZDATA帧使用1024字节块+CRC32
4. ZFIN结束握手正确完成
5. 支持取消传输
