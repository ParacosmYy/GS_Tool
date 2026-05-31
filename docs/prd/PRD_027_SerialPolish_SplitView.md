# PRD-027: 终端/串口代码打磨与MainWindow瘦身拆分

## 背景

commit #26 已完成 ChartModel 降采样Bug修复、SerialDriverDetector 驱动检测UI集成和 objectName/QSS 审计补全，当前评分 28 分。

本次迭代 #27 属于代码质量打磨和架构瘦身迭代，聚焦五个问题:

1. **TerminalModel::lineAt() 悬空引用风险 (P0)** -- 经审查发现头文件声明已修正为按值返回，但需确认所有调用点已适配且无残留引用语义假设。TerminalWidget 第414行仍直接调用 `m_model->lineAt(i).data`，该路径在 HEX 搜索模式下无缓存保护，需验证安全性并补充注释。

2. **SerialConfigPanel::m_driverInfoLbl 内联样式违规 (P1)** -- 第108行 `m_driverInfoLbl->setStyleSheet("font-size: 11px; padding: 4px;")` 直接在C++代码中硬编码样式，违反 CLAUDE.md 6.8 节铁律第1条。需将颜色和排版属性迁移到 QSS 主题文件，通过 objectName 选择器驱动。

3. **MainWindow 多个控件缺少 objectName (P1)** -- `themeCombo`、`langCombo`、`themeLabel`、`langLabel`、`rightWidget`、`rightPanel`、`serialPanel`、`terminalContainer` 共 8 个局部变量 widget 未设置 objectName，QSS 无法精确选中，违反 CLAUDE.md 6.8 节铁律第2条。

4. **MainWindow.cpp 行数超标 (P0)** -- 当前 630 行，超过 CLAUDE.md 4.6 节规定的 500 行上限。需提取 `SettingsController`（loadSettings/saveSettings/onThemeChanged/onLanguageChanged）和 `ToolbarController`（setupToolbar + toolbar信号连接），将 MainWindow 瘦身到 500 行以下。

5. **工具栏ComboBox QSS 缺失 (P2)** -- `themeCombo` 和 `langCombo` 目前只有全局 QComboBox 样式，缺少 objectName 级别的工具栏专用样式（紧凑宽度、工具栏内嵌的视觉风格）。

