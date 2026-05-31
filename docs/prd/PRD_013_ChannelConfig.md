# PRD-013: ChannelConfig通道配置 + ChartModel图表数据模型

## 背景

PRD-004中实现了ChartWidget作为Qt Charts的封装，支持多通道实时波形显示和滑动窗口。当前存在以下问题:

**问题1: 缺少通道配置机制**
ChartWidget的`onFrameParsed`方法将所有数值字段自动创建通道，用户无法控制哪些字段需要绘图、哪些需要忽略。实际场景中，一帧可能包含"温度"、"电压"、"状态码"等字段，用户只需要看"温度"和"电压"，不想看"状态码"。当前所有数值字段都会被绘图，导致噪声通道干扰。

**问题2: 缺少公式映射能力**
FieldDef提供了`scale`和`offsetVal`做线性变换，但用户有时需要在图表中显示组合值，例如"电压 x 电流 = 功率"、或"温度F = 温度C x 1.8 + 32"。当前架构没有公式计算的接入点。

**问题3: ChartWidget职责过重**
ChartWidget同时承担了"数据接收与通道管理"、"滑动窗口维护"、"Y轴自动范围计算"、"图表渲染"四项职责。随着需求增长（降采样、数据导出、多图表联动），ChartWidget会持续膨胀。

**问题4: 无采样率控制**
当帧率很高（例如1000Hz传感器数据），直接逐点渲染会导致Qt Charts性能下降。需要采样率控制（如每10帧取1帧显示）。

本PRD通过引入ChannelConfig（通道配置模型）和ChartModel（图表数据模型）来解决上述问题:
- **ChannelConfig**: 定义数据源映射规则，从协议字段中筛选、重命名、应用公式
- **ChartModel**: 从ChartWidget中剥离数据管理职责，独立管理多通道数据缓冲区、滑动窗口、降采样

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | ChannelConfig: 通道配置数据结构，支持协议字段名映射到通道名 | P0 | chart/ |
| R2 | ChannelConfig: 支持线性公式 `y = a * x + b`，其中x可以是单个字段或两个字段的组合 | P0 | chart/ |
| R3 | ChannelConfig: 支持启用/禁用通道，禁用的通道不接收数据 | P0 | chart/ |
| R4 | ChannelConfig: 支持通道颜色配置，提供默认颜色表 | P1 | chart/ |
| R5 | ChannelConfig: JSON序列化/反序列化，通过SettingsManager持久化 | P1 | chart/ |
| R6 | ChartModel: 独立管理多通道数据缓冲区和滑动窗口 | P0 | chart/ |
| R7 | ChartModel: 从FrameParser的frameParsed信号接收数据，根据ChannelConfig分发到各通道 | P0 | chart/ |
| R8 | ChartModel: 支持降采样策略（最大值/最小值/平均值/首值），在窗口内数据量过大时自动触发 | P0 | chart/ |
| R9 | ChartModel: 当新数据到来时发出信号通知ChartWidget更新渲染 | P0 | chart/ |
| R10 | ChartModel: 提供通道数据导出接口（CSV格式），复用DataExporter | P2 | chart/ |
| R11 | 重构ChartWidget: 移除内部数据管理和onFrameParsed，改为从ChartModel获取渲染数据 | P0 | chart/ |
| R12 | 通道配置面板UI: 在ChartWidget工具栏添加"Config"按钮，打开通道配置对话框 | P1 | chart/ |
| R13 | ChannelConfig: 支持自动发现 -- 当FrameDefinition变更时，自动列出所有可用字段供用户选择映射 | P1 | chart/ |

## 接口设计

### ChannelConfig -- 通道配置

