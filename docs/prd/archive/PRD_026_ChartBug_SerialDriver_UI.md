# PRD-026: ChartWidget渲染Bug修复 + 串口驱动检测 + UI打磨 + 方向前缀着色

## 背景

commit #25 已完成 P0 串口功能修复（终端复制、搜索高亮、DTR/RTS控制、自动换行符），当前评分 27 分。

本次迭代 #26 是常规功能迭代，聚焦四个方向:

1. **ChartWidget渲染Bug**: 波形图控件在初始显示时仅展示背景/边框，直到用户点击 "Clear" 按钮后才正常渲染图表区域。此Bug导致用户首次进入波形图面板时看到的是空白区域，严重影响第一印象。需要排查 ChartView 初始化流程、尺寸策略、以及无数据时的渲染路径。

2. **串口驱动检测**: 嵌入式开发者在首次使用串口工具时，常遇到"插上设备但没有驱动"的问题（CH340/CP2102/FT232/PL2303/CP210x 等）。本需求在启动时和端口刷新时，通过 Windows SetupAPI 或 WMI 检测系统已安装的串口驱动，并在 SerialConfigPanel 中展示驱动信息，无驱动时给出安装警告。

3. **UI打磨 -- SerialConfigPanel objectName 审计**: SerialConfigPanel 当前多个子控件缺少 objectName，导致 QSS 无法精确选择器匹配。同时端口下拉框应展示驱动信息，面板整体需要符合 CLAUDE.md 6.3 节的现代间距和视觉层次标准。

4. **方向前缀着色**: TerminalWidget 的 `formatToCache()` 方法在 `m_showDirectionPrefix` 为 true 时，将 `[TX:]` 和 `[RX:]` 前缀拼接到 `cached.text` 中，但 paintEvent 中整行使用单一颜色（按 direction 选 m_txColor 或 m_rxColor）渲染。需求是 `[TX:]` 使用绿色 `#a6e3a1`，`[RX:]` 使用蓝色 `#89b4fa`，通过 QPainter 分段绘制实现。

