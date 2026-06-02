# PRD-004: 帧可视化编辑器 + 实时波形图 + Intel HEX解析器

## 背景
PRD-003实现了帧解析器和结果表格，但帧格式定义只能通过代码API设置。需要一个可视化编辑器让用户在UI上配置帧格式。同时，解析出的数值字段应当能实时绘制波形图。Intel HEX解析器为后续OTA升级做准备。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | FrameVisualEditor可视化帧格式编辑器（添加字段/属性编辑/预览） | P0 | protocol/ |
| R2 | ChartWidget Qt Charts封装（实时波形，多通道，滑动窗口） | P0 | chart/ |
| R3 | ChannelConfig通道配置（从协议字段自动映射） | P1 | chart/ |
| R4 | IntelHexParser Intel HEX文件解析器（为OTA准备） | P1 | protocol/ |

## 接口设计

### FrameVisualEditor
```cpp
class FrameVisualEditor : public QWidget {
    Q_OBJECT
public:
    explicit FrameVisualEditor(QWidget* parent = nullptr);
    FrameDefinition currentDefinition() const;
    void setDefinition(const FrameDefinition& def);
signals:
    void definitionChanged(const FrameDefinition& def);
};
```

### ChartWidget
```cpp
class ChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChartWidget(QWidget* parent = nullptr);
    void addChannel(const QString& name, const QColor& color);
    void removeChannel(const QString& name);
    void appendData(const QString& channel, double value);
    void setWindowSize(int points);
    void clear();
};
```

### IntelHexParser
```cpp
namespace IntelHex {
struct Record { quint8 byteCount; quint16 address; quint8 type; QByteArray data; quint8 checksum; };
bool parse(const QString& filePath, QByteArray& outBinary, quint16& startAddress);
bool parseRecords(const QString& filePath, QVector<Record>& outRecords);
}
```

## 设计模式
- **观察者模式**: FrameVisualEditor发出definitionChanged信号，FrameParser响应更新
- **策略模式**: ChartWidget支持多种数据源映射

## 依赖的公共组件
- `FrameDefinition` — 帧格式数据模型
- `CRC` — Intel HEX校验验证
- `HexConverter` — HEX显示

## 影响范围
| 文件 | 操作 |
|------|------|
| `src/protocol/FrameVisualEditor.h/cpp` | 新增 |
| `src/chart/ChartWidget.h/cpp` | 新增 |
| `src/protocol/IntelHexParser.h/cpp` | 新增 |
| `src/core/MainWindow.h/cpp` | 修改（集成编辑器、波形图到导航树） |
| `CMakeLists.txt` | 修改 |

## 验收标准
1. 可视化编辑器可配置帧头(HEX)、长度字段位置、校验类型
2. 添加/删除/编辑字段（名称、类型、偏移、缩放）
3. 编辑器变更实时更新到FrameParser
4. ChartWidget实时滚动波形，支持3个以上通道
5. Intel HEX文件解析出二进制数据并验证校验
