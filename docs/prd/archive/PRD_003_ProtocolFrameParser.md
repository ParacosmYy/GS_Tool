# PRD-003: 协议帧格式定义+状态机解析器+解析结果展示

## 背景
嵌入式调试工具需要解析自定义串口协议帧。典型场景：MCU按照 "帧头+长度+数据+校验" 格式发送数据，工具需要实时解析并在表格中展示各字段含义和值。目前缺少协议解析能力，用户只能看原始HEX数据。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | FrameDefinition帧格式数据模型（帧头/尾/长度/校验/字段定义） | P0 | protocol/ |
| R2 | FrameParser状态机解析器（Idle→Header→Length→Payload→Checksum→Done/Error） | P0 | protocol/ |
| R3 | ProtocolView解析结果展示（QTableView字段名=值表格，错误标红） | P0 | protocol/ |
| R4 | CRC校验复用utils/CRC.h公共组件 | P0 | protocol/ |
| R5 | 导航树增加Protocol节点，点击切换到ProtocolView | P1 | core/ |

## 接口设计

### FrameDefinition
```cpp
// 单个字段定义
struct FieldDef {
    QString name;       // 字段名称
    int offset;         // 在帧内的字节偏移
    int size;           // 字段字节数
    enum Type { UInt8, UInt16LE, UInt16BE, UInt32LE, UInt32BE, Int8, Int16LE, Int16BE, Float, Raw };
    Type type;
    double scale;       // 缩放系数（显示值 = 原始值 × scale + offset）
    double offset_val;
    QString unit;       // 单位（如 "mV", "°C"）
};

// 完整帧格式定义
struct FrameDefinition {
    QByteArray header;          // 帧头（如 AA 55）
    QByteArray footer;          // 帧尾（可选，如 0D 0A）
    int lengthFieldOffset;      // 长度字段在帧内的偏移（-1表示无长度字段）
    int lengthFieldSize;        // 长度字段字节数（1或2）
    int lengthAdjust;           // 长度字段值需要加的偏移（长度字段值=有效数据长度+lengthAdjust）
    int checksumOffset;         // 校验字段在帧内的偏移（-1表示无校验）
    enum ChecksumType { None, Sum8, CRC8, CRC16CCITT, CRC16Modbus, CRC32 };
    ChecksumType checksumType;
    int checksumSize;           // 校验字段字节数
    int checksumStart;          // 校验计算起始偏移（通常为0）
    int checksumEnd;            // 校验计算结束偏移（-1表示到校验字段前）
    QVector<FieldDef> fields;   // 数据字段列表
};
```

### FrameParser
```cpp
class FrameParser : public QObject {
    Q_OBJECT
public:
    explicit FrameParser(QObject* parent = nullptr);
    void setDefinition(const FrameDefinition& def);
    void feed(const QByteArray& data);      // 喂入字节流
    void reset();
signals:
    void frameParsed(const QVariantMap& fields);  // 解析成功
    void frameError(const QString& reason);        // 校验失败/格式错误
};
```

### ProtocolView
```cpp
class ProtocolView : public QWidget {
    Q_OBJECT
public:
    explicit ProtocolView(QWidget* parent = nullptr);
    void addFrame(const QVariantMap& fields);     // 添加解析结果
    void clear();
public slots:
    void onFrameParsed(const QVariantMap& fields);
    void onFrameError(const QString& reason);
};
```

## 设计模式
- **状态模式 (State)**: FrameParser内部状态机，Idle/Header/Receiving/Checksum枚举驱动行为
- **观察者模式 (Observer)**: FrameParser发出frameParsed/frameError信号，ProtocolView响应
- **策略模式 (Strategy)**: CRC校验类型通过CRC.h公共组件的策略选择处理

## 依赖的公共组件
- `CRC` (utils/CRC.h) — CRC8/CRC16-CCITT/CRC16-Modbus/CRC32校验计算
- `HexConverter` (utils/HexConverter.h) — HEX显示
- `IConnection` — 数据源（通过信号/槽获取字节流）
- `TerminalModel` — 可选的原始数据源

## 影响范围
| 文件 | 操作 |
|------|------|
| `src/protocol/FrameDefinition.h` | 新增 |
| `src/protocol/FrameParser.h/cpp` | 新增 |
| `src/protocol/ProtocolView.h/cpp` | 新增 |
| `src/core/MainWindow.h/cpp` | 修改（集成ProtocolView到导航树） |
| `CMakeLists.txt` | 修改（新增源文件） |

## 验收标准
1. 定义帧格式 AA 55 LEN DATA CRC，喂入数据后自动解析出各字段
2. 校验错误时发出frameError信号，ProtocolView标红显示
3. ProtocolView以表格形式展示：序号、时间、各字段名=值
4. 长度字段支持1字节和2字节（大小端）
5. 复用CRC.h公共组件，不重复实现校验算法