**审查基准**: commit #25, score 27。

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | ChartWidget初始化渲染Bug修复 -- 首次显示即渲染完整图表区域，无需先点击Clear | P0 | chart/ChartWidget.h/cpp |
| R2 | 串口驱动检测 -- Windows SetupAPI/WMI检测已安装驱动，SerialConfigPanel展示驱动信息和安装警告 | P1 | serial/SerialConfigPanel.h/cpp, 新增 serial/SerialDriverDetector.h/cpp |
| R3 | SerialConfigPanel objectName审计 -- 所有子控件设置objectName，QSS可精确匹配 | P1 | serial/SerialConfigPanel.h/cpp, resources/themes/*.qss |
| R4 | 方向前缀着色 -- [TX:]绿色#00a6e3a1, [RX:]蓝色#89b4fa，QPainter分段渲染 | P1 | terminal/TerminalWidget.h/cpp |

---

## 需求详细说明

---

### R1: ChartWidget初始化渲染Bug修复 (P0)

#### 问题分析

ChartWidget 在首次显示时，波形图区域仅展示 QChart 的背景（深灰色矩形），看不到坐标轴、网格线和标题。直到用户点击 "Clear" 按钮后，图表区域才正常渲染出完整的坐标系。

经过代码审查，可能的根因分析:

1. **QChartView 初始尺寸为零**: ChartWidget::setupUI() 中，`m_chartView` 被添加到 layout 时 stretch factor 为 1（`layout->addWidget(m_chartView, 1)`），但 QWidget 的 layout 在首次 show() 之前可能尚未完成几何计算。QChartView 需要有效的 widget 尺寸才能触发 QChart 的首次布局计算。

2. **轴范围未触发重绘**: setupUI() 中设置了 `m_xAxis->setRange(0, 10)` 和 `m_yAxis->setRange(0, 100)`，但这些设置发生在 QChart 被添加到 QChartView 之前或同时。QChart 的内部布局可能在首次 show() 时才计算 plotArea，导致初始轴范围设置被跳过。

3. **onClearClicked 触发正确渲染的原因**: `onDataCleared()` 槽函数中调用了 `m_xAxis->setRange(0, 10)` 和 `m_yAxis->setRange(0, 100)`，此时 QChartView 已经有了有效的几何尺寸，QChart 能够正确计算 plotArea 并触发布局更新，因此点击 Clear 后图表正常显示。

#### 方案设计

##### 修复策略: showEvent 中强制触发首次布局

在 ChartWidget 中重写 `showEvent(QShowEvent*)`，在控件首次变为可见时强制触发 QChart 的布局更新:

```cpp
// ChartWidget.h 新增:
protected:
    void showEvent(QShowEvent* event) override;

private:
    bool m_firstShow = true;
```

```cpp
// ChartWidget.cpp 新增:
void ChartWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (m_firstShow) {
        m_firstShow = false;
        // 强制QChart重新计算布局，确保坐标轴和plotArea正确初始化
        m_chart->createDefaultAxes();
        m_xAxis->setRange(0, 10);
        m_yAxis->setRange(0, 100);
        m_chartView->update();
    }
}
```

##### 备选方案: QTimer::singleShot 延迟初始化

如果 showEvent 方案在某些 Qt 版本下仍然不够（QChartView 的内部布局可能延迟到事件循环下一轮），使用 `QTimer::singleShot` 在事件循环空闲后触发:

```cpp
// setupUI() 末尾:
QTimer::singleShot(0, this, [this]() {
    m_xAxis->setRange(0, 10);
    m_yAxis->setRange(0, 100);
    m_chartView->update();
});
```

##### 推荐方案: 双保险

同时使用 showEvent 和 setMinimumSize，确保 QChartView 始终有有效的初始尺寸:

```cpp
// setupUI() 中，m_chartView 创建后:
m_chartView = new QChartView(m_chart);
m_chartView->setRenderHint(QPainter::Antialiasing);
m_chartView->setMinimumSize(200, 150);  // 确保初始尺寸不为零
layout->addWidget(m_chartView, 1);
```

**设计决策**:

1. **setMinimumSize 作为基础保障**: 确保 QChartView 在任何布局计算阶段都有非零尺寸，避免 Qt Charts 内部跳过布局。
2. **showEvent 作为激活触发**: 在控件首次可见时强制重置轴范围，触发 QChart 的 plotArea 重新计算。
3. **不引入额外的 QTimer**: showEvent 已足够覆盖首次显示场景，避免异步延迟带来的时序不确定性。

---

### R2: 串口驱动检测 (P1)

#### 问题分析

嵌入式开发者首次使用串口工具时，常见问题:

- 插入 USB 转串口适配器（CH340、CP2102、FT232 等）后，设备管理器中显示黄色感叹号
- 串口工具中看不到任何端口
- 用户不清楚是硬件问题还是驱动问题

当前 EmbedDebug 的 `SerialConfigPanel::refreshPorts()` 仅通过 `QSerialPortInfo::availablePorts()` 列出可用端口，没有驱动层面的检测和提示。

#### 方案设计

##### 新增类: SerialDriverDetector

```cpp
// serial/SerialDriverDetector.h
#ifndef SERIALDRIVERDETECTOR_H
#define SERIALDRIVERDETECTOR_H

#include <QObject>
#include <QStringList>
#include <QMap>

// 串口驱动信息
struct DriverInfo {
    QString driverName;     // 驱动名称，如 "CH340", "CP2102"
    QString manufacturer;   // 厂商名称
    bool installed;         // 是否已安装
    QString infPath;        // INF路径（可选）
};

// 串口驱动检测器 -- 通过Windows SetupAPI检测已安装的串口驱动
// 在表现层(serial/)中，因为它直接服务于SerialConfigPanel的UI展示需求
class SerialDriverDetector : public QObject {
    Q_OBJECT

public:
    explicit SerialDriverDetector(QObject* parent = nullptr);

    // 检测系统中已安装的串口适配器驱动
    // 返回已检测到的驱动列表
    QList<DriverInfo> detectDrivers();

    // 获取所有已知驱动的安装状态
    QMap<QString, bool> knownDriverStatus();

    // 判断是否存在任何已安装的串口驱动
    bool hasAnyDriverInstalled();

    // 获取端口关联的驱动名称（通过设备描述匹配）
    QString driverForPort(const QString& portName);

private:
    // 通过WMI查询Win32_PnPEntity中的串口设备
    QList<DriverInfo> detectViaWMI();

    // 通过QSerialPortInfo的description/manufacturer字段匹配已知驱动
    QList<DriverInfo> detectViaPortInfo();

    // 已知驱动关键字映射（description关键字 -> DriverInfo）
    static const QMap<QString, DriverInfo> kKnownDrivers;
};

#endif // SERIALDRIVERDETECTOR_H
```

##### 检测策略

采用**双通道检测**，优先使用轻量级方案:

**通道1: QSerialPortInfo 字段匹配（主通道，跨平台可用）**

`QSerialPortInfo` 提供 `description()` 和 `manufacturer()` 字段，可以匹配已知驱动:

| description 关键字 | manufacturer 关键字 | 驱动标识 |
|-------------------|-------------------|---------|
| "CH340" / "CH341" | "wch" / "WCH" | CH340 |
| "CP210" | "Silicon Labs" / "Silabs" | CP210x |
| "FT232" / "FTDI" | "FTDI" | FT232 |
| "PL2303" | "Prolific" | PL2303 |

**通道2: WMI 查询 Win32_PnPEntity（增强通道，Windows专用）**

通过 WMI 查询 `Win32_PnPEntity WHERE PNPClass = 'Ports'`，获取更完整的设备驱动信息:

```cpp
// WMI查询伪代码（通过QProcess调用wmic或通过COM接口）
// wmic path Win32_PnPEntity where "PNPClass='Ports'" get Name,DeviceID,Manufacturer
```

**通道3（降级）: SetupAPI 枚举（需要Windows SDK头文件）**

如果WMI不可用，通过 `SetupDiGetClassDevs` + `SetupDiEnumDeviceInfo` 枚举 PORTS 类设备。此方案需要 `<windows.h>` 和 `<setupapi.h>`，增加平台依赖性，仅在必要时使用。

**推荐策略**: 优先使用通道1（QSerialPortInfo），无需平台特定代码。通道2作为增强信息源，检测是否有未关联到 COM 端口的驱动安装。通道3暂不实现。

##### SerialConfigPanel 集成

```cpp
// SerialConfigPanel 新增成员:
QLabel* m_driverInfoLabel;          // 驱动信息标签
SerialDriverDetector* m_driverDetector;  // 驱动检测器

// SerialConfigPanel::setupUI() 中，端口选择区域下方新增:
m_driverInfoLabel = new QLabel;
m_driverInfoLabel->setObjectName("driverInfoLabel");
m_driverInfoLabel->setWordWrap(true);
m_driverInfoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
portLayout->addWidget(m_driverInfoLabel);  // 或独立一行
```

```cpp
// SerialConfigPanel::refreshPorts() 中，刷新后检测驱动:
void SerialConfigPanel::refreshPorts()
{
    // ... 现有端口刷新逻辑 ...

    // 驱动检测
    auto drivers = m_driverDetector->detectDrivers();
    updateDriverInfo(drivers);
}

void SerialConfigPanel::updateDriverInfo(const QList<DriverInfo>& drivers)
{
    if (drivers.isEmpty()) {
        m_driverInfoLabel->setText(tr("未检测到串口驱动，请安装 CH340/CP2102/FT232 等驱动"));
        m_driverInfoLabel->setStyleSheet("color: #f38ba8;");  // error色
    } else {
        QStringList names;
        for (const auto& d : drivers) {
            names << d.driverName;
        }
        m_driverInfoLabel->setText(tr("已检测驱动: %1").arg(names.join(", ")));
        m_driverInfoLabel->setStyleSheet("color: #a6e3a1;");  // success色
    }
}
```

**端口下拉框增强**: 在 `refreshPorts()` 中，利用 `QSerialPortInfo::description()` 和驱动匹配结果，为每个端口项追加驱动标识:

```cpp
// 端口项格式: "COM3 - CH340 (USB-SERIAL CH340)"
QString driverName = m_driverDetector->driverForPort(port.portName());
QString displayText = name;
if (!desc.isEmpty()) {
    displayText += " - " + desc;
}
if (!driverName.isEmpty()) {
    displayText = name + " [" + driverName + "] - " + desc;
}
m_portCombo->addItem(displayText, name);
```

**设计决策**:

1. **SerialDriverDetector 放在 serial/ 目录**: 虽然驱动检测涉及系统底层能力，但它直接服务于 SerialConfigPanel 的 UI 展示需求，且当前仅用于表现层。如果未来需要在基础设施层复用（如自动安装驱动），再提取到 connection/ 或 utils/。
2. **优先使用 QSerialPortInfo**: 跨平台兼容，无额外依赖。Windows 专用 API（WMI/SetupAPI）作为增强信息源。
3. **颜色使用 inline stylesheet**: 驱动信息标签的颜色只有两种状态（成功/警告），使用内联样式比在三个 QSS 文件中各加一条规则更简洁。后续可迁移到 QSS 的 objectName 选择器。

---

### R3: SerialConfigPanel objectName审计与UI打磨 (P1)

#### 问题分析

当前 SerialConfigPanel::setupUI() 中，以下控件缺少 objectName:

| 控件 | 当前 objectName | 需要设置的 objectName |
|------|----------------|---------------------|
| m_portCombo | (无) | "portCombo" |
| m_refreshBtn | (无) | "portRefreshBtn" |
| m_baudCombo | (无) | "baudCombo" |
| m_dataBitsCombo | (无) | "dataBitsCombo" |
| m_parityCombo | (无) | "parityCombo" |
| m_stopBitsCombo | (无) | "stopBitsCombo" |
| m_flowControlCombo | (无) | "flowControlCombo" |
| 端口分组 QGroupBox | (无) | "portGroup" |
| 参数分组 QGroupBox | (无) | "paramGroup" |
| 控制信号分组 QGroupBox | (无) | "signalGroup" |

已有 objectName 的控件:
- m_connectBtn -> "connectBtn" (已有)
- m_dtrCheck -> "dtrCheck" (已有)
- m_rtsCheck -> "rtsCheck" (已有)

#### 方案设计

##### objectName 设置

在 setupUI() 中为所有子控件设置 objectName:

```cpp
m_portCombo->setObjectName("portCombo");
m_refreshBtn->setObjectName("portRefreshBtn");
m_baudCombo->setObjectName("baudCombo");
m_dataBitsCombo->setObjectName("dataBitsCombo");
m_parityCombo->setObjectName("parityCombo");
m_stopBitsCombo->setObjectName("stopBitsCombo");
m_flowControlCombo->setObjectName("flowControlCombo");
portGroup->setObjectName("portGroup");
paramGroup->setObjectName("paramGroup");
signalGroup->setObjectName("signalGroup");
```

##### UI间距调整

根据 CLAUDE.md 6.3 节标准:

| 元素 | 当前值 | 目标值 | 说明 |
|------|-------|-------|------|
| 面板内边距 | 12px | 12px | 符合 8-12px 标准，保持 |
| 控件间距（FormLayout） | 默认 | 显式设置 spacing=8 | 符合 6-8px 标准 |
| 分组间距 | 默认 | 显式设置 mainLayout spacing=12 | 符合 12-16px 标准 |
| 输入框高度 | 默认 | 28-32px | 通过 QSS min-height 或 setFixedHeight |
| 按钮高度 | connectBtn 36px | 保持 36px | 符合按钮最小高度 28px 标准 |
| 刷新按钮 | 默认 | 与端口下拉框等高 | 视觉对齐 |

```cpp
// setupUI() 间距调整:
mainLayout->setSpacing(12);           // 分组间距
formLayout->setSpacing(8);            // 表单行间距
formLayout->setContentsMargins(12, 16, 12, 8);  // 分组内边距
```

##### QSS 样式更新

在三个主题 QSS 文件中新增 SerialConfigPanel 专用样式:

```css
/* === SerialConfigPanel 串口配置面板 === */
QGroupBox#portGroup,
QGroupBox#paramGroup,
QGroupBox#signalGroup {
    color: #cdd6f4;
    border: 1px solid #313244;
    border-radius: 6px;
    margin-top: 12px;
    padding-top: 16px;
    font-weight: bold;
}

