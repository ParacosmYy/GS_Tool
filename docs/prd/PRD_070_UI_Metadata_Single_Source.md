# PRD-070: UI元数据单一来源 -- PanelDescriptor收敛面板映射/包装/导航事实

## 背景
当前主窗口面板数量持续增长，面板相关元数据分散在多个文件中重复维护:

- `PanelManagerCreation.cpp` / `PanelManagerFactory*.cpp` 维护 raw widget 创建、`objectName` 和初始可见性。
- `PanelManagerWrap.cpp` 维护 BasePanel 包装和标题。
- `PanelManagerQuery.cpp` 维护导航分组、导航标题和 `allPanels()` 切换列表。
- `MainWindowInit.cpp` 再从 `panelMappings()` 派生命令面板和可选图标导航栏分类。

这种分散方式导致新增或调整面板时容易漏改标题、漏注册 wrapper、导航项和面板栈顺序不一致。第一阶段需要在不改变运行时行为的前提下，引入 `PanelDescriptor` 作为 `core/panels` 内部运行时描述表，让 `wrapPanels()`、`panelMappings()` 和 `allPanels()` 从同一份描述派生。

## 目标

| ID | 目标 | 说明 |
|----|------|------|
| G1 | 建立面板 UI 元数据单一来源 | 同一面板的稳定 ID、分组、标题、图标名、包装策略只在一处声明 |
| G2 | 降低新增面板的漏改风险 | 后续新增面板时减少重复清单维护 |
| G3 | 保持当前 UI 行为不变 | 不改变导航顺序、默认面板、会话恢复、响应式折叠和三类例外面板 |
| G4 | 为后续接口化预留边界 | 第一阶段不把 `QWidget*` 泄露到 `interfaces/`，后续再抽静态契约 |

## 非目标

- 不引入模块自注册机制。
- 不移动面板类、不删除历史分叉目录。
- 不修改 `NavigationController` 对外接口。
- 不把会话恢复从 index 改成 panel id。
- 不接入真实图标渲染；`iconName` 先作为元数据字段预留。
- 不改 `Terminal`、`SearchBar`、`QuickCmdBar` 三个现有例外行为。
- 不借本轮修正 `allPanels()` 中特殊控件与后续布局管理的历史语义；只保持当前返回集合与顺序。
- 不调整导航分类名称、面板标题文案或翻译上下文。
- 不调整 `IConnection`、`utils/`、CMake 构建组织。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 新增 `PanelWrapperPolicy`，表达 `Wrapped`、`RawPersistent`、`Overlay`、`FixedBar`、`Floating` 等包装/展示策略 | P0 | `src/core/panels/` |
| R2 | 新增 `PanelDescriptor`，包含 `id`、`objectName`、`groupKey`、`titleKey`、`iconName`、`rawWidget`、`wrapperPolicy`、`navOrder`、`stackOrder`、`navVisible`、`includeInPanelStack` | P0 | `src/core/panels/` |
| R3 | `PanelManager` 提供内部 `panelDescriptors()`，在 `createPanels()` 和 `wrapPanels()` 之后可返回完整运行时描述表 | P0 | `src/core/panels/` |
| R4 | `wrapPanels()` 从 descriptor 派生 BasePanel 包装器，不再单独维护一套标题清单 | P0 | `src/core/panels/` |
| R5 | `panelMappings()` 从 descriptor 派生 `NavPanelMapping`，保留现有返回类型和导航顺序 | P0 | `src/core/panels/`, `src/core/navigation/` |
| R6 | `allPanels()` 从 descriptor 派生面板栈列表，保留 Terminal/SearchBar/QuickCmdBar 的现有例外位置 | P0 | `src/core/panels/`, `src/core/mainwindow/` |
| R7 | 文档中明确图标与组件入口: `core/theme/IconManager.*` 为运行时图标入口，`core/widgets/` 为基础 UI 壳层，历史 `*2` 与多套 SVG provider 不承载新入口 | P1 | `docs/prd/`, 后续架构文档 |
| R8 | wrapper 创建规则保持不变: parent、初始隐藏、`objectName = raw.objectName + "Wrapper"`、`m_wrappers` 映射语义均不变 | P0 | `src/core/panels/` |
| R9 | 面板统计计数保持不变，`m_totalPanelCreations`、`m_totalPanelRegisters` 等增量语义与当前实现一致 | P0 | `src/core/panels/` |
| R10 | 翻译上下文保持不变，导航分组继续使用 `Nav`，面板标题继续使用 `MainWindow`，不引入裸用户可见字符串 | P0 | `src/core/panels/`, `src/core/navigation/` |

