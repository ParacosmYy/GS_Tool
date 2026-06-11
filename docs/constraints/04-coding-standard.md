# 04 - 编码规范

> 本文档是 EmbedDebug 约束体系的第4模块。每次编码时必须遵守。

---

## 一、语言和风格

- 统一使用 C++17。
- 任何涉及构建目录、启动脚本、Qt 运行依赖、CMake 配置的改动，必须基于单一 `build/` 路径验证，且保持 `EmbedDebug.bat` 双击可启动。
- 头文件引用顺序固定为：Qt -> STL -> 项目头文件。
- 项目内头文件优先使用相对 `src` 根目录的路径，例如 `#include "core/Constants.h"`。
- Qt 信号/槽必须使用新式 `connect` 语法，禁止 `SIGNAL` / `SLOT` 宏。
- 注释以中文为主，公开接口必须能让接手者快速理解职责和约束。

---

## 二、文件组织规则

### 2.1 默认组织方式

- 默认情况下，每个可复用类使用独立的 `.h` / `.cpp` 文件。
- 默认情况下，文件名与类名保持一致，采用 PascalCase。
- 默认情况下，源文件放在职责对应的子目录中，避免把不同层次的实现堆在一起。
- 如果一个新文件的归属不清楚，先回到 `03-architecture.md` 和 `07-directory-structure.md` 确认 canonical 目录，不要先创建平行分支目录。
- Serial Station 新文件必须使用 C++ `.h/.cpp` 组织，路径和职责按 `docs/serial_station_architecture.md` 执行，不允许按脚本语言目录样式创建 `.py` 生产文件。

### 2.2 允许的例外

以下情况可以不遵循“一类一对 `.h/.cpp`”：

- 纯接口类，只有抽象声明，没有实现细节时，可以只有 `.h`。
- 模板类、内联工具函数、编译期常量集合，若必须头文件内实现，可以采用 header-only。
- 仅供单个 `.cpp` 使用的内部辅助类型，可以放在同文件内作为匿名命名空间或局部私有类型。
- 由 Qt 元对象机制要求的简单类型封装，若拆分只会增加噪音，可以按可读性优先处理。

### 2.3 例外要求

- 选择例外时，必须在 PR 描述里说明原因。
- header-only 方案必须保证接口稳定、包含关系清晰，避免把实现细节扩散到全工程。
- 单文件内放多个类型时，必须保持“一个主职责 + 少量紧密辅助类型”的边界，禁止把无关功能塞进同一文件。

---

## 三、规模目标和拆分规则

### 3.1 默认目标值

以下是默认目标，不是死线：

| 项目 | 默认目标 | 说明 |
|------|----------|------|
| `.cpp` 文件 | 300 行以内 | 方便单次 review 和后续维护 |
| `.h` 文件 | 160 行以内 | 保持接口聚焦，减少包含负担 |
| 单个公开类 | 6 个以内核心职责点 | 超出时优先考虑拆分协作类 |
| 单个方法 | 60 行以内 | 超过时优先提炼为私有 helper |
| `MainWindow.cpp` | 300 行以内 | 作为主装配层，而不是业务层 |

### 3.2 触发拆分的信号

出现以下任一情况时，默认应拆分，而不是继续堆代码：

- 一个类同时负责数据处理、状态管理、UI 更新和持久化。
- 一个方法同时包含输入校验、状态转换、错误处理和 UI 反馈。
- 一个 `.cpp` 文件里出现多个“看起来独立”的流程函数，且彼此只通过成员变量传递状态。
- 公开方法数量快速增长，但外部调用点只有少数几个。
- 需要在同一文件里反复写“先做 A，再做 B，再做 C”的模板式流程。

### 3.3 例外机制

- 超过默认目标值不一定立即违规，但必须在 PR 中解释为什么暂时不拆。
- 允许短期超过目标值的场景包括：迁移期、一次性重构中间态、需要保持原子改动的修复、强约束的 Qt 绑定代码。
- 若文件已经超过 500 行，或方法已经超过 80 行，必须给出明确拆分计划，不能只写“后续优化”。
- 例外不能无限延期；如果同一文件在后续迭代里继续增长，应优先拆分而不是重复申请例外。

---

## 四、MainWindow 约束

### 4.1 默认职责

`MainWindow` 只承担“装配”和“转发”职责：

- 创建顶层对象并完成依赖注入。
- 组织 UI 容器、布局和页面切换。
- 连接信号/槽，把用户操作转发给 Controller / Manager。
- 保存和恢复窗口级别的界面状态。

### 4.2 明确禁止

- 禁止在 `MainWindow` 中编写串口、协议解析、文件导出、脚本回放等业务逻辑。
- 禁止把状态机、数据缓存、数据处理算法塞进 `MainWindow`。
- 禁止把复杂 if/else 流程写成“按钮点击即完成所有事情”的巨型槽函数。