QPushButton#portRefreshBtn {
    background-color: transparent;
    color: #89b4fa;
    border: 1px solid #45475a;
    border-radius: 4px;
    padding: 4px 12px;
    font-size: 13px;
}
QPushButton#portRefreshBtn:hover { background-color: #313244; border-color: #89b4fa; }
QPushButton#portRefreshBtn:pressed { background-color: #45475a; }

QLabel#driverInfoLabel {
    font-size: 12px;
    padding: 2px 0px;
}

QComboBox#portCombo {
    min-height: 28px;
}
```

---

### R4: 方向前缀着色 (P1)

#### 问题分析

TerminalWidget::paintEvent() 中，当前渲染逻辑为:

```cpp
// 根据方向设置文字颜色（使用缓存的方向）
if (cached.direction == DataDirection::Tx) {
    painter.setPen(m_txColor);
} else {
    painter.setPen(m_rxColor);
}

// 绘制数据内容
painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.text);
```

`cached.text` 中已包含 `[TX:] ` 或 `[RX:] ` 前缀（由 formatToCache() 拼接），但整行使用统一颜色渲染。需求:

- `[TX:]` 使用绿色 `#a6e3a1` -- 与当前 m_txColor 一致
- `[RX:]` 使用蓝色 `#89b4fa` -- 与 Catppuccin Mocha 的 accent 色一致
- 数据内容保持原有的方向颜色