**审查基准**: commit #26, score 28。

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | TerminalModel::lineAt() 安全审计 -- 确认按值返回已落地，补充所有调用点注释，验证无悬空引用路径 | P0 | terminal/TerminalModel.h/cpp, terminal/TerminalWidget.cpp |
| R2 | SerialConfigPanel::m_driverInfoLbl 内联样式迁移 -- 删除 setStyleSheet，属性迁移到 QSS 三个主题文件 | P1 | serial/SerialConfigPanel.cpp, resources/themes/*.qss |
| R3 | MainWindow objectName 补全 -- 8个缺少 objectName 的控件逐一设置 | P1 | core/MainWindow.cpp |
| R4 | MainWindow.cpp 瘦身拆分 -- 提取 SettingsController + ToolbarController，行数从 630 降到 500 以内 | P0 | core/MainWindow.h/cpp, core/SettingsController.h/cpp(新增), core/ToolbarController.h/cpp(新增) |
| R5 | 工具栏 ComboBox QSS 补全 -- themeCombo/langCombo 专用 objectName 样式 | P2 | resources/themes/*.qss |

---

## 需求详细说明

---

### R1: TerminalModel::lineAt() 安全审计 (P0)

#### 问题分析

`TerminalModel::lineAt()` 原设计返回 `const TerminalLine&`，但内部 `QMutexLocker` 在函数返回时释放，调用者拿到的引用指向环形缓冲区 `m_buffer[physicalIndex(index)]`。如果另一个线程在调用者使用引用之前触发了 `appendLine()`（覆盖头指针位置数据），引用即悬空，导致 UAF（Use-After-Free）。

当前代码状态:
- `TerminalModel.h` 第36行: 已声明为 `TerminalLine lineAt(int index) const` (按值返回)
- `TerminalModel.cpp` 第64行: 实现为 `TerminalLine TerminalModel::lineAt(int index) const` (按值返回)
- 头文件注释已更新为 "返回值而非引用，避免QMutexLocker释放后引用悬空"

**问题已修复**，但需完成以下审计工作:

1. TerminalWidget 中有两处调用 `lineAt()`:
   - 第169行: `m_cachedLines[i] = formatToCache(m_model->lineAt(i))` -- 在 `paintEvent` 上下文中，按值返回后立即传入 `formatToCache()`，安全。
   - 第414行: `m_model ? m_model->lineAt(i).data : QByteArray()` -- 在 HEX 搜索模式下，直接访问 `lineAt(i).data` 取原始字节。按值返回后 `.data` 是拷贝的 QByteArray 的 data() 指针，临时对象在语句结束时销毁。但由于 `HexConverter::toHexString()` 接受 `const QByteArray&`，临时对象的生命周期被 const 引用延长到完整表达式结束，因此安全。

2. 需在第414行补充安全注释，说明按值返回的临时对象生命周期规则。

#### 方案设计

无代码逻辑变更。仅补充注释:

```cpp
// TerminalWidget.cpp 第414行补充注释:
// lineAt() 按值返回 TerminalLine，临时对象生命周期被 toHexString(const QByteArray&)
// 的 const& 参数延长至完整表达式结束，无悬空风险
QString hexText = HexConverter::toHexString(
    m_model ? m_model->lineAt(i).data : QByteArray());
```

---

### R2: SerialConfigPanel::m_driverInfoLbl 内联样式迁移 (P1)

#### 问题分析

`SerialConfigPanel.cpp` 第108行:

```cpp
m_driverInfoLbl->setStyleSheet("font-size: 11px; padding: 4px;");
```

违反 CLAUDE.md 6.8 节铁律第1条: "禁止在C++代码中硬编码颜色值到setStyleSheet()"。虽然此处只设置了 font-size 和 padding，未设置颜色，但 inline stylesheet 会覆盖 QSS 主题文件中的同名属性，导致主题切换时标签样式不跟随变化。

#### 方案设计

**步骤 1**: 删除 `SerialConfigPanel.cpp` 第108行的 `setStyleSheet` 调用。

```cpp
// 修改前:
m_driverInfoLbl->setObjectName("driverInfoLbl");
m_driverInfoLbl->setWordWrap(true);
m_driverInfoLbl->setStyleSheet("font-size: 11px; padding: 4px;");

// 修改后:
m_driverInfoLbl->setObjectName("driverInfoLbl");
m_driverInfoLbl->setWordWrap(true);
```

**步骤 2**: 在三个主题 QSS 文件中新增 `QLabel#driverInfoLbl` 样式。

```css
/* dark_terminal.qss */
QLabel#driverInfoLbl {
    color: #a6adc8;
    font-size: 11px;
    padding: 4px;
}

/* modern_dark.qss */
QLabel#driverInfoLbl {
    color: #565f89;
    font-size: 11px;
    padding: 4px;
}

/* light.qss */
QLabel#driverInfoLbl {
    color: #6b7280;
    font-size: 11px;
    padding: 4px;
}
```

**objectName**: 已有 `driverInfoLbl`（第106行），无需新增。

---

### R3: MainWindow objectName 补全 (P1)

#### 问题分析

`MainWindow.cpp` 中以下控件未设置 objectName:

| 控件 | 类型 | 声明位置 | 用途 |
|------|------|---------|------|
| `themeCombo` | QComboBox* | 第220行 | 主题切换下拉框 |
| `langCombo` | QComboBox* | 第241行 | 语言切换下拉框 |
| `themeLabel` | QLabel* | 第217行 | "主题:" 标签 |
| `langLabel` | QLabel* | 第237行 | "语言:" 标签 |
| `rightWidget` | QWidget* | 第92行 | 右侧容器 |
| `rightPanel` | QWidget* | 第97行 | 右侧面板 |
| `serialPanel` | QWidget* | 第104行 | 串口面板 |
| `terminalContainer` | QWidget* | 第140行 | 终端容器 |

违反 CLAUDE.md 6.8 节铁律第2条: "所有 QWidget 必须设置 objectName"。

#### 方案设计

逐一在控件创建后添加 `setObjectName`:

```cpp
// setupUI() 中:
auto* rightWidget = new QWidget;
rightWidget->setObjectName("rightWidget");

