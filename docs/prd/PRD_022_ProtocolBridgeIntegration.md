# PRD-022: Protocol Bridge 集成方案

## 背景

commit #23 已实现 JustFloat/FireWater 协议桥的核心解析能力:

- `IProtocolBridge` -- 协议桥抽象接口，定义 `feed(data)` / `reset()` / `name()` 和 `frameParsed(fields, rawFrame)` 信号
- `JustFloatBridge` -- VOFA+ JustFloat 小端浮点字节流解析，自动通道检测
- `FireWaterBridge` -- VOFA+ FireWater CSV 文本协议解析，支持头部行检测

但三个桥实现仍独立存在于 `src/protocol/` 目录中，未接入主窗口数据流。用户当前只能通过 FrameParser（帧定义模式）查看波形数据，无法在运行时选择 JustFloat 或 FireWater 协议桥。

**现状数据流** (commit #23):

```
ConnectionController::dataReceived(data)
    -> TerminalModel::appendReceived(data)
    -> FrameParser::feed(data)                <-- 唯一协议入口
    -> DataLogger::logData(data, Received)
```

```
FrameParser::frameParsed(fields, rawFrame)
    -> ProtocolView::onFrameParsed
    -> ChartModel::onFrameParsed              <-- 仅支持帧定义模式
```

**目标数据流** (PRD-022 完成后):

```
ConnectionController::dataReceived(data)
    -> TerminalModel::appendReceived(data)
    -> ProtocolBridgeManager::feed(data)      <-- 统一协议入口
    -> DataLogger::logData(data, Received)
         |
         +-> [FrameParser | JustFloatBridge | FireWaterBridge].feed(data)
         |
         v
    ProtocolBridgeManager::frameParsed(fields, rawFrame)
         -> ProtocolView::onFrameParsed
         -> ChartModel::onFrameParsed
```

**审查基准**: commit #23, score 25。

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | ProtocolBridgeManager -- 协议桥管理器，管理桥的创建、模式切换、信号转发 | P0 | protocol/ProtocolBridgeManager (新建) |
| R2 | ChartProtocolMode 枚举 -- 定义帧解析/JustFloat/FireWater 三种数据源模式 | P0 | core/Constants.h (修改) |
| R3 | UI集成 -- 在 ChartWidget 工具栏中添加协议桥选择下拉框 | P0 | chart/ChartWidget.h/cpp (修改), QSS主题文件 (修改) |
| R4 | ChannelConfigSet 兼容性 -- 桥产生的 QVariantMap 字段自动映射为通道配置 | P0 | chart/ChannelConfig.h/cpp (修改), chart/ChartModel.h/cpp (修改) |

---

## 需求详细说明

---

### R1: ProtocolBridgeManager (P0)

#### 问题分析

当前 MainWindow::connectSignals() 中有 3 处直接引用 `m_frameParser`:

1. 第 313 行: `m_frameParser->feed(data)` -- 数据接收时喂入解析器
2. 第 382-388 行: `m_frameParser->frameParsed` 信号连接到 ProtocolView 和 ChartModel
3. 第 393 行: `m_frameEditor->definitionChanged` 触发 `m_frameParser->setDefinition(def)`

要支持多协议桥切换，需要一个管理层来:
- 持有三种协议桥实例（FrameParser + JustFloatBridge + FireWaterBridge）
- 根据当前模式将 `feed(data)` 路由到正确的桥
- 转发活跃桥的 `frameParsed` 信号到下游（ProtocolView + ChartModel）
- 模式切换时断开旧桥信号、连接新桥信号、重置旧桥状态

#### 方案设计

**关键决策: FrameParser 不实现 IProtocolBridge 接口**

FrameParser 是经过多次迭代验证的成熟组件（PRD-003 定义，commit #3 实现），它有自己的 `frameParsed` 和 `frameError` 信号、`setDefinition()` 接口。让它继承 IProtocolBridge 需要:
- 修改 FrameParser 的基类
- 添加不必要的 `name()` / `reset()` 语义变化
- 增加回归风险

因此采用 **适配器模式**: ProtocolBridgeManager 内部同时持有 `FrameParser*` 和 `IProtocolBridge*` 实例，通过信号转发实现统一出口。

```
ProtocolBridgeManager 内部结构:

    m_frameParser (FrameParser)         -- 帧定义模式专用，不继承 IProtocolBridge
    m_justfloatBridge (JustFloatBridge) -- JustFloat 模式，继承 IProtocolBridge
    m_firewaterBridge (FireWaterBridge) -- FireWater 模式，继承 IProtocolBridge

信号转发机制:
    当 mode == FrameParser 时:
        FrameParser::frameParsed -> ProtocolBridgeManager::frameParsed (直接转发)
        FrameParser::frameError  -> ProtocolBridgeManager::frameError  (直接转发)

    当 mode == JustFloat 时:
        JustFloatBridge::frameParsed -> ProtocolBridgeManager::frameParsed (直接转发)

    当 mode == FireWater 时:
        FireWaterBridge::frameParsed -> ProtocolBridgeManager::frameParsed (直接转发)
```

##### 接口定义

```cpp
// protocol/ProtocolBridgeManager.h

#ifndef PROTOCOLBRIDGEMANAGER_H
#define PROTOCOLBRIDGEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>

#include "protocol/IProtocolBridge.h"

class FrameParser;
class JustFloatBridge;
class FireWaterBridge;
struct FrameDefinition;

// 协议桥管理器 -- 管理多种协议桥的创建、切换、信号转发
// 职责:
//   1. 持有 FrameParser / JustFloatBridge / FireWaterBridge 三个实例
//   2. 根据当前模式将 feed(data) 路由到正确的桥
//   3. 转发活跃桥的 frameParsed/frameError 信号到下游
//   4. 模式切换时管理信号连接的断开/重连
//   5. 桥检测到通道数变化时发出 channelInfoChanged 信号
//
// 设计模式: 策略模式(运行时切换桥) + 适配器模式(统一FrameParser和IProtocolBridge)
// 业务层: 不依赖表现层类
class ProtocolBridgeManager : public QObject {
    Q_OBJECT

public:
    explicit ProtocolBridgeManager(QObject* parent = nullptr);
    ~ProtocolBridgeManager() override;

    // ---- 模式管理 ----

    // 设置当前协议模式（切换内部活跃桥）
    void setMode(ChartProtocolMode mode);

    // 获取当前协议模式
    ChartProtocolMode mode() const;

    // ---- 数据输入 ----

    // 向当前活跃桥喂入原始字节流（来自串口/网络）
    void feed(const QByteArray& data);

    // ---- 配置接口 ----

    // 设置 FrameParser 的帧定义（仅 FrameParser 模式有效）
    void setFrameDefinition(const FrameDefinition& def);

    // 设置 JustFloat 固定通道数（0=自动检测，仅 JustFloat 模式有效）
    void setJustFloatFixedChannelCount(int count);

    // 设置 FireWater 分隔符（默认逗号，仅 FireWater 模式有效）
    void setFireWaterDelimiter(const QString& delimiter);

    // ---- 状态查询 ----

    // 获取内部 FrameParser 实例（供 FrameVisualEditor 直接操作）
    // 注意: 仅在 FrameParser 模式下调用 setDefinition 有意义
    FrameParser* frameParser() const;

    // 获取当前活跃桥的名称
    QString activeBridgeName() const;

    // 重置所有桥的内部状态
    void resetAll();

    // 重置当前活跃桥的内部状态
    void resetActive();

signals:
    // 统一的帧解析结果信号（与 FrameParser::frameParsed 签名一致）
    // 下游组件（ProtocolView, ChartModel）连接此信号即可，无需关心协议来源
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    // 帧解析错误信号（仅 FrameParser 模式会发出）
    void frameError(const QString& reason, const QByteArray& rawFrame);

    // 协议模式切换完成
    void modeChanged(ChartProtocolMode newMode);

    // 桥检测到通道信息变化（通道数/名称更新）
    // names: 自动检测到的通道名称列表（如 ["CH1","CH2","CH3"]）
    void channelInfoChanged(const QStringList& names);

private:
    // 切换内部信号连接: 断开旧桥、连接新桥
    void reconnectSignals();

    // 持有的三个桥实例（本类拥有，在析构时自动销毁）
    FrameParser* m_frameParser;
    JustFloatBridge* m_justfloatBridge;
    FireWaterBridge* m_firewaterBridge;

    ChartProtocolMode m_mode;

    // 当前活跃的信号连接（用于模式切换时断开）
    QMetaObject::Connection m_frameParsedConn;
    QMetaObject::Connection m_frameErrorConn;
};

#endif // PROTOCOLBRIDGEMANAGER_H
```

##### 实现要点

1. **构造函数**: 创建三个桥实例（均为 `this` 的子 QObject，自动管理生命周期）。默认模式为 `ChartProtocolMode::FrameParser`。初始连接 FrameParser 的信号。

2. **setMode(mode)**:
   - 如果与当前模式相同，跳过
   - 调用当前活跃桥的 `reset()` 清空缓冲区
   - 调用 `reconnectSignals()` 断开旧桥信号、连接新桥信号
   - 发出 `modeChanged(mode)` 信号
   - 如果新桥已有通道信息，立即发出 `channelInfoChanged(names)`

3. **feed(data)**:
   - 根据 `m_mode` 路由到对应桥的 `feed()` 方法
   - FrameParser 模式: `m_frameParser->feed(data)`
   - JustFloat 模式: `m_justfloatBridge->feed(data)`
   - FireWater 模式: `m_firewaterBridge->feed(data)`

4. **reconnectSignals()**:
   - 断开 `m_frameParsedConn` 和 `m_frameErrorConn`
   - 根据 `m_mode` 连接新桥:
     ```cpp
     case FrameParser:
         m_frameParsedConn = connect(m_frameParser, &FrameParser::frameParsed,
             this, &ProtocolBridgeManager::frameParsed);
         m_frameErrorConn = connect(m_frameParser, &FrameParser::frameError,
             this, &ProtocolBridgeManager::frameError);
         break;
     case JustFloat:
         m_frameParsedConn = connect(m_justfloatBridge, &JustFloatBridge::frameParsed,
             this, &ProtocolBridgeManager::frameParsed);
         // JustFloat 无 frameError 信号，不连接
         break;
     case FireWater:
         m_frameParsedConn = connect(m_firewaterBridge, &FireWaterBridge::frameParsed,
             this, &ProtocolBridgeManager::frameParsed);
         // FireWater 无 frameError 信号，不连接
         break;
     ```

5. **通道信息变化检测**: JustFloatBridge 和 FireWaterBridge 在首次检测到通道时，发出带通道名的 QVariantMap。ProtocolBridgeManager 可通过对比前后帧的字段名列表来检测通道信息变化并发出 `channelInfoChanged` 信号。也可在各桥内部添加 `channelNames()` 查询接口，由 Manager 在模式切换或首次解析后主动查询并通知。

6. **frameParser() 访问器**: 保留此方法供 `FrameVisualEditor::definitionChanged` 信号直接设置帧定义。MainWindow 仍需此访问路径。

---

### R2: ChartProtocolMode 枚举 (P0)

#### 方案设计

在 `Constants.h` 中新增枚举，定义三种数据源模式:

```cpp
// 图表协议模式 -- 控制数据源使用哪种协议解析
enum class ChartProtocolMode {
    FrameParser,    // 用户定义帧格式（默认模式，向后兼容）
    JustFloat,      // VOFA+ JustFloat 小端浮点字节流
    FireWater       // VOFA+ FireWater CSV 文本协议
};
```

**设计决策**:

1. 放在 `Constants.h` 而非新建枚举文件: 与 `ConnectionType` / `DisplayMode` / `ConnectionState` 等全局枚举保持一致，减少文件碎片化。

2. 使用 `enum class` 而非 `enum`: 遵循 CLAUDE.md 5.2 编码规范，避免命名污染。

3. 默认值为 `FrameParser`: 向后兼容，不改变现有用户行为。

4. 枚举值顺序: FrameParser 在前，因为它是默认模式和最常用的模式。

---

### R3: UI集成 -- 协议桥选择入口 (P0)

#### 问题分析

用户需要一个入口来选择当前使用的协议桥。根据 FEATURE-001 的 Open Question #1，有两个候选位置:

**方案 A: ChartWidget 工具栏** (推荐)
- 位置: ChartWidget 现有的控制栏（已有暂停按钮、清除按钮、窗口大小下拉框）
- 优点: 协议选择与波形图强相关，本地化在图表区域内符合直觉
- 缺点: 不可见时（切换到其他面板）无法看到当前模式

**方案 B: MainWindow 主工具栏**
- 位置: 顶部工具栏，与显示模式/主题下拉框并列
- 优点: 始终可见
- 缺点: 工具栏空间有限，协议选择不在波形面板的上下文中

**选择方案 A**: 协议桥选择是波形数据源配置，放在 ChartWidget 工具栏中最符合操作逻辑。用户使用波形功能时自然会查看波形面板，此时协议选择入口就在眼前。

#### 方案设计

##### ChartWidget 工具栏变更

在现有的控制栏中新增一个 QComboBox，位于窗口大小下拉框之后:

```
[暂停] [清除] | 窗口: [200 v] | 协议: [帧定义 v] | 状态: 0 点
```

QComboBox 项目:
- "帧定义" (FrameParser)
- "JustFloat"
- "FireWater"

##### 接口变更

```cpp
// ChartWidget.h 新增:

// 协议模式变更信号（通知 MainWindow 切换 ProtocolBridgeManager 模式）
signals:
    void protocolModeChanged(ChartProtocolMode mode);

private slots:
    // 协议下拉框选择变更
    void onProtocolModeChanged(int index);
```

ChartWidget::setupUI() 中新增:

```cpp
// 协议模式选择下拉框
m_protocolModeCombo = new QComboBox();
m_protocolModeCombo->setObjectName("protocolModeCombo");
m_protocolModeCombo->addItem(tr("帧定义"), static_cast<int>(ChartProtocolMode::FrameParser));
m_protocolModeCombo->addItem("JustFloat", static_cast<int>(ChartProtocolMode::JustFloat));
m_protocolModeCombo->addItem("FireWater", static_cast<int>(ChartProtocolMode::FireWater));
m_protocolModeCombo->setCurrentIndex(0);
connect(m_protocolModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ChartWidget::onProtocolModeChanged);
```

```cpp
void ChartWidget::onProtocolModeChanged(int index)
{
    auto mode = static_cast<ChartProtocolMode>(m_protocolModeCombo->currentData().toInt());
    emit protocolModeChanged(mode);
}
```

##### MainWindow 信号连接

MainWindow::connectSignals() 中新增:

```cpp
// ChartWidget 协议模式切换 -> ProtocolBridgeManager
connect(m_chartWidget, &ChartWidget::protocolModeChanged,
        m_bridgeManager, &ProtocolBridgeManager::setMode);

// ProtocolBridgeManager 模式变更 -> 持久化
connect(m_bridgeManager, &ProtocolBridgeManager::modeChanged,
        this, [this](ChartProtocolMode mode) {
    SettingsManager::instance().saveProtocolMode(
        static_cast<int>(mode));
});

// ProtocolBridgeManager 通道信息变更 -> ChartWidget 自动配置通道
connect(m_bridgeManager, &ProtocolBridgeManager::channelInfoChanged,
        this, [this](const QStringList& names) {
    // 从桥的通道名列表自动生成 ChannelConfigSet
    // 详见 R4
});
```

##### 模式切换时的 ChartWidget 行为

当 ProtocolBridgeManager::modeChanged 触发时:
1. ChartWidget 应清除当前所有通道数据（`m_model->clear()`）
2. ChartWidget 应清除当前 ChannelConfigSet（准备接收新协议的通道配置）
3. 等待新桥的 `frameParsed` 信号携带新字段名，触发 R4 的自动通道映射

##### ProtocolView 兼容

在 JustFloat/FireWater 模式下:
- ProtocolView 仍接收 `frameParsed` 信号，正常显示字段值
- `frameError` 信号仅在 FrameParser 模式下发出，JustFloat/FireWater 模式下 ProtocolView 不显示错误行
- ProtocolView 的右键菜单"导出JSON"功能无需修改，它只消费 QVariantMap

##### 模式持久化

SettingsManager 新增方法:

```cpp
// 保存/加载协议模式
void saveProtocolMode(int mode);
int loadProtocolMode() const;  // 默认返回 0 (FrameParser)
```

MainWindow::loadSettings() 中恢复:

```cpp
int savedMode = settings.loadProtocolMode();
m_bridgeManager->setMode(static_cast<ChartProtocolMode>(savedMode));
// 同步 ChartWidget 下拉框
m_protocolModeCombo->setCurrentIndex(savedMode);
```

> 注意: 需要通过 ChartWidget 暴露 `setProtocolMode()` 方法或直接同步 ComboBox 索引。

##### QSS 样式

三个主题文件中新增:

```css
/* 协议模式选择下拉框 */
QComboBox#protocolModeCombo {
    min-width: 90px;
    max-width: 120px;
}
```

样式复用现有 ComboBox 样式（6.6 组件设计规范），无需新增语义色。

---

### R4: ChannelConfigSet 兼容性 -- QVariantMap 字段自动映射 (P0)

#### 问题分析

当前 ChannelConfigSet 的通道生成依赖 `FrameDefinition` 中的 `FieldDef` 列表:

```cpp
// ChannelConfig.h 第 126 行
static ChannelConfigSet generateDefaults(const QVector<FieldDef>& fields);
```

FrameDefinition -> FieldDef -> ChannelConfigSet 是完整链路。但 JustFloat/FireWater 桥没有 FrameDefinition，它们直接从数据中自动检测通道。

桥产生的 `frameParsed` 信号中 `QVariantMap fields` 的键就是通道名:
- JustFloat: `{"CH1": 3.14, "CH2": 2.72, "CH3": 1.0}`
- FireWater: `{"温度": 25.5, "湿度": 60.2}` 或 `{"CH1": 100, "CH2": 200}`

ChartModel::onFrameParsed() 接收这些 QVariantMap 后，需要通过 ChannelConfigSet 来确定哪些通道需要记录数据。因此需要一个从 QVariantMap 字段名到 ChannelConfigSet 的映射机制。

#### 方案设计

##### 新增 ChannelConfigSet::generateFromFieldNames()

```cpp
// ChannelConfig.h 新增静态方法:

// 从字段名称列表生成默认通道配置
// 用于 JustFloat/FireWater 协议桥的自动通道映射
// 每个字段名生成一个 Direct 模式的 ChannelConfig
// displayName 等于字段名，颜色使用 kDefaultColors 自动分配
// scale = 1.0, offset = 0.0, sampleDivisor = 1
static ChannelConfigSet generateFromFieldNames(const QStringList& fieldNames);
```

实现逻辑与 `generateDefaults(const QVector<FieldDef>& fields)` 基本一致，区别在于:
- 输入是 `QStringList`（字段名列表）而非 `FieldDef` 列表
- 不需要过滤 `type == Raw` 的字段（桥的通道全部是数值类型）
- scale/offset/sampleDivisor 使用默认值

##### 自动通道配置触发时机

两种触发路径:

**路径 1: 首帧解析触发（推荐）**

ChartModel 内部检测 fields 的键与当前 ChannelConfigSet 的通道名是否匹配。如果不匹配（即通道结构变化），自动重建配置。

```cpp
// ChartModel::onFrameParsed() 内部变更:

void ChartModel::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    // 检测通道结构是否变化
    if (m_configSet.channels().isEmpty() ||
        !isFieldMatch(fields)) {
        // 自动生成新配置
        QStringList fieldNames = fields.keys();
        ChannelConfigSet newConfig = ChannelConfigSet::generateFromFieldNames(fieldNames);
        setChannelConfigSet(newConfig);
    }

    // 原有的数据分发逻辑
    // ...
}
```

新增辅助方法:

```cpp
// 检查 fields 的键是否与当前 ChannelConfigSet 的通道名匹配
bool isFieldMatch(const QVariantMap& fields) const;
```

**路径 2: 信号驱动**

ProtocolBridgeManager 检测到通道信息变化后发出 `channelInfoChanged(names)` 信号，MainWindow 在槽中调用:

```cpp
connect(m_bridgeManager, &ProtocolBridgeManager::channelInfoChanged,
        this, [this](const QStringList& names) {
    ChannelConfigSet config = ChannelConfigSet::generateFromFieldNames(names);
    m_chartWidget->model()->setChannelConfigSet(config);
});
```

**选择路径 2 作为主路径，路径 1 作为兜底保护**:
- 路径 2 在通道信息变化时立即重建配置，不需要等到首帧数据到达 ChartModel
- 路径 1 在路径 2 未触发时（例如用户手动清除了通道配置后桥又发出数据）提供兜底

##### ChartWidget::configureFromFrameDefinition 保留

现有的 `ChartWidget::configureFromFrameDefinition(const FrameDefinition& def)` 方法在 FrameParser 模式下仍然有效。当 ProtocolBridgeManager 处于 FrameParser 模式时，FrameVisualEditor 的 definitionChanged 信号仍通过此路径配置通道。

当模式为 JustFloat 或 FireWater 时，FrameVisualEditor 的定义变更信号应被忽略（因为桥不使用帧定义），ProtocolBridgeManager::setMode() 已将数据路由到其他桥，FrameParser 收不到数据。

##### 模式切换时通道重建的完整流程

1. 用户在 ChartWidget 下拉框中选择 "JustFloat"
2. ChartWidget 发出 `protocolModeChanged(JustFloat)`
3. ProtocolBridgeManager::setMode(JustFloat):
   - reset 旧的 FrameParser
   - reconnectSignals() 切换到 JustFloatBridge
   - emit modeChanged(JustFloat)
4. MainWindow 收到 modeChanged:
   - SettingsManager::saveProtocolMode
   - ChartModel::clear() 清除旧数据
5. 串口数据持续到达，JustFloatBridge 开始解析
6. JustFloatBridge 首次检测到通道数（例如 3 通道）:
   - emit frameParsed({"CH1": val1, "CH2": val2, "CH3": val3}, rawFrame)
7. ProtocolBridgeManager 转发 frameParsed 到 ChartModel
8. ChartModel::onFrameParsed 检测到配置为空，自动调用:
   ```cpp
   ChannelConfigSet::generateFromFieldNames({"CH1", "CH2", "CH3"})
   ```
9. 新配置生效，ChartWidget 创建 3 条曲线并开始绘制

---

## 接口设计

### 新增接口

| 接口 | 文件 | 说明 |
|------|------|------|
| `ProtocolBridgeManager` | `protocol/ProtocolBridgeManager.h/cpp` (新建) | 协议桥管理器 |
| `ChartProtocolMode` 枚举 | `core/Constants.h` (修改) | 三种数据源模式 |
| `ChannelConfigSet::generateFromFieldNames()` | `chart/ChannelConfig.h/cpp` (修改) | 从字段名列表生成通道配置 |
| `ChartModel::isFieldMatch()` | `chart/ChartModel.h/cpp` (修改) | 检测字段与通道配置匹配 |
| `ChartWidget::protocolModeChanged()` 信号 | `chart/ChartWidget.h/cpp` (修改) | 协议模式变更信号 |
| `ChartWidget::onProtocolModeChanged()` 槽 | `chart/ChartWidget.h/cpp` (修改) | 下拉框选择处理 |
| `SettingsManager::saveProtocolMode()` | `utils/SettingsManager.h/cpp` (修改) | 协议模式持久化 |
| `SettingsManager::loadProtocolMode()` | `utils/SettingsManager.h/cpp` (修改) | 协议模式恢复 |

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| `MainWindow` 构造函数 | 内部变更: 新增 ProtocolBridgeManager 初始化，替换 m_frameParser | 无外部影响 |
| `MainWindow::connectSignals()` | 内部变更: 信号连接委托给 ProtocolBridgeManager | 无外部影响 |
| `MainWindow::loadSettings()` | 内部变更: 恢复协议模式设置 | 无外部影响 |
| `MainWindow::saveSettings()` | 内部变更: 保存协议模式设置 | 无外部影响 |
| `ChartWidget::setupUI()` | 内部变更: 新增协议下拉框 | 无外部影响 |
| `ChartModel::onFrameParsed()` | 内部变更: 增加自动通道配置检测逻辑 | 无外部影响 |

### 移除接口

无。现有接口全部保留，仅变更内部实现。

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| `IProtocolBridge` | `protocol/IProtocolBridge.h` | ProtocolBridgeManager 持有 IProtocolBridge 指针 | R1 |
| `JustFloatBridge` | `protocol/JustFloatBridge.h` | ProtocolBridgeManager 创建并管理实例 | R1 |
| `FireWaterBridge` | `protocol/FireWaterBridge.h` | ProtocolBridgeManager 创建并管理实例 | R1 |
| `FrameParser` | `protocol/FrameParser.h` | ProtocolBridgeManager 持有并适配 | R1 |
| `ChannelConfigSet` | `chart/ChannelConfig.h` | 新增 generateFromFieldNames 方法 | R4 |
| `ChartModel` | `chart/ChartModel.h` | 接收桥的 frameParsed 信号 | R4 |
| `ChartWidget` | `chart/ChartWidget.h` | 新增协议模式选择 UI | R3 |
| `SettingsManager` | `utils/SettingsManager.h` | 协议模式持久化 | R3 |
| `ThemeManager` | `core/ThemeManager.h` | QSS 样式自动应用 | R3 |
| `ConnectionController` | `core/ConnectionController.h` | dataReceived 信号中转 | R1 |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **策略模式 (Strategy)** | IProtocolBridge 是策略接口，ProtocolBridgeManager 在运行时切换策略 | R1 | 与 CLAUDE.md 4.1 中策略模式要求一致 |
| **适配器模式 (Adapter)** | ProtocolBridgeManager 适配 FrameParser 到统一信号出口 | R1 | FrameParser 不实现 IProtocolBridge，通过信号转发适配 |
| **观察者模式 (Observer)** | Qt 信号/槽链路: 桥 -> Manager -> ChartModel/ProtocolView | R1,R3,R4 | 保持现有观察者模式不变 |
| **中介者模式 (Mediator)** | MainWindow 作为 ProtocolBridgeManager 与 ChartWidget 之间的中介 | R3 | ChartWidget 不直接持有 ProtocolBridgeManager |

---

## 影响范围

### 文件变更矩阵

| 文件 | 变更类型 | R1 | R2 | R3 | R4 |
|------|---------|-----|-----|-----|-----|
| `src/protocol/ProtocolBridgeManager.h` | 新建 | 约 80 行 | -- | -- | -- |
| `src/protocol/ProtocolBridgeManager.cpp` | 新建 | 约 180 行 | -- | -- | -- |
| `src/core/Constants.h` | 修改 | -- | +5 行 | -- | -- |
| `src/chart/ChartWidget.h` | 修改 | -- | -- | +15 行 | -- |
| `src/chart/ChartWidget.cpp` | 修改 | -- | -- | +30 行 | -- |
| `src/chart/ChannelConfig.h` | 修改 | -- | -- | -- | +3 行 |
| `src/chart/ChannelConfig.cpp` | 修改 | -- | -- | -- | +25 行 |
| `src/chart/ChartModel.h` | 修改 | -- | -- | -- | +3 行 |
| `src/chart/ChartModel.cpp` | 修改 | -- | -- | -- | +25 行 |
| `src/core/MainWindow.h` | 修改 | +2 行 | -- | -- | -- |
| `src/core/MainWindow.cpp` | 修改 | +30 行, -10 行 | -- | +5 行 | +10 行 |
| `src/utils/SettingsManager.h` | 修改 | -- | -- | +3 行 | -- |
| `src/utils/SettingsManager.cpp` | 修改 | -- | -- | +15 行 | -- |
| `resources/themes/dark_terminal.qss` | 修改 | -- | -- | +5 行 | -- |
| `resources/themes/modern_dark.qss` | 修改 | -- | -- | +5 行 | -- |
| `resources/themes/light.qss` | 修改 | -- | -- | +5 行 | -- |
| `CMakeLists.txt` | 修改 | +2 行 | -- | -- | -- |

### 预计变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| ProtocolBridgeManager.h | 约 80 行 | -- | -- |
| ProtocolBridgeManager.cpp | 约 180 行 | -- | -- |
| Constants.h | 5 行 | -- | -- |
| ChartWidget.h/cpp | 45 行 | -- | -- |
| ChannelConfig.h/cpp | 28 行 | -- | -- |
| ChartModel.h/cpp | 28 行 | -- | -- |
| MainWindow.h/cpp | 47 行 | 约 10 行 | 约 10 行 |
| SettingsManager.h/cpp | 18 行 | -- | -- |
| QSS 主题文件 (3x) | 15 行 | -- | -- |
| CMakeLists.txt | 2 行 | -- | -- |
| **合计** | **约 448 行** | **约 10 行** | **约 10 行** |

### 跨模块影响评估

- **ProtocolBridgeManager 与 ConnectionController**: ConnectionController 的 `dataReceived` 信号需要被 MainWindow 路由到 ProtocolBridgeManager::feed()。ConnectionController 本身不变，仅 MainWindow 的信号连接代码变更。
- **ProtocolBridgeManager 与 ChartWidget**: ChartWidget 通过 `protocolModeChanged` 信号通知模式变更，MainWindow 作为中介转发给 ProtocolBridgeManager。ChartWidget 不直接持有 ProtocolBridgeManager。
- **ProtocolBridgeManager 与 FrameVisualEditor**: FrameVisualEditor 的 `definitionChanged` 信号仍连接到 MainWindow，MainWindow 通过 `m_bridgeManager->setFrameDefinition(def)` 转发。在 JustFloat/FireWater 模式下此调用无效但不报错。
- **ChartModel 与 ChannelConfigSet**: ChartModel 新增自动通道检测逻辑，仅在配置为空或不匹配时触发，不影响现有 FrameParser 模式下的通道配置流程。

---

## 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | ProtocolBridgeManager.h/cpp 编译通过 | `cmake --build build` | 0 error, 0 warning |
| R1-AC2 | FrameParser 模式下数据流正常 | 手动: 定义帧格式 -> 接收数据 -> ProtocolView和ChartWidget显示正常 | 与 commit #23 行为一致 |
| R1-AC3 | JustFloat 模式下数据流正常 | 手动: 切换到JustFloat -> 发送小端浮点+NaN尾标 -> ChartWidget显示曲线 | 曲线数=通道数 |
| R1-AC4 | FireWater 模式下数据流正常 | 手动: 切换到FireWater -> 发送CSV行 -> ChartWidget显示曲线 | 曲线数=列数 |
| R1-AC5 | 模式切换不需要重连串口 | 手动: 已连接状态下切换模式 | 不触发断开/重连 |
| R1-AC6 | 模式切换时旧数据清除 | 手动: 切换模式 -> 检查ChartWidget | 旧曲线消失，状态归零 |
| R2-AC1 | ChartProtocolMode 枚举定义正确 | 代码检查: 3个值 (FrameParser, JustFloat, FireWater) | 编译通过 |
| R3-AC1 | ChartWidget 工具栏显示协议下拉框 | 手动: 切换到波形图面板 | 看到包含"帧定义/JustFloat/FireWater"的下拉框 |
| R3-AC2 | 协议下拉框切换触发模式变更 | 手动: 选择JustFloat | ProtocolBridgeManager模式切换为JustFloat |
| R3-AC3 | 协议模式持久化 | 手动: 选择FireWater -> 重启应用 | 默认模式恢复为FireWater |
| R3-AC4 | QSS主题样式覆盖 | 手动: 切换三个主题 | 下拉框样式在所有主题下正常 |
| R4-AC1 | JustFloat 通道自动映射 | 手动: 发送3通道JustFloat数据 | ChartWidget自动创建3条曲线，名称为CH1/CH2/CH3 |
| R4-AC2 | FireWater 通道自动映射 | 手动: 发送带头部行的CSV | 使用头部行名称作为通道名 |
| R4-AC3 | 切换回FrameParser模式通道恢复 | 手动: 从JustFloat切回帧定义 -> 设置帧格式 -> 接收数据 | 通道按帧定义配置 |
| AC-1 | 编译零错误零警告 | `cmake --build build` | 0 error, 0 warning |
| AC-2 | EmbedDebug.bat 正常启动 | 双击 EmbedDebug.bat | 应用窗口正常显示 |
| AC-3 | 现有功能回归: 串口收发 | 手动: 终端收发数据 | 功能正常 |
| AC-4 | 现有功能回归: OTA传输 | 手动: XMODEM传输 | 功能正常 |
| AC-5 | 现有功能回归: 数据录制回放 | 手动: 录制 -> 回放 | 功能正常 |

---

## 实施优先级

| 顺序 | 步骤 | 理由 |
|------|------|------|
| 1 | Constants.h 新增 ChartProtocolMode 枚举 (R2) | 基础依赖，其他模块引用 |
| 2 | 创建 ProtocolBridgeManager.h/cpp (R1) | 核心管理器，独立于 UI |
| 3 | ChannelConfigSet::generateFromFieldNames() (R4) | 通道自动映射，独立于管理器 |
| 4 | ChartModel 自动通道检测 (R4) | 依赖 R4 的 generateFromFieldNames |
| 5 | ChartWidget UI 集成 (R3) | 依赖 R1 和 R2 |
| 6 | MainWindow 信号连接重构 (R1+R3+R4) | 串联所有组件 |
| 7 | SettingsManager 持久化 (R3) | 独立于核心逻辑 |
| 8 | QSS 主题文件更新 (R3) | UI 样式，最后处理 |
| 9 | 编译验证 | 确保零错误 |
| 10 | 手动功能测试 | 验证端到端流程 |

---

## 验证度量指标

### 代码度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| ProtocolBridgeManager.h 行数 | wc -l | 新建 | < 100 行 |
| ProtocolBridgeManager.cpp 行数 | wc -l | 新建 | < 250 行 |
| MainWindow.cpp 行数变化 | wc -l | commit #23 值 | +20/-10 行以内 |
| ChannelConfig.cpp 新增行数 | diff | 0 | < 30 行 |
| ChartModel.cpp 新增行数 | diff | 0 | < 30 行 |

### 架构度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| ProtocolBridgeManager 持有的 UI 组件指针数 | 代码检查 | 0 (不持有任何 QWidget) |
| ChartWidget 对 ProtocolBridgeManager 的直接依赖 | 代码检查 | 0 (通过信号/中介者通信) |
| 新增类所属层次正确性 | 代码检查 | ProtocolBridgeManager 在业务层, 枚举在基础设施常量 |
| 分层依赖方向 | 代码检查 | 表现层 -> 业务层 -> 数据层, 无反向依赖 |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| 三种模式端到端数据流 | 手动测试 | 全部通过 |
| 模式切换无重连 | 手动测试 | 连接保持 |
| JustFloat 通道自动检测 | 手动测试: 发送4通道数据 | 4条曲线自动出现 |
| FireWater 头部行检测 | 手动测试: 发送"temp,hum\n25,60\n" | 通道名为temp/hum |
| 现有功能回归 | 手动测试: 终端/OTA/录制 | 100% 通过 |
| 编译零错误零警告 | cmake --build | 0 error, 0 warning |
| EmbedDebug.bat 启动 | 双击 bat 文件 | 正常启动 |