#### 方案设计

##### 方案A: 缓存中分离前缀和数据（推荐）

在 CachedLine 结构中新增前缀长度字段，paintEvent 中分段绘制:

```cpp
// TerminalWidget.h CachedLine 新增:
struct CachedLine {
    QString text;            // 按当前DisplayMode格式化后的文本
    DataDirection direction; // 收/发方向
    qint64 timestamp;        // epoch毫秒时间戳
    int prefixLength = 0;    // 方向前缀字符数（含尾部空格），0表示无前缀
};
```

```cpp
// formatToCache() 中记录前缀长度:
CachedLine TerminalWidget::formatToCache(const TerminalLine& line) const
{
    CachedLine cached;
    cached.direction = line.direction;
    cached.timestamp = line.timestamp.toMSecsSinceEpoch();

    QString prefix;
    if (m_showDirectionPrefix) {
        prefix = (line.direction == DataDirection::Tx) ? "[TX:] " : "[RX:] ";
    }
    cached.prefixLength = prefix.length();

    // ... 现有格式化逻辑，拼接到 cached.text ...
    return cached;
}
```

```cpp
// paintEvent() 中分段绘制:
// 方向前缀着色绘制
if (cached.prefixLength > 0) {
    QString prefixText = cached.text.left(cached.prefixLength);
    QColor prefixColor = (cached.direction == DataDirection::Tx)
        ? QColor(0xa6, 0xe3, 0xa1)    // TX前缀: 绿色 #a6e3a1
        : QColor(0x89, 0xb4, 0xfa);   // RX前缀: 蓝色 #89b4fa
    painter.setPen(prefixColor);
    painter.drawText(xOffset + 4, y + m_lineHeight - 4, prefixText);

    // 计算前缀绘制宽度，数据内容从偏移处开始
    int prefixWidth = fm.horizontalAdvance(prefixText);

    // 数据内容使用原有方向颜色
    if (cached.direction == DataDirection::Tx) {
        painter.setPen(m_txColor);
    } else {
        painter.setPen(m_rxColor);
    }
    painter.drawText(xOffset + 4 + prefixWidth, y + m_lineHeight - 4,
                     cached.text.mid(cached.prefixLength));
} else {
    // 无方向前缀时，保持原有渲染逻辑
    if (cached.direction == DataDirection::Tx) {
        painter.setPen(m_txColor);
    } else {
        painter.setPen(m_rxColor);
    }
    painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.text);
}
```