m_rightPanel = new QWidget;
m_rightPanel->setObjectName("rightPanel");

auto* serialPanel = new QWidget;
serialPanel->setObjectName("serialPanel");

auto* terminalContainer = new QWidget;
terminalContainer->setObjectName("terminalContainer");

// setupToolbar() 中:
auto* themeLabel = new QLabel(tr(" 主题: "));
themeLabel->setObjectName("themeLabel");

m_themeCombo = new QComboBox;
m_themeCombo->setObjectName("themeCombo");

auto* langLabel = new QLabel(tr(" 语言: "));
langLabel->setObjectName("langLabel");

m_langCombo = new QComboBox;
m_langCombo->setObjectName("langCombo");
```

---

### R4: MainWindow.cpp 瘦身拆分 (P0)

#### 问题分析

MainWindow.cpp 当前 630 行，超过 CLAUDE.md 4.6 节规定的 500 行上限。主要膨胀来源:

| 方法 | 行数 | 职责 |
|------|------|------|
| `setupToolbar()` | 64行 (181-245) | 工具栏UI创建 + 控件初始化 |
| `loadSettings()` | 37行 (445-481) | 窗口几何/主题/串口配置/语言恢复 |
| `saveSettings()` | 23行 (483-506) | 窗口几何/主题/串口配置持久化 |
| `onThemeChanged()` | 8行 (569-575) | 主题切换响应 |
| `onLanguageChanged()` | 12行 (577-588) | 语言切换响应 |
| toolbar信号连接 (connectSignals内) | 6行 (376-381) | themeCombo/langCombo 信号 |

合计约 150 行可提取。

#### 方案设计

##### 新增类 1: SettingsController

**职责**: 管理应用设置的加载、保存、主题切换、语言切换。

```
文件: src/core/SettingsController.h / src/core/SettingsController.cpp
层级: 业务层 (QObject)
```

```cpp
// SettingsController.h
class SettingsController : public QObject {
    Q_OBJECT

public:
    explicit SettingsController(QObject* parent = nullptr);

    // 设置依赖的UI组件指针（由MainWindow在setupUI后注入）
    void setThemeCombo(QComboBox* combo);
    void setLangCombo(QComboBox* combo);
    void setSerialConfig(SerialConfigPanel* panel);
    void setMainWindow(QWidget* window);

    // 加载/保存
    void loadSettings();
    void saveSettings();

public slots:
    void onThemeChanged(int index);
    void onLanguageChanged(int index);

private:
    QComboBox* m_themeCombo = nullptr;
    QComboBox* m_langCombo = nullptr;
    SerialConfigPanel* m_serialConfig = nullptr;
    QPointer<QWidget> m_mainWindow;
};
```

**提取的方法**:
- `MainWindow::loadSettings()` -> `SettingsController::loadSettings()`
- `MainWindow::saveSettings()` -> `SettingsController::saveSettings()`
- `MainWindow::onThemeChanged(int)` -> `SettingsController::onThemeChanged(int)`
- `MainWindow::onLanguageChanged(int)` -> `SettingsController::onLanguageChanged(int)`

**预计行数**: SettingsController.h 约 40 行，SettingsController.cpp 约 100 行。

##### 新增类 2: ToolbarController

**职责**: 管理工具栏的创建、控件布局和信号连接。

```
文件: src/core/ToolbarController.h / src/core/ToolbarController.cpp
层级: 表现层 (QObject)
```

```cpp
// ToolbarController.h
class ToolbarController : public QObject {
    Q_OBJECT

public:
    explicit ToolbarController(QObject* parent = nullptr);

