# PRD-008: YMODEM协议传输实现

## 背景
YMODEM是XMODEM的增强版本，在XMODEM-CRC基础上增加了Block 0(文件名/大小/时间戳)和批量文件传输能力。大多数STM32 bootloader支持XMODEM，但部分高级bootloader和Linux目标支持YMODEM。实现YMODEM补全OTA协议栈。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | YModemTransfer实现YMODEM协议(Sender: PC→MCU) | P0 | ota/protocols/ |
| R2 | 支持Block 0文件信息传输(文件名+大小+修改时间) | P0 | ota/protocols/ |
| R3 | 支持批量文件传输(多文件连续发送) | P1 | ota/protocols/ |
| R4 | OtaManager集成YMODEM协议选项 | P0 | ota/ |
| R5 | OtaWidget协议下拉框增加YMODEM选项 | P0 | ota/ |

## 接口设计

```cpp
class YModemTransfer : public QObject {
    Q_OBJECT
public:
    void setConnection(IConnection* conn);
    void setFilePath(const QString& path);
    void setFilePaths(const QStringList& paths);  // 批量传输
    bool start();
    void cancel();
signals:
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();
    void transferError(const QString& reason);
};
```

## 设计模式
- **策略模式**: YMODEM与XMODEM同为OTA协议策略，可互换
- **观察者**: 进度/完成/错误信号通知UI

## 依赖
- `IConnection` — 数据通道
- `CRC` — CRC16-CCITT校验
- `XModemTransfer` — 复用块构建逻辑

## 与XMODEM的区别
1. YMODEM使用CRC模式启动(发送'C')
2. Block 0包含文件元信息: 文件名(ASCII) + 文件大小(ASCII) + 修改时间(Octal) + 填充0x00
3. 支持批量传输: 多个文件依次发送，每个文件一个Block 0
4. 结束标记: 批量传输最后发送空的Block 0(128字节全0)

## 验收标准
1. Block 0正确携带文件名和大小信息
2. 数据块使用128字节 + CRC16
3. 支持取消传输
4. 进度信号实时更新
5. OtaManager和OtaWidget正确集成