```cpp
#ifndef CHANNELCONFIG_H
#define CHANNELCONFIG_H

#include <QString>
#include <QColor>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

// 单个通道的配置 -- 描述如何从协议帧字段映射到一条曲线
struct ChannelConfig {
    // ---- 数据源映射 ----

    // 数据源类型: 单字段直接映射 或 双字段组合公式
    enum class SourceMode {
        Direct,         // y = scale * field_value + offset
        Combine         // y = scale * (fieldA op fieldB) + offset
    };
    SourceMode sourceMode = SourceMode::Direct;

    // 单字段模式的源字段名（对应FrameDefinition中FieldDef.name）
    QString sourceField;

    // 双字段组合模式的两个源字段名
    QString sourceFieldA;
    QString sourceFieldB;

    // 组合运算符（仅Combine模式）
    enum class CombineOp {
        Add,        // A + B
        Subtract,   // A - B
        Multiply,   // A * B
        Divide      // A / B
    };
    CombineOp combineOp = CombineOp::Add;

    // 线性变换系数: displayValue = scale * rawValue + offset
    double scale = 1.0;
    double offset = 0.0;

    // ---- 通道显示属性 ----

    // 通道名称（显示在图例中，可与sourceField不同）
    QString displayName;

    // 通道颜色（无效颜色表示使用自动分配）
    QColor color;

    // 是否启用此通道（禁用时不接收数据、不显示曲线）
    bool enabled = true;

    // 单位（显示在Y轴标题或图例中）
    QString unit;

    // ---- 采样控制 ----

    // 降采样: 每隔 sampleDivisor 帧取一个数据点 (1=不降采样)
    int sampleDivisor = 1;

    // ---- 计算接口 ----

    // 从帧解析结果中计算本通道的数值
    // fields: FrameParser发出的QVariantMap
    // 返回: 计算后的double值; 如果字段不存在或被禁用，返回NaN
    double compute(const QVariantMap& fields) const;

    // 判断帧数据中是否包含本通道所需的所有字段
    bool canCompute(const QVariantMap& fields) const;

    // ---- JSON序列化 ----

    QJsonObject toJson() const;
    static ChannelConfig fromJson(const QJsonObject& obj);
};

// 通道配置集合 -- 管理多个ChannelConfig，负责整体序列化
class ChannelConfigSet {
public:
    // 添加通道配置
    void addChannel(const ChannelConfig& config);

    // 移除通道配置（按displayName）
    void removeChannel(const QString& displayName);

    // 获取所有通道配置
    const QVector<ChannelConfig>& channels() const;

    // 按displayName查找通道配置（返回nullptr表示未找到）
    ChannelConfig* findChannel(const QString& displayName);
    const ChannelConfig* findChannel(const QString& displayName) const;

    // 从帧解析结果中计算所有启用通道的值
    // 返回: QMap<displayName, value>，仅包含能成功计算的通道
    QMap<QString, double> computeAll(const QVariantMap& fields) const;

    // ---- JSON序列化 ----
    QJsonObject toJson() const;
    static ChannelConfigSet fromJson(const QJsonObject& obj);

    // 根据当前FrameDefinition的字段列表，生成默认通道配置
    // 每个数值字段生成一个Direct模式的ChannelConfig
    static ChannelConfigSet generateDefaults(const QVector<struct FieldDef>& fields);

private:
    QVector<ChannelConfig> m_channels;
};

#endif // CHANNELCONFIG_H
```

### ChartModel -- 图表数据模型