    // 创建工具栏UI并添加到MainWindow
    void setupToolbar(QMainWindow* mainWindow);

    // 获取工具栏控件指针（供外部查询）
    QComboBox* displayModeCombo() const;
    QAction* timestampAction() const;
    QAction* dirPrefixAction() const;
    QAction* clearAction() const;
    QAction* exportAction() const;
    QComboBox* themeCombo() const;
    QComboBox* langCombo() const;

signals:
    // 代理信号 -- 将工具栏控件事件转发给外部
    void displayModeChanged(int index);
    void timestampToggled(bool checked);
    void dirPrefixToggled(bool checked);
    void clearRequested();
    void exportRequested();
    void themeChanged(int index);
    void languageChanged(int index);

private:
    QComboBox* m_displayModeCombo = nullptr;
    QAction* m_timestampAction = nullptr;
    QAction* m_dirPrefixAction = nullptr;
    QAction* m_clearAction = nullptr;
    QAction* m_exportAction = nullptr;
    QComboBox* m_themeCombo = nullptr;
    QComboBox* m_langCombo = nullptr;
};
```

**提取的方法**:
- `MainWindow::setupToolbar()` 全部 -> `ToolbarController::setupToolbar(QMainWindow*)`
- toolbar 相关的信号连接（connectSignals 中第352-381行）-> ToolbarController 内部连接，通过代理信号对外暴露

**预计行数**: ToolbarController.h 约 50 行，ToolbarController.cpp 约 130 行。

##### MainWindow 适配

MainWindow 变更:

1. 新增两个成员: `SettingsController* m_settingsController` 和 `ToolbarController* m_toolbarController`
2. 构造函数中初始化两个控制器
3. 删除 `setupToolbar()` 方法体，改为调用 `m_toolbarController->setupToolbar(this)`
4. 删除 `loadSettings()`/`saveSettings()`/`onThemeChanged()`/`onLanguageChanged()` 方法体，改为委托给 `m_settingsController`
5. `connectSignals()` 中的 toolbar 信号改为连接 `m_toolbarController` 的代理信号
6. `closeEvent()` 中 `saveSettings()` 改为 `m_settingsController->saveSettings()`

**预计 MainWindow.cpp 行数**: 630 - 150(提取) + 40(适配) = 约 520 行，再通过精简 connectSignals 中的重复代码达到 500 行以内。

---

### R5: 工具栏 ComboBox QSS 补全 (P2)

#### 问题分析

`themeCombo` 和 `langCombo` 嵌在 QToolBar 中，目前只有全局 `QComboBox` 样式。工具栏中的 ComboBox 应该有更紧凑的视觉效果: 较小的内边距、与工具栏背景融合的边框色、下拉箭头与工具栏风格一致。

`displayModeCombo` 已有专用 QSS（通过 `QComboBox#displayModeCombo`），但 themeCombo/langCombo 缺少。

#### 方案设计

在三个主题 QSS 文件中新增:

```css
/* dark_terminal.qss */
QComboBox#themeCombo, QComboBox#langCombo {
    background-color: #313244;
    border: 1px solid #45475a;
    border-radius: 4px;
    padding: 3px 8px;
    color: #cdd6f4;
    font-size: 12px;
}
QComboBox#themeCombo:hover, QComboBox#langCombo:hover {
    border-color: #89b4fa;
}
QComboBox#themeCombo::drop-down, QComboBox#langCombo::drop-down {
    border: none;
}

/* modern_dark.qss */
QComboBox#themeCombo, QComboBox#langCombo {
    background-color: #292e42;
    border: 1px solid #3b4261;
    border-radius: 4px;
    padding: 3px 8px;
    color: #c0caf5;
    font-size: 12px;
}
QComboBox#themeCombo:hover, QComboBox#langCombo:hover {
    border-color: #7aa2f7;
}
QComboBox#themeCombo::drop-down, QComboBox#langCombo::drop-down {
    border: none;
}

/* light.qss */
QComboBox#themeCombo, QComboBox#langCombo {
    background-color: #ffffff;
    border: 1px solid #d1d5db;
    border-radius: 4px;
    padding: 3px 8px;
    color: #111827;
    font-size: 12px;
}
QComboBox#themeCombo:hover, QComboBox#langCombo:hover {
    border-color: #3b82f6;
}
QComboBox#themeCombo::drop-down, QComboBox#langCombo::drop-down {
    border: none;
}
```