### 4.3 默认目标与升级规则

| 项目 | 默认目标 | 升级条件 |
|------|----------|----------|
| `MainWindow.cpp` | 300 行以内 | 超过后优先拆分到 Controller / Manager / Helper |
| `MainWindow` 公开槽函数 | 8 个以内 | 超过后说明页面职责是否过载 |
| `MainWindow` 直接依赖对象 | 10 个以内 | 超过后检查是否需要门面类或页面分层 |

### 4.4 允许的例外

- 应用启动阶段的临时装配代码可以集中在 `MainWindow`，但必须保持可读。
- 少量纯 UI 跳转逻辑可以保留在 `MainWindow`，前提是不涉及业务状态修改。
- 若某个短期重构需要让 `MainWindow` 暂时变大，必须同时提供拆分后的落点说明。

### 4.5 推荐拆分方向

- 页面切换和导航逻辑交给 `NavigationController`。
- 串口连接和会话状态交给 `ConnectionController`。
- 发送与快捷输入交给 `SendController`。
- 录制、回放和历史数据交给 `RecordingController`。
- 设置项和持久化交给 `SettingsController`。

---

## 五、注释和文档规范

### 5.1 必须注释的对象

以下内容必须有中文注释：

- 公开类。
- 公开方法。
- 公开信号。
- 公共枚举及其关键枚举值。
- 需要外部维护者理解的成员变量。
- 非显而易见的生命周期、线程、拥有权约束。

### 5.2 注释的最低要求

注释不要求堆砌背景故事，但必须回答三个问题：

- 这个对象做什么。
- 它依赖什么、输出什么。
- 有哪些调用约束或副作用。

### 5.3 推荐格式

```cpp
/**
 * @brief 说明这个类或方法的职责
 * @param xxx 参数含义
 * @return 返回值含义
 *
 * 补充说明适用场景、线程约束或拥有权约束。
 */
```

### 5.4 禁止项

- 禁止用空泛描述代替真正解释，例如“用于管理某某”但不说明边界。
- 禁止让公开接口完全没有说明。
- 禁止长期保留 `TODO` / `FIXME` 而没有对应任务或追踪链接。

---

## 六、命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `SerialConnection` |
| 方法 | camelCase | `setBaudRate()` |
| 成员变量 | `m_` 前缀 + camelCase | `m_portName` |
| 常量 | `k` 前缀 + PascalCase | `kMaxBufferSize` |
| 枚举值 | PascalCase | `ConnectionState::Connected` |
| 文件名 | PascalCase.h / PascalCase.cpp | `SerialConnection.h` |
| 头文件卫士 | 全大写 | `SERIAL_CONNECTION_H` |
| 命名空间 | camelCase | `namespace hexConvert` |
| 宏 | UPPER_SNAKE_CASE | `EMBEDDEBUG_VERSION` |

### 6.1 Serial Station 命名补充

| 类型 | 规范 | 示例 |
|------|------|------|
| 工站入口类 | `SerialStation*` 前缀 | `SerialStationController` |
| UI 面板类 | `Serial*Panel` 或明确 QWidget 名称 | `SerialPortPanel`, `SerialLogPanel` |
| core 类 | `Serial*` 前缀，避免和旧模块重名 | `SerialManager`, `SerialDispatcher` |
| 协议接口 | `ISerialProtocol` | `ISerialProtocol.h` |
| 具体协议类 | 协议名 + `Protocol/Parser/Command/Frame` | `ModbusRtuProtocol`, `CustomMdParser` |
| 测试文件 | `test_serial_station_<对象>.cpp` 或 `tests/serial_station/test_<对象>.cpp` | `test_modbus_rtu_protocol.cpp` |

---

## 七、头文件规则

```cpp
#ifndef SERIAL_CONNECTION_H
#define SERIAL_CONNECTION_H

#include <QtCore/QString>
#include <memory>

#include "core/Types.h"

class SerialConnection : public QObject {
    Q_OBJECT

public:
    explicit SerialConnection(QObject* parent = nullptr);
    ~SerialConnection() override;

    SerialConnection(const SerialConnection&) = delete;
    SerialConnection& operator=(const SerialConnection&) = delete;

signals:
    void dataReady(const QByteArray& data);

private slots:
    void onInternalEvent();

private:
    QString m_portName;
};
#endif // SERIAL_CONNECTION_H
```

### 7.1 排序要求

- Qt 头文件在前。
- STL 头文件在中。
- 项目头文件放最后。

### 7.2 使用边界

- 头文件中只放声明和极少量必须内联的代码。
- 如果某个实现细节只是为了隐藏复杂度，不要把它们扩散到头文件。
- 对于模板、泛型或强依赖内联的工具类型，允许 header-only，但必须控制包含成本。

---