```cpp
#ifndef CHARTMODEL_H
#define CHARTMODEL_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPointF>
#include <QVariantMap>
#include <QTimer>
#include "chart/ChannelConfig.h"

// 图表数据模型 -- 管理多通道数据缓冲区、滑动窗口、降采样
// 职责: 接收帧数据 -> 按ChannelConfig分发 -> 维护滑动窗口 -> 通知视图更新
// 不负责: 图表渲染（由ChartWidget负责）
class ChartModel : public QObject {
    Q_OBJECT

public:
    explicit ChartModel(QObject* parent = nullptr);

    // ---- 配置 ----

    // 设置通道配置集合
    void setChannelConfigSet(const ChannelConfigSet& configSet);

    // 获取当前通道配置集合
    const ChannelConfigSet& channelConfigSet() const;

    // 设置滑动窗口大小（显示的最大数据点数）
    void setWindowSize(int points);

    // 获取滑动窗口大小
    int windowSize() const;

    // 设置刷新间隔（毫秒），数据到来后按此间隔合并刷新，减少渲染频率
    // 0表示每次数据到来都立即刷新
    void setRefreshInterval(int ms);

    // ---- 数据查询 ----

    // 获取指定通道的当前可见数据点（滑动窗口内的点）
    QVector<QPointF> channelData(const QString& displayName) const;

    // 获取所有启用通道的数据（用于批量渲染）
    // 返回: QMap<displayName, QVector<QPointF>>
    QMap<QString, QVector<QPointF>> allChannelData() const;

    // 获取指定通道的Y值范围
    // 返回: pair<min, max>，无数据时返回 <0, 0>
    QPair<double, double> channelYRange(const QString& displayName) const;

    // 获取所有通道的全局Y值范围（用于自动Y轴）
    QPair<double, double> globalYRange() const;

    // 获取当前通道名称列表
    QStringList channelNames() const;

    // 获取当前X轴范围（样本计数范围）
    QPair<double, double> xRange() const;

    // 获取统计信息
    int totalPointsReceived() const;    // 总共接收的数据点数
    int currentFrameIndex() const;      // 当前帧索引（X轴计数器）

    // ---- 操作 ----

    // 清除所有通道数据
    void clear();

signals:
    // 通道数据更新通知（视图据此刷新渲染）
    // updatedChannels: 本次数据更新的通道名列表
    void dataUpdated(const QStringList& updatedChannels);

    // 通道配置变更通知（增删通道时发出）
    void channelsChanged();

    // 全部数据已清除
    void dataCleared();

public slots:
    // 接收帧解析结果，按ChannelConfig分发到各通道
    // 连接: FrameParser::frameParsed -> ChartModel::onFrameParsed
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

private slots:
    // 定时刷新（用于合并高频数据更新，减少信号发射频率）
    void onRefreshTick();

private:
    // 单个通道的内部数据缓冲区
    struct ChannelBuffer {
        QVector<QPointF> points;        // 滑动窗口内的数据点
        int sampleCounter = 0;          // 降采样计数器
    };

    // 从配置中重建通道缓冲区
    void rebuildBuffers();

    // 向指定通道追加一个数据点
    void appendPoint(const QString& displayName, double value);

    ChannelConfigSet m_configSet;
    QMap<QString, ChannelBuffer> m_buffers;     // displayName -> buffer
    int m_windowSize = 200;
    int m_frameIndex = 0;                       // 全局帧计数器（X轴）
    int m_totalPoints = 0;

    // 刷新合并
    QTimer* m_refreshTimer;
    int m_refreshInterval = 0;                  // 0=立即刷新
    QStringList m_pendingUpdates;               // 待刷新的通道名
};

#endif // CHARTMODEL_H
```

### ChartWidget 重构后的接口

ChartWidget在重构后不再直接接收frameParsed信号，改为从ChartModel获取渲染数据:

```cpp
// 重构后的ChartWidget -- 仅负责图表渲染，不含数据管理逻辑
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);

    // 设置数据模型（必须在使用前调用）
    void setModel(ChartModel* model);

    // 获取数据模型
    ChartModel* model() const;

    // 添加一个显示通道（创建QLineSeries并注册到图表）
    void addChannel(const QString& name, const QColor& color = QColor());

    // 移除一个显示通道
    void removeChannel(const QString& name);

    // 清除所有通道数据
    void clear();

    // 获取当前通道列表
    QStringList channels() const;

    // 设置Y轴范围
    void setYRange(double min, double max);

    // 启用/禁用自动Y轴范围
    void setAutoYRange(bool enabled);

public slots:
    // 当ChartModel数据更新时调用，批量刷新所有受影响通道的渲染
    void onDataUpdated(const QStringList& updatedChannels);

    // 当ChartModel通道配置变更时调用，同步增删QLineSeries
    void onChannelsChanged();

    // 当ChartModel数据清除时调用
    void onDataCleared();

private slots:
    void onPauseToggled(bool paused);
    void onClearClicked();
    void onConfigClicked();     // 新增: 打开通道配置对话框

private:
    void setupUI();

    // 单个通道的渲染数据
    struct ChannelRenderData {
        QLineSeries* series;
        QColor color;
    };

    QChartView* m_chartView;
    QChart* m_chart;
    QValueAxis* m_xAxis;
    QValueAxis* m_yAxis;

    QMap<QString, ChannelRenderData> m_channels;
    int m_windowSize = 200;
    bool m_autoYRange = true;
    bool m_paused = false;

    // 数据模型
    ChartModel* m_model = nullptr;

    // 控制栏
    QPushButton* m_pauseBtn;
    QPushButton* m_clearBtn;
    QPushButton* m_configBtn;       // 新增: 通道配置按钮
    QComboBox* m_windowSizeCombo;
    QLabel* m_statusLabel;

    static const QVector<QColor> kDefaultColors;
};
```