---

## 接口设计

### 新增接口

| 接口 | 文件 | 说明 |
|------|------|------|
| `SettingsController::SettingsController()` | core/SettingsController.h/cpp | 构造函数 |
| `SettingsController::setThemeCombo()` | core/SettingsController.h/cpp | 注入主题下拉框指针 |
| `SettingsController::setLangCombo()` | core/SettingsController.h/cpp | 注入语言下拉框指针 |
| `SettingsController::setSerialConfig()` | core/SettingsController.h/cpp | 注入串口配置面板指针 |
| `SettingsController::setMainWindow()` | core/SettingsController.h/cpp | 注入主窗口指针 |
| `SettingsController::loadSettings()` | core/SettingsController.h/cpp | 加载所有持久化设置 |
| `SettingsController::saveSettings()` | core/SettingsController.h/cpp | 保存所有设置到磁盘 |
| `SettingsController::onThemeChanged(int)` | core/SettingsController.h/cpp | 主题切换槽函数 |
| `SettingsController::onLanguageChanged(int)` | core/SettingsController.h/cpp | 语言切换槽函数 |
| `ToolbarController::ToolbarController()` | core/ToolbarController.h/cpp | 构造函数 |
| `ToolbarController::setupToolbar()` | core/ToolbarController.h/cpp | 创建工具栏UI |
| `ToolbarController::displayModeCombo()` | core/ToolbarController.h/cpp | 获取显示模式下拉框 |
| `ToolbarController::themeCombo()` | core/ToolbarController.h/cpp | 获取主题下拉框 |
| `ToolbarController::langCombo()` | core/ToolbarController.h/cpp | 获取语言下拉框 |
| `ToolbarController::displayModeChanged` | core/ToolbarController.h | 代理信号 |
| `ToolbarController::timestampToggled` | core/ToolbarController.h | 代理信号 |
| `ToolbarController::dirPrefixToggled` | core/ToolbarController.h | 代理信号 |
| `ToolbarController::clearRequested` | core/ToolbarController.h | 代理信号 |
| `ToolbarController::exportRequested` | core/ToolbarController.h | 代理信号 |
| `ToolbarController::themeChanged` | core/ToolbarController.h | 代理信号 |
| `ToolbarController::languageChanged` | core/ToolbarController.h | 代理信号 |

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| `MainWindow::setupToolbar()` | 委托给 ToolbarController | 外部行为不变 |
| `MainWindow::loadSettings()` | 委托给 SettingsController | 外部行为不变 |
| `MainWindow::saveSettings()` | 委托给 SettingsController | 外部行为不变 |
| `MainWindow::onThemeChanged(int)` | 委托给 SettingsController | 信号签名不变 |
| `MainWindow::onLanguageChanged(int)` | 委托给 SettingsController | 信号签名不变 |
| `MainWindow::connectSignals()` | toolbar部分改为连接代理信号 | 行为不变 |
| `MainWindow::closeEvent()` | saveSettings委托调用 | 行为不变 |

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| `SettingsManager` | utils/SettingsManager.h/cpp | SettingsController 的底层持久化引擎 | R4 |
| `ThemeManager` | core/ThemeManager.h/cpp | SettingsController 的主题加载引擎 | R4 |
| `SerialConfigPanel` | serial/SerialConfigPanel.h | SettingsController 恢复串口配置 | R4 |
| `TerminalModel` | terminal/TerminalModel.h | lineAt() 安全审计 | R1 |
| `HexConverter` | utils/HexConverter.h | HEX搜索路径的lineAt使用 | R1 |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **委托模式 (Delegation)** | MainWindow 将设置和工具栏职责委托给专门控制器 | R4 | MainWindow 不再直接处理，通过控制器间接操作 |
| **中介者模式 (Mediator)** | ToolbarController 作为工具栏UI和MainWindow之间的中介 | R4 | 工具栏控件的信号通过代理信号转发，MainWindow不需要知道工具栏内部控件 |
| **依赖注入 (DI)** | SettingsController 通过 setter 接收UI组件指针 | R4 | 控制器不创建UI，由MainWindow注入已创建的控件 |