## 接口设计

### PanelWrapperPolicy

```cpp
/**
 * @brief 面板包装和展示策略
 *
 * 该枚举只表达 UI 编排语义，不绑定具体业务面板。
 */
enum class PanelWrapperPolicy {
    Wrapped,        ///< 使用标准 BasePanel 外壳，参与普通面板切换
    RawPersistent,  ///< 原始控件常驻显示，不套 BasePanel，如 Terminal
    Overlay,        ///< 叠加层控件，如 TerminalSearchBar
    FixedBar,       ///< 固定底栏控件，如 QuickCommandBar
    Floating        ///< 浮动窗口或对话框，不进入主面板切换栈
};
```

### PanelDescriptor

```cpp
/**
 * @brief 主窗口内置面板的运行时描述
 *
 * 第一阶段放在 core/panels 内部使用，允许持有 QWidget*。
 * 不放入 interfaces/，避免 L0 契约层依赖 QWidget 生命周期和 core 编排细节。
 */
struct PanelDescriptor {
    const char* id;             ///< 稳定英文 ID，如 "serial.config"
    const char* objectName;     ///< raw widget objectName
    const char* groupKey;       ///< 导航分组翻译键，使用 QT_TRANSLATE_NOOP("Nav", ...)
    const char* titleKey;       ///< 面板标题翻译键，使用 QT_TRANSLATE_NOOP("MainWindow", ...)
    const char* iconName;       ///< Lucide 逻辑名，不包含路径或 .svg
    QWidget* rawWidget;         ///< 已创建的原始面板
    PanelWrapperPolicy wrapperPolicy;
    int navOrder;               ///< 导航顺序，保持现有 panelMappings() 顺序
    int stackOrder;             ///< 面板栈顺序，保持现有 allPanels() 顺序
    bool navVisible;            ///< 是否出现在导航树/命令面板
    bool includeInPanelStack;   ///< 是否加入右侧面板栈布局
};
```

## 架构边界

- `PanelDescriptor` 第一阶段是 `core/panels` 内部运行时描述，不作为跨模块接口。
- `PanelDescriptor` 可以持有 `QWidget*`，但不得被下层模块 include。
- `NavigationController` 继续消费 `QVector<NavPanelMapping>`，不直接依赖 descriptor。
- `PanelManager` 继续只负责创建、注册、包装、映射和统计，不新增业务逻辑。
- `IPanelProvider::PanelMeta` 暂时保持不变；后续若要模块自注册，再抽取不含 `QWidget*` 的静态 `PanelDescriptor` 到 `src/interfaces/`。
- `iconName` 只保存 Lucide 逻辑名，不保存资源路径、颜色、尺寸、`QIcon` 或 `QPixmap`。
- descriptor 中的 `groupKey` 与 `titleKey` 必须保留现有翻译上下文语义: `QCoreApplication::translate("Nav", groupKey)` 与 `QCoreApplication::translate("MainWindow", titleKey)`。

## 影响范围

| 文件 | 变更类型 | 风险 |
|------|---------|------|
| `src/core/panels/PanelDescriptor.h` | 新增内部描述类型 | 低 |
| `src/core/panels/PanelManager.h` | 新增内部 descriptor 查询声明 | 中 |
| `src/core/panels/PanelManagerDescriptors.cpp` | 新增 descriptor 表派生逻辑 | 中 |
| `src/core/panels/PanelManagerWrap.cpp` | 改为从 descriptor 创建 wrapper | 中 |
| `src/core/panels/PanelManagerQuery.cpp` | 改为从 descriptor 派生 mappings/allPanels | 高 |
| `src/core/panels/PanelManagerPanels.h` | 更新新增面板维护注释，避免继续要求重复登记 mappings/allPanels/wrapPanels 三份元数据 | 低 |
| `src/core/panels/PanelManagerCreation.cpp` / `PanelManagerFactory*.cpp` | 原则上不改创建顺序，仅必要时对齐 objectName | 中 |
| `src/core/mainwindow/MainWindowInit.cpp` | 第一阶段不改或只保持消费旧接口 | 低 |