## 八、内存管理

- QObject 派生对象优先交给父子树管理生命周期。
- 非 QObject 对象优先使用 `std::unique_ptr`，只有确有共享所有权时才使用 `std::shared_ptr`。
- 禁止裸 `new` 之后没有明确拥有者的写法。
- 大缓冲区优先使用 `QByteArray`、`QVector`、`std::vector`，避免手工管理 `malloc` / `free`。

---

## 九、错误处理规范

### 9.1 严重等级

| 等级 | 含义 | 处理方式 |
|------|------|----------|
| INFO | 正常提示信息 | 记录日志，不中断流程 |
| WARNING | 可恢复异常 | 记录日志并提示用户 |
| ERROR | 功能不可用 | 记录日志、提示用户、禁用相关功能 |
| FATAL | 应用无法继续 | 记录日志、提示用户、安全退出 |

### 9.2 传播规则

1. 基础设施层错误必须向上传播，不能在底层静默吞掉。
2. catch 块至少要记录日志，禁止空 catch。
3. 面向用户的错误信息必须中文化，并使用 `tr()` 包裹。
4. 业务错误优先用枚举类或错误对象表达，禁止使用裸数字作为语义错误码。

```cpp
emit errorOccurred(ErrorLevel::Error, tr("串口 %1 打开失败: %2").arg(m_portName, error));
```

### 9.3 检查清单

- 所有可能失败的 IO 操作都有错误处理。
- 错误信号接到了 UI 提示或状态系统。
- catch 块不为空。
- 面向用户的错误文本已 `tr()` 包裹。

---

## 十、线程规范

### 10.1 主线程约束

| 对象类型 | 线程要求 | 原因 |
|---------|---------|------|
| 所有 `QWidget` 及其子类 | 必须在主线程 | GUI 操作不是线程安全的 |
| ThemeManager 操作 | 必须在主线程 | QSS 应用会触发界面刷新 |
| PanelManager 操作 | 必须在主线程 | 面板增删涉及 UI 生命周期 |

### 10.2 线程安全模式

1. 数据生产者-消费者优先使用已封装的线程安全缓冲结构。
2. 跨线程通信优先用 Qt 信号/槽，让连接类型由 Qt 自动决定。
3. 串口 IO、协议解析等耗时工作放在后台线程，结果通过信号返回主线程。

### 10.3 禁止事项

| 禁止 | 原因 | 正确做法 |
|------|------|----------|
| 在非主线程操作 QWidget | GUI 不线程安全 | 通过信号通知主线程 |
| 在非主线程调用 `setStyleSheet()` | 会触发界面更新 | 回到主线程统一刷新 |
| 在主线程用 `QThread::sleep()` 阻塞 | 冻结 UI | 使用事件驱动或 worker 线程 |

### 10.4 Serial Station 线程边界

- `SerialReaderWorker` 和 `SerialCommandWorker` 不允许持有 QWidget 指针。
- 串口读取结果必须通过 Qt signal 交给 `SerialStationController` 或 `SerialDispatcher`，再由主线程更新 UI。
- 协议 `feed()` 可以维护内部解析缓存，但不得直接启动线程、写文件或触发 UI。

---

## 十一、单元测试标准

### 11.1 测试文件命名与位置

```
tests/
├── test_crc.cpp
├── test_hexconverter.cpp
├── test_ringbuffer.cpp
├── test_frameparser.cpp
├── serial_station/
│   ├── test_serial_manager.cpp
│   ├── test_serial_dispatcher.cpp
│   └── test_modbus_rtu_protocol.cpp
└── test_<模块名>_<功能>.cpp
```

命名规则：`test_<被测模块>_<被测功能>.cpp`

### 11.2 编写要求

| 要求 | 说明 |
|------|------|
| 测试框架 | Qt Test（QTest） |
| 公开方法 | 优先覆盖正常路径和边界条件 |
| 独立可运行 | 单个测试文件可单独编译运行 |
| 不依赖外部状态 | 不依赖具体串口/网络连接 |
| 中文描述 | QTest 的描述信息用中文 |

### 11.3 覆盖率期望

| 模块类型 | 期望覆盖率 | 优先测试 |
|---------|-----------|---------|
| utils/ | ≥ 80% | CRC、HexConverter、RingBuffer |
| protocol/ | ≥ 70% | FrameParser、ModbusEngine |
| apps/serial_station/protocols/ | ≥ 80% | buildCommand、feed、半包/粘包、异常校验 |
| apps/serial_station/core/ | ≥ 70% | SerialManager、SerialDispatcher、SerialCodec |
| connection/ | ≥ 50% | IConnection 接口、SerialConnection |
| UI 层 | ≥ 30% | 关键交互逻辑 |

### 11.4 运行方式

```bash
cmake --build build --target test
cd build && ctest --output-on-failure
```