---

## 影响范围

### 文件变更矩阵

| 文件 | 变更类型 | R1 | R2 | R3 | R4 | R5 |
|------|---------|-----|-----|-----|-----|-----|
| `src/terminal/TerminalModel.h` | 无变更(已修复) | -- | -- | -- | -- | -- |
| `src/terminal/TerminalModel.cpp` | 无变更(已修复) | -- | -- | -- | -- | -- |
| `src/terminal/TerminalWidget.cpp` | 注释补充 | +2行 | -- | -- | -- | -- |
| `src/serial/SerialConfigPanel.cpp` | 删除inline样式 | -- | -1行 | -- | -- | -- |
| `src/core/MainWindow.h` | 新增成员+删除方法 | -- | -- | -- | +5行/-4行 | -- |
| `src/core/MainWindow.cpp` | objectName补全+委托改造 | -- | -- | +8行 | -150行/+40行 | -- |
| `src/core/SettingsController.h` | 新增 | -- | -- | -- | +40行 | -- |
| `src/core/SettingsController.cpp` | 新增 | -- | -- | -- | +100行 | -- |
| `src/core/ToolbarController.h` | 新增 | -- | -- | -- | +50行 | -- |
| `src/core/ToolbarController.cpp` | 新增 | -- | -- | -- | +130行 | -- |
| `resources/themes/dark_terminal.qss` | 追加样式 | -- | +5行 | -- | -- | +9行 |
| `resources/themes/modern_dark.qss` | 追加样式 | -- | +5行 | -- | -- | +9行 |
| `resources/themes/light.qss` | 追加样式 | -- | +5行 | -- | -- | +9行 |

### 预计变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| TerminalWidget.cpp | 2 行 | -- | -- |
| SerialConfigPanel.cpp | -- | -- | 1 行 |
| MainWindow.h | 5 行 | 4 行 | -- |
| MainWindow.cpp | 48 行 | 20 行 | 150 行 |
| SettingsController.h | 40 行 | -- | -- |
| SettingsController.cpp | 100 行 | -- | -- |
| ToolbarController.h | 50 行 | -- | -- |
| ToolbarController.cpp | 130 行 | -- | -- |
| dark_terminal.qss | 14 行 | -- | -- |
| modern_dark.qss | 14 行 | -- | -- |
| light.qss | 14 行 | -- | -- |
| **合计** | **约 417 行** | **约 24 行** | **约 151 行** |

### 跨模块影响评估

- **MainWindow -> SettingsController**: MainWindow 不再持有 SettingsManager/ThemeManager 的直接调用，全部通过 SettingsController 中介。对 SerialConfigPanel 的 restoreConfig 调用也移入 SettingsController。
- **MainWindow -> ToolbarController**: MainWindow 不再直接创建工具栏控件，通过 ToolbarController 的 getter 获取控件指针用于信号连接。connectSignals 中 toolbar 部分改为连接 ToolbarController 的代理信号。
- **QSS 主题文件**: 三个主题文件各新增 `QLabel#driverInfoLbl` 和 `QComboBox#themeCombo`/`QComboBox#langCombo` 样式块，不影响现有样式。
- **TerminalModel**: 无变更。R1 为纯注释审计，确认按值返回已安全落地。

---