## 迁移步骤

1. 新增 `PanelDescriptor.h` 和 `PanelManagerDescriptors.cpp`。
2. 在 `PanelManager` 内部生成 descriptor 列表，顺序严格匹配当前 `panelMappings()` / `allPanels()` 行为。
3. 使用 `navOrder` 和 `stackOrder` 同时保留当前导航顺序和面板栈顺序，避免用单一列表顺序误改行为。
4. 将 `wrapPanels()` 改为遍历 descriptor，对 `Wrapped` 策略创建 `BasePanel`。
5. 将 `panelMappings()` 改为从 descriptor 中筛选 `navVisible` 面板并按 `navOrder` 生成旧 `NavPanelMapping`。
6. 将 `allPanels()` 改为从 descriptor 中筛选 `includeInPanelStack` 面板并按 `stackOrder` 生成布局列表。
7. 保持 `Terminal`、`SearchBar`、`QuickCmdBar` 三类例外策略不变。
8. 保持 `allPanels()` 对特殊控件的当前返回集合与顺序，避免本轮引入布局修正。
9. 保持 wrapper `objectName` 规则和面板统计计数语义不变。
10. 编译验证并做启动/导航冒烟验证。

## 风险与回归点

| 风险 | 说明 | 缓解 |
|------|------|------|
| 导航顺序变化 | 会影响会话 index 恢复和用户习惯 | descriptor 顺序必须按现有 `panelMappings()` 顺序迁移 |
| 面板栈顺序变化 | 会影响初始布局、动画目标和固定栏位置 | `allPanels()` 派生结果必须与现有列表一致 |
| wrapper 丢失 | 面板可能无法显示标题栏或动画 | `Wrapped` 面板数量需与当前 wrapper 数一致 |
| 例外面板误包装 | Terminal/SearchBar/QuickCmdBar 行为会变化 | 用 `RawPersistent`/`Overlay`/`FixedBar` 明确表达 |
| 翻译上下文丢失 | 导航标题和命令面板标签可能不可翻译 | `groupKey/titleKey` 继续使用现有翻译上下文 |
| 历史布局语义被顺手修正 | `allPanels()` 与后续终端布局管理存在特殊历史关系，贸然改变可能导致重复或缺失显示 | 第一阶段只保持当前集合与顺序，不做布局修复 |

## 验收标准

1. `panelMappings()`、`allPanels()`、`wrapPanels()` 均从同一份 descriptor 派生。
2. 导航树分组、标题、顺序与改动前保持一致。
3. 右侧面板栈顺序与改动前保持一致。
4. Terminal 仍为默认可见面板，不被 BasePanel 包装。
5. SearchBar 仍作为 overlay 搜索栏，不被 BasePanel 包装。
6. QuickCmdBar 仍为底部固定栏，不被 BasePanel 包装。
7. `NavigationController` 对外接口不变。
8. 无新增下层模块 include `core/panels` 或 `core/navigation`。
9. 编译通过: `cmake --build build`。
10. 启动验证通过，导航点击、命令面板跳转和会话恢复无明显回归。
11. wrapper 数量、wrapper objectName 规则、`m_wrappers` 映射语义与改动前一致。
12. `.h <= 200` 行、`.cpp <= 500` 行约束不被破坏。

## 后续阶段

- Phase 2: 为 `NavPanelMapping` 增加稳定 `id` 和 `iconName`，逐步替代按翻译标题反查。
- Phase 3: 将不含 `QWidget*` 的静态面板契约抽到 `src/interfaces/`，服务模块自注册。
- Phase 4: 统一图标渲染入口，冻结 `core/icon/`、`core/icons/`、`core/iconprovider/` 中的历史分叉。