## 依赖的公共组件

| 组件 | 文件 | 复用方式 |
|------|------|---------|
| `SettingsManager` | `utils/SettingsManager.h/cpp` | 持久化ChannelConfigSet到JSON配置文件 |
| `FieldDef` | `protocol/FrameDefinition.h` | ChannelConfigSet::generateDefaults根据FieldDef列表生成默认配置 |
| `FrameDefinition` | `protocol/FrameDefinition.h` | 字段列表来源，ChannelConfig的sourceField引用FieldDef.name |
| `FrameParser` | `protocol/FrameParser.h` | frameParsed信号驱动ChartModel接收数据 |
| `ThemeManager` | `core/ThemeManager.h/cpp` | 通道颜色适配当前主题（暗色/亮色下的默认颜色表） |

## 设计模式

### 1. 观察者模式 (Observer)

**应用位置**: ChartModel与ChartWidget之间的数据更新通知

**原因**: ChartModel作为数据层，ChartWidget作为表现层。ChartModel不知道谁在显示数据，通过Qt信号/槽机制（观察者模式的Qt实现）通知视图更新。这符合项目架构原则中"数据层不能依赖表现层"的约束。

**信号链路**:
```
FrameParser::frameParsed
    -> ChartModel::onFrameParsed          (数据层: 计算并缓冲数据)
    -> ChartModel::dataUpdated            (信号: 通知视图)
    -> ChartWidget::onDataUpdated         (表现层: 刷新渲染)
```

### 2. 策略模式 (Strategy)

**应用位置**: ChannelConfig中的SourceMode和降采样

**原因**: 数据源映射有两种策略（Direct单字段直接映射 / Combine双字段组合），降采样有多种策略（最大值/最小值/平均值/首值）。使用枚举+switch分支实现策略选择，避免为每种映射创建子类（过度设计）。当前阶段枚举足够，如果未来映射公式需要用户自定义表达式解析，再替换为表达式引擎策略。

### 3. 桥接模式 (Bridge)

**应用位置**: ChartModel将"数据管理"与"图表渲染"解耦

**原因**: ChartWidget原本将数据缓冲区管理和图表渲染耦合在一起。引入ChartModel作为数据管理的抽象层，ChartWidget仅持有ChartModel的引用来获取渲染数据。这使得:
- 同一ChartModel可以被多个视图共享（未来多窗口联动）
- ChartWidget可以独立测试渲染逻辑
- 数据缓冲区策略（滑动窗口大小、降采样）的变更不影响渲染代码

## 数据流设计

### 完整数据流

```
串口/TCP数据流
    |
    v
ConnectionManager::dataReceived (QByteArray)
    |
    v
FrameParser::feed (字节流状态机解析)
    |
    v
FrameParser::frameParsed (QVariantMap fields, QByteArray rawFrame)
    |
    +---> ProtocolView::onFrameParsed     (表格展示)
    |
    +---> ChartModel::onFrameParsed       (数据管理)
              |
              v
          ChannelConfigSet::computeAll    (按配置计算各通道值)
              |
              v
          ChartModel内部: 降采样 + 滑动窗口维护
              |
              v
          ChartModel::dataUpdated (QStringList updatedChannels)
              |
              v
          ChartWidget::onDataUpdated      (批量刷新渲染)
```

### ChannelConfig.compute 计算流程