## 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | lineAt() 头文件声明为按值返回 | 代码检查: TerminalModel.h 第36行 | `TerminalLine lineAt(int index) const` |
| R1-AC2 | lineAt() 实现为按值返回 | 代码检查: TerminalModel.cpp 第64行 | `TerminalLine TerminalModel::lineAt(...)` |
| R1-AC3 | TerminalWidget 第414行有安全注释 | 代码检查 | 注释说明临时对象生命周期延长 |
| R2-AC1 | m_driverInfoLbl 无 setStyleSheet 调用 | 代码检查: SerialConfigPanel.cpp | 不存在 setStyleSheet 字符串 |
| R2-AC2 | dark_terminal.qss 包含 QLabel#driverInfoLbl | 代码检查 | 存在完整样式块(color/font-size/padding) |
| R2-AC3 | modern_dark.qss 包含 QLabel#driverInfoLbl | 代码检查 | 存在完整样式块 |
| R2-AC4 | light.qss 包含 QLabel#driverInfoLbl | 代码检查 | 存在完整样式块 |
| R2-AC5 | 驱动信息标签文字在暗色/亮色主题下均清晰可读 | 手动: 切换主题查看标签 | 颜色对比度足够 |
| R3-AC1 | rightWidget 有 objectName | 代码检查: setupUI() | `setObjectName("rightWidget")` |
| R3-AC2 | rightPanel 有 objectName | 代码检查 | `setObjectName("rightPanel")` |
| R3-AC3 | serialPanel 有 objectName | 代码检查 | `setObjectName("serialPanel")` |
| R3-AC4 | terminalContainer 有 objectName | 代码检查 | `setObjectName("terminalContainer")` |
| R3-AC5 | themeCombo 有 objectName | 代码检查 | `setObjectName("themeCombo")` |
| R3-AC6 | langCombo 有 objectName | 代码检查 | `setObjectName("langCombo")` |
| R3-AC7 | themeLabel 有 objectName | 代码检查 | `setObjectName("themeLabel")` |
| R3-AC8 | langLabel 有 objectName | 代码检查 | `setObjectName("langLabel")` |
| R4-AC1 | SettingsController.h/cpp 文件存在 | 文件检查 | 存在且编译通过 |
| R4-AC2 | ToolbarController.h/cpp 文件存在 | 文件检查 | 存在且编译通过 |
| R4-AC3 | MainWindow.cpp 行数 <= 500 | `wc -l` | 不超过 500 行 |
| R4-AC4 | 主题切换功能正常 | 手动: 工具栏下拉切换主题 | 主题切换生效，UI 正确刷新 |
| R4-AC5 | 语言切换功能正常 | 手动: 工具栏下拉切换语言 | 状态栏提示语言切换，重启后生效 |
| R4-AC6 | 窗口几何持久化正常 | 手动: 调整窗口大小 -> 关闭 -> 重新打开 | 窗口恢复到上次的位置和大小 |
| R4-AC7 | 串口配置持久化正常 | 手动: 选择串口参数 -> 关闭 -> 重新打开 | 配置面板恢复到上次的设置 |
| R5-AC1 | dark_terminal.qss 包含 themeCombo/langCombo 样式 | 代码检查 | 存在 `QComboBox#themeCombo` 和 `QComboBox#langCombo` |
| R5-AC2 | modern_dark.qss 包含 themeCombo/langCombo 样式 | 代码检查 | 同上 |
| R5-AC3 | light.qss 包含 themeCombo/langCombo 样式 | 代码检查 | 同上 |
| R5-AC4 | 工具栏 ComboBox 悬浮态有边框颜色变化 | 手动: 鼠标悬浮在主题/语言下拉框上 | border 颜色从默认变为 accent 色 |
| AC-1 | 编译零错误零警告 | `cmake --build build` | 0 error, 0 warning |
| AC-2 | EmbedDebug.bat 正常启动 | 双击 EmbedDebug.bat | 应用窗口正常显示 |
| AC-3 | 现有功能回归: 工具栏显示模式切换 | 手动: 切换文本/HEX/混合/十进制 | 终端显示模式正确切换 |
| AC-4 | 现有功能回归: 时间戳开关 | 手动: 点击时间戳按钮 | 终端显示/隐藏时间戳 |
| AC-5 | 现有功能回归: 清屏 | 手动: 点击清屏按钮 | 终端清空 |
| AC-6 | 现有功能回归: 导出 | 手动: 点击导出按钮 | 文件保存对话框弹出，导出正常 |
| AC-7 | 现有功能回归: 搜索高亮 | 手动: Ctrl+F 搜索 | 搜索高亮正常显示 |
| AC-8 | 现有功能回归: 驱动检测信息 | 手动: 查看配置面板底部 | 驱动信息标签正常显示 |