##### 配色方案

| 前缀 | 颜色 | Hex值 | 说明 |
|------|------|-------|------|
| `[TX:]` | 绿色 | `#a6e3a1` | Catppuccin Mocha green，与 m_txColor 一致 |
| `[RX:]` | 蓝色 | `#89b4fa` | Catppuccin Mocha blue，accent 色，与 m_rxColor 不同（m_rxColor 是 #cdd6f4 白色） |

**设计决策**:

1. **在 CachedLine 中记录 prefixLength 而非分离两个 QString**: 避免内存翻倍，一个 int 即可。paintEvent 中通过 mid() 切割，QPainter 的 drawText 只渲染指定范围。
2. **前缀颜色硬编码在 TerminalWidget 中**: 方向前缀的颜色是视觉设计的一部分（TX=发送=绿，RX=接收=蓝），与语义色板中的 success/accent 对应。后续可迁移到 ThemeManager 提供 `directionPrefixTxColor()` / `directionPrefixRxColor()` 方法。
3. **搜索高亮兼容**: 搜索高亮基于 `cached.text` 的完整文本计算偏移，前缀着色不影响搜索匹配逻辑。搜索高亮覆盖在前缀之上绘制，视觉上前缀和数据的高亮是一体的。

---

## 接口设计

### 新增接口