```
输入: QVariantMap fields (来自FrameParser)

1. 检查 enabled == false -> 返回 NaN
2. 检查 sourceMode:
   a. Direct模式:
      - 从fields中取sourceField对应的值
      - 转double，失败返回NaN
      - 返回 scale * value + offset
   b. Combine模式:
      - 从fields中取sourceFieldA和sourceFieldB的值
      - 分别转double，任一失败返回NaN
      - 按 combineOp 计算: Add/Subtract/Multiply/Divide
      - Divide模式下B为0返回NaN
      - 返回 scale * result + offset
```

### ChartModel 降采样逻辑

```
输入: 一个新的数据点 (channelName, value)

1. 查找或创建通道缓冲区 ChannelBuffer
2. 递增 sampleCounter
3. 如果 sampleCounter % sampleDivisor != 0:
      丢弃此点，返回
4. 创建 QPointF(frameIndex, value)，追加到缓冲区
5. 如果缓冲区大小 > windowSize:
      移除最旧的点（FIFO）
6. 将 channelName 加入 pendingUpdates 列表
7. 如果 refreshInterval == 0:
      立即发射 dataUpdated(pendingUpdates)，清空列表
   否则:
      等待 refreshTimer 触发时批量发射
```

## 配置持久化设计

### SettingsManager中的存储结构

```
key: "chart/channelConfigs"
value: JSON对象，格式如下:

{
    "channels": [
        {
            "sourceMode": 0,           // 0=Direct, 1=Combine
            "sourceField": "温度",
            "sourceFieldA": "",
            "sourceFieldB": "",
            "combineOp": 0,            // 0=Add, 1=Subtract, 2=Multiply, 3=Divide
            "scale": 1.0,
            "offset": 0.0,
            "displayName": "温度",
            "color": "#89b4fa",        // 空字符串表示自动分配
            "enabled": true,
            "unit": "C",
            "sampleDivisor": 1
        },
        {
            "sourceMode": 1,           // Combine模式: 功率 = 电压 * 电流
            "sourceFieldA": "电压",
            "sourceFieldB": "电流",
            "combineOp": 2,            // Multiply
            "scale": 1.0,
            "offset": 0.0,
            "displayName": "功率",
            "color": "",
            "enabled": true,
            "unit": "W",
            "sampleDivisor": 1
        }
    ]
}

key: "chart/windowSize"
value: 200 (int)

key: "chart/refreshInterval"
value: 0 (int, 毫秒, 0=立即刷新)
```

### 自动发现与默认配置生成

当用户在FrameVisualEditor中修改帧格式定义后:
1. FrameVisualEditor发出 `definitionChanged(FrameDefinition)` 信号
2. MainWindow中的连接将新定义传递给 ChartModel
3. ChartModel调用 `ChannelConfigSet::generateDefaults(def.fields)` 生成默认配置
4. 默认配置规则:
   - 跳过type为`Raw`的字段
   - 每个数值字段生成一个Direct模式的ChannelConfig
   - displayName默认等于字段名
   - 颜色使用kDefaultColors自动分配
   - scale和offset沿用FieldDef中的值
5. 用户可通过配置面板修改、删除默认配置，或添加Combine模式的自定义通道

## 影响范围

| 文件 | 操作 | 说明 |
|------|------|------|
| `src/chart/ChannelConfig.h/cpp` | **新增** | 通道配置数据结构和计算逻辑 |
| `src/chart/ChartModel.h/cpp` | **新增** | 图表数据模型，数据缓冲区和滑动窗口管理 |
| `src/chart/ChartWidget.h/cpp` | **修改** | 移除内部数据管理和onFrameParsed，改为从ChartModel获取数据；新增Config按钮 |
| `src/core/MainWindow.h/cpp` | **修改** | 新增ChartModel成员；修改信号连接：FrameParser->ChartModel->ChartWidget |
| `src/utils/SettingsManager.h/cpp` | **修改** | 新增通道配置的保存/加载便捷方法 |
| `CMakeLists.txt` | **修改** | 添加ChannelConfig、ChartModel源文件 |

### MainWindow信号连接变更