---

## 实施优先级

| 顺序 | 步骤 | 理由 |
|------|------|------|
| 1 | TerminalModel lineAt() 安全审计注释 (R1) | 最小改动，确认安全基线 |
| 2 | SerialConfigPanel driverInfoLbl 样式迁移 (R2) | 独立修改，消除代码规范违规 |
| 3 | MainWindow objectName 补全 (R3) | 前置条件: R5 的 QSS 选择器依赖这些 objectName |
| 4 | 工具栏 ComboBox QSS 补全 (R5) | 依赖 R3 的 objectName |
| 5 | SettingsController 提取 (R4-part1) | 独立提取，不影响工具栏 |
| 6 | ToolbarController 提取 (R4-part2) | 独立提取，需在 SettingsController 之后（themeCombo/langCombo 归属需明确） |
| 7 | MainWindow 适配和瘦身 (R4-part3) | 整合两个控制器，删除旧代码 |
| 8 | 编译验证 | 确保零错误 |
| 9 | 全功能回归测试 | 验证拆分后所有功能正常 |

---

## 验证度量指标

### 代码度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| MainWindow.cpp 行数 | `wc -l` | 630 行 | <= 500 行 |
| MainWindow.h 行数 | `wc -l` | 153 行 | <= 170 行 |
| SettingsController.h 行数 | `wc -l` | (新增) | <= 50 行 |
| SettingsController.cpp 行数 | `wc -l` | (新增) | <= 120 行 |
| ToolbarController.h 行数 | `wc -l` | (新增) | <= 60 行 |
| ToolbarController.cpp 行数 | `wc -l` | (新增) | <= 150 行 |
| 新增文件数 | 文件检查 | -- | 4 个 (.h/.cpp 对) |

### 架构度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| MainWindow 私有方法数 | 代码检查 | setupUI/setupStatusBar/connectSignals/updateStatusBar/updateDataStatistics + 构造/析构/关闭 (<= 8个) |
| MainWindow 成员指针数 | 代码检查 | 减少 6 个 toolbar 成员到 ToolbarController |
| SettingsController 依赖方向 | 代码检查 | 仅依赖 SettingsManager/ThemeManager/SerialConfigPanel (业务层 -> 基础设施层/表现层注入) |
| ToolbarController 依赖方向 | 代码检查 | 仅依赖 Qt Widget 类 (纯表现层) |
| 分层依赖方向 | 代码检查 | 无反向依赖 |
| objectName 覆盖率 | 全局扫描所有 new QWidget/QComboBox/QLabel | 100% 控件有 objectName |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| 编译零错误零警告 | cmake --build | 0 error, 0 warning |
| EmbedDebug.bat 启动 | 双击 bat | 正常启动 |
| 主题切换 | 手动: 三主题循环切换 | 切换即时生效，驱动信息标签颜色跟随主题 |
| 语言切换 | 手动: 中英切换 | 状态栏提示正确 |
| 窗口几何持久化 | 手动: resize + 重启 | 恢复正确 |
| 串口配置持久化 | 手动: 修改配置 + 重启 | 恢复正确 |
| 工具栏 ComboBox 样式 | 视觉检查: 三主题下的 themeCombo/langCombo | 边框、背景、悬浮态均正确 |