| 接口 | 文件 | 说明 |
|------|------|------|
| `ChartWidget::showEvent()` | chart/ChartWidget.h/cpp (修改) | 首次显示时强制触发QChart布局更新 |
| `SerialDriverDetector::detectDrivers()` | serial/SerialDriverDetector.h/cpp (新增) | 检测已安装的串口驱动 |
| `SerialDriverDetector::knownDriverStatus()` | serial/SerialDriverDetector.h/cpp (新增) | 获取所有已知驱动的安装状态 |
| `SerialDriverDetector::hasAnyDriverInstalled()` | serial/SerialDriverDetector.h/cpp (新增) | 判断是否存在任何已安装驱动 |
| `SerialDriverDetector::driverForPort()` | serial/SerialDriverDetector.h/cpp (新增) | 获取端口关联的驱动名称 |
| `SerialConfigPanel::updateDriverInfo()` | serial/SerialConfigPanel.h/cpp (修改) | 更新驱动信息标签显示 |

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| `CachedLine` 结构体 | 新增 `int prefixLength` 字段 | 内部数据结构变更，无外部影响 |
| `ChartWidget::setupUI()` | m_chartView 新增 setMinimumSize | 无外部影响，仅影响初始布局 |
| `TerminalWidget::formatToCache()` | 记录 prefixLength 到 CachedLine | 内部变更，无外部影响 |
| `TerminalWidget::paintEvent()` | 方向前缀分段着色渲染 | 视觉行为变更（预期改善），无功能影响 |
| `SerialConfigPanel::setupUI()` | 新增 objectName + 间距调整 + 驱动信息标签 | UI变更，无功能影响 |
| `SerialConfigPanel::refreshPorts()` | 端口项展示驱动标识 + 调用驱动检测 | 行为增强，不影响端口选择逻辑 |

### 移除接口

无。所有现有接口保留。

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| `QSerialPortInfo` | Qt SerialPort 模块 | 端口枚举和设备描述获取 | R2 |
| `QChartView` / `QChart` | Qt Charts 模块 | 波形图渲染 | R1 |
| `ThemeManager` | core/ThemeManager.h/cpp | 方向前缀配色获取（未来迁移） | R4 |
| `TerminalModel` | terminal/TerminalModel.h | 数据行访问 | R4 |
| `CachedLine` | terminal/TerminalWidget.h | 缓存行结构 | R4 |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **策略模式 (Strategy)** | 驱动检测的双通道策略（QSerialPortInfo 优先，WMI 增强） | R2 | 优先使用轻量级策略，仅在信息不足时降级到重量级策略 |
| **观察者模式 (Observer)** | 驱动信息变更通知 SerialConfigPanel 更新显示 | R2 | Qt 信号/槽机制，驱动检测完成后触发 UI 更新 |

---

## 影响范围

### 文件变更矩阵

| 文件 | 变更类型 | R1 | R2 | R3 | R4 |
|------|---------|-----|-----|-----|-----|
| `src/chart/ChartWidget.h` | 修改 | +3 行 | -- | -- | -- |
| `src/chart/ChartWidget.cpp` | 修改 | +15 行 | -- | -- | -- |
| `src/serial/SerialDriverDetector.h` | **新增** | -- | +60 行 | -- | -- |
| `src/serial/SerialDriverDetector.cpp` | **新增** | -- | +120 行 | -- | -- |
| `src/serial/SerialConfigPanel.h` | 修改 | -- | +8 行 | -- | -- |
| `src/serial/SerialConfigPanel.cpp` | 修改 | -- | +50 行 | +15 行 | -- |
| `src/terminal/TerminalWidget.h` | 修改 | -- | -- | -- | +2 行 |
| `src/terminal/TerminalWidget.cpp` | 修改 | -- | -- | -- | +25 行 |
| `resources/themes/dark_terminal.qss` | 修改 | -- | -- | +20 行 | -- |
| `resources/themes/modern_dark.qss` | 修改 | -- | -- | +20 行 | -- |
| `resources/themes/light.qss` | 修改 | -- | -- | +20 行 | -- |
| `CMakeLists.txt` | 修改 | -- | +2 行 | -- | -- |