```cpp
// 变更前 (当前):
connect(m_frameParser, &FrameParser::frameParsed,
        m_chartWidget, &ChartWidget::onFrameParsed);

// 变更后:
connect(m_frameParser, &FrameParser::frameParsed,
        m_chartModel, &ChartModel::onFrameParsed);
connect(m_chartModel, &ChartModel::dataUpdated,
        m_chartWidget, &ChartWidget::onDataUpdated);
connect(m_chartModel, &ChartModel::channelsChanged,
        m_chartWidget, &ChartWidget::onChannelsChanged);
connect(m_chartModel, &ChartModel::dataCleared,
        m_chartWidget, &ChartWidget::onDataCleared);
```

## 验收标准

### 功能验收

| # | 验收条件 | 验证方法 |
|---|---------|---------|
| AC1 | ChartModel接收到frameParsed信号后，根据ChannelConfig正确分发数据到各通道 | 单元测试: 构造包含多个数值字段的QVariantMap，验证各通道缓冲区中的数据值 |
| AC2 | Direct模式通道正确应用scale和offset | 单元测试: 配置scale=2.0, offset=10.0，输入value=5.0，验证缓冲区值为20.0 |
| AC3 | Combine模式通道正确执行A+B/A-B/A*B/A/B | 单元测试: 四种运算符各测一组，除法额外测试除零返回NaN |
| AC4 | disabled通道不接收数据 | 单元测试: 配置enabled=false，发送数据后验证缓冲区为空 |
| AC5 | 滑动窗口正确工作，数据点数不超过windowSize | 测试: windowSize=10，连续发送20帧，验证缓冲区只保留最后10个点 |
| AC6 | 降采样正确工作，sampleDivisor=3时每3帧保留1帧 | 测试: sampleDivisor=3，发送9帧，验证缓冲区只有3个点 |
| AC7 | refreshInterval > 0时，高频数据到来不会触发过多次渲染 | 测试: refreshInterval=100ms，100ms内发送50帧，验证dataUpdated信号只发射1次 |
| AC8 | ChannelConfigSet的JSON序列化和反序列化正确 | 单元测试: 构造配置集 -> toJson -> fromJson -> 验证各字段一致 |
| AC9 | ChartWidget重构后显示效果与重构前一致 | 集成测试: 使用相同帧数据，对比重构前后波形图显示结果 |
| AC10 | 通道配置通过SettingsManager持久化，重启应用后配置恢复 | 测试: 配置3个通道 -> 关闭应用 -> 重新打开 -> 验证通道配置完整恢复 |
| AC11 | 禁用某个通道后，对应曲线在图表中消失 | 测试: 在运行中禁用一个通道，验证曲线消失且不再更新 |
| AC12 | ChartWidget工具栏Config按钮可打开通道配置面板 | 手动测试: 点击Config按钮，验证配置面板弹出并显示当前通道列表 |
| AC13 | FrameDefinition变更后自动生成默认通道配置 | 测试: 在FrameVisualEditor中添加一个新字段，验证图表自动出现对应通道 |

### 性能验收

| # | 验收条件 | 验证方法 |
|---|---------|---------|
| AC14 | 10个通道、1000Hz帧率下，CPU占用不超过15% | 压力测试: 模拟1000Hz数据流，观察任务管理器CPU占用 |
| AC15 | 滑动窗口2000点、8通道时，图表滚动无卡顿 | 手动测试: 设置windowSize=2000，8个通道同时绘图，观察帧率 |
| AC16 | ChartModel::onFrameParsed单次执行时间不超过0.1ms | 性能测试: QElapsedTimer测量单次调用耗时 |

### 代码质量验收

| # | 验收条件 |
|---|---------|
| AC17 | ChannelConfig和ChartModel位于数据层，不依赖任何表现层类 |
| AC18 | ChartWidget重构后不包含QVariantMap、FieldDef等协议层直接引用 |
| AC19 | 所有新类遵循项目命名规范（PascalCase类名、m_前缀成员变量） |
| AC20 | 所有新类包含中文注释，头文件卫士、头文件引用顺序符合规范 |
| AC21 | 零编译错误，零编译警告 |