### 预计变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| ChartWidget.h/cpp | 18 行 | 2 行 | -- |
| SerialDriverDetector.h/cpp | 180 行 | -- | -- |
| SerialConfigPanel.h/cpp | 73 行 | 10 行 | -- |
| TerminalWidget.h/cpp | 27 行 | 15 行 | -- |
| QSS主题文件 (x3) | 60 行 | -- | -- |
| CMakeLists.txt | 2 行 | -- | -- |
| **合计** | **约 360 行** | **约 27 行** | **约 0 行** |

### 跨模块影响评估

- **ChartWidget 独立修复**: R1 的 ChartWidget 渲染Bug修复完全自包含，不影响其他模块。
- **SerialDriverDetector 是纯新增模块**: R2 新增的驱动检测器仅被 SerialConfigPanel 调用，不修改现有类的接口。
- **TerminalWidget 变更向后兼容**: R4 在 CachedLine 中新增 prefixLength 字段（默认值 0），不影响现有的缓存逻辑。m_showDirectionPrefix 为 false 时，prefixLength 为 0，paintEvent 走原有渲染路径。
- **QSS 变更仅影响样式**: R3 的 objectName 和 QSS 样式变更不影响任何功能逻辑。

---

## 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | 首次进入波形图面板时图表正常渲染 | 手动: 切换到波形图面板 | 看到完整的坐标系（X轴Samples、Y轴Value）、网格线、工具栏，无需先点击Clear |
| R1-AC2 | 无数据时图表显示空白坐标系 | 手动: 启动应用后立即切换到波形图 | 坐标轴可见，plotArea 有背景色，不是空白矩形 |
| R1-AC3 | 有数据时图表正常显示曲线 | 手动: 连接串口接收帧数据后切换到波形图 | 曲线正常渲染 |
| R1-AC4 | 点击Clear后图表正常 | 手动: 有数据时点击Clear | 坐标系和工具栏正常显示，数据清除 |
| R1-AC5 | 窗口resize时图表跟随调整 | 手动: 拖拽窗口边框改变大小 | 图表区域平滑调整，无错乱 |
| R2-AC1 | 启动时自动检测已安装驱动 | 手动: 启动应用（系统已安装CH340驱动） | SerialConfigPanel中显示"已检测驱动: CH340"绿色标签 |
| R2-AC2 | 端口下拉框展示驱动标识 | 手动: 插入CH340设备后刷新端口 | 端口项显示 "COM3 [CH340] - USB-SERIAL CH340" 格式 |
| R2-AC3 | 无驱动时显示警告 | 手动: 在无串口驱动的系统上启动 | 显示红色警告"未检测到串口驱动，请安装..." |
| R2-AC4 | 刷新端口时更新驱动信息 | 手动: 插拔USB设备后点击刷新 | 驱动信息标签更新 |
| R2-AC5 | 无串口设备但有驱动时不误报 | 手动: 系统已安装驱动但未插入设备 | 不显示红色警告（驱动已安装，仅无设备） |
| R3-AC1 | 所有SerialConfigPanel子控件有objectName | 代码检查: grep setObjectName | m_portCombo, m_refreshBtn, m_baudCombo等10个控件均有objectName |
| R3-AC2 | QSS中新增SerialConfigPanel专用样式 | 代码检查: 三个QSS文件中均有 | QGroupBox#portGroup、QPushButton#portRefreshBtn等样式规则存在 |
| R3-AC3 | 面板间距符合CLAUDE.md 6.3标准 | 手动: 检查面板视觉 | 分组间距12px，表单行间距8px，视觉层次清晰 |
| R3-AC4 | 端口刷新按钮hover/pressed状态 | 手动: 鼠标悬浮和点击刷新按钮 | 有视觉反馈（背景色变化） |
| R4-AC1 | [TX:]前缀显示为绿色 | 手动: 开启方向前缀 -> 发送数据 | [TX:] 文字为绿色 #a6e3a1 |
| R4-AC2 | [RX:]前缀显示为蓝色 | 手动: 开启方向前缀 -> 接收数据 | [RX:] 文字为蓝色 #89b4fa |
| R4-AC3 | 关闭方向前缀时无视觉变化 | 手动: 关闭方向前缀 -> 收发数据 | 渲染与原有行为一致（无前缀分段绘制） |
| R4-AC4 | 搜索高亮兼容前缀着色 | 手动: 开启方向前缀 -> 搜索匹配关键词 | 搜索高亮正常显示，不被前缀着色干扰 |
| R4-AC5 | HEX模式方向前缀着色正常 | 手动: 切换HEX显示 -> 开启方向前缀 | [TX:]绿色，HEX数据为发送色 |
| AC-1 | 编译零错误零警告 | `cmake --build build` | 0 error, 0 warning |
| AC-2 | EmbedDebug.bat正常启动 | 双击 EmbedDebug.bat | 应用窗口正常显示 |
| AC-3 | 现有功能回归: 波形图数据收发 | 手动: 连接串口 -> 接收帧数据 -> 波形图 | 数据正常显示，通道正确 |
| AC-4 | 现有功能回归: 终端收发 | 手动: 连接串口 -> 收发数据 | 终端正常显示 |
| AC-5 | 现有功能回归: 搜索高亮 | 手动: Ctrl+F -> 搜索关键词 | 高亮和F3导航正常 |
| AC-6 | 现有功能回归: OTA传输 | 手动: XMODEM传输 | 功能正常 |
| AC-7 | 三个主题QSS完整 | 手动: 切换暗色/现代暗色/亮色主题 | 所有新增控件样式正确 |

---

## 实施优先级

| 顺序 | 步骤 | 理由 |
|------|------|------|
| 1 | ChartWidget showEvent修复 (R1) | P0 Bug修复，最小改动量，独立验证 |
| 2 | CachedLine.prefixLength + formatToCache修改 (R4) | 内部数据结构变更，需先完成才能修改paintEvent |
| 3 | TerminalWidget paintEvent前缀着色 (R4) | 依赖步骤2的prefixLength字段 |
| 4 | SerialDriverDetector 新增类 (R2) | 纯新增，独立开发 |
| 5 | SerialConfigPanel objectName审计 + 间距调整 (R3) | UI层改动，可与R2并行 |
| 6 | SerialConfigPanel 驱动检测集成 (R2) | 依赖步骤4和步骤5 |
| 7 | QSS主题文件更新 (R3) | 依赖步骤5的objectName |
| 8 | CMakeLists.txt 更新 | 新增 SerialDriverDetector 文件 |
| 9 | 编译验证 | 确保零错误 |
| 10 | 手动功能测试 | 验证所有验收标准 |

---

## 验证度量指标

### 代码度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| ChartWidget.h 新增行数 | diff | 102 行 | < 110 行 |
| ChartWidget.cpp 新增行数 | diff | 271 行 | < 290 行 |
| SerialDriverDetector.h 行数 | wc -l | 0 (新增) | < 70 行 |
| SerialDriverDetector.cpp 行数 | wc -l | 0 (新增) | < 150 行 |
| SerialConfigPanel.cpp 行数 | diff | 232 行 | < 290 行 |
| TerminalWidget.cpp 行数 | diff | 514 行 | < 540 行 |

### 架构度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| SerialDriverDetector 依赖方向 | 代码检查 | 仅依赖 Qt 模块（QSerialPortInfo），不依赖项目内部类 |
| CachedLine 新增字段默认值 | 代码检查 | prefixLength=0，向后兼容 |
| QSS 文件同步率 | 代码检查 | 三个主题文件均包含新增样式规则 |
| objectName 覆盖率 | 代码检查 | SerialConfigPanel 所有子控件100%设置objectName |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| ChartWidget首次显示渲染 | 手动测试: 切换到波形图面板 | 首次即渲染完整坐标系 |
| 驱动检测准确率 | 手动测试: 插入CH340/CP2102设备 | 正确识别驱动名称 |
| 前缀着色视觉准确性 | 手动测试: 截图取色对比 | TX=#a6e3a1, RX=#89b4fa |
| 编译零错误零警告 | cmake --build | 0 error, 0 warning |
| EmbedDebug.bat启动 | 双击bat | 正常启动 |
