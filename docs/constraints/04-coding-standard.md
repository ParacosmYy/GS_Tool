# 04 - 编码规范

> 本文档是 EmbedDebug 约束体系的第4模块。每次编码时必须遵守。

---

## 一、语言和风格

- C++17 标准
- 详细中文注释（开发者是C++/Qt新手）
- 头文件引用使用相对src目录的路径: `#include "core/Constants.h"`
- Qt信号/槽用新式connect语法（函数指针），**禁止** SIGNAL/SLOT 宏
- 每个类一对 .h/.cpp 文件，放在对应子目录中

---

## 二、MainWindow 嵌入式 main 哲学（铁律）

MainWindow 必须像嵌入式项目的 `main.c` 一样简洁:
- **只做三件事**: 初始化对象 → 组装 UI → 连接信号/槽
- **禁止在 MainWindow 中编写业务逻辑** — 所有逻辑委托给 Controller/Manager 类
- **MainWindow.cpp 目标行数: ≤500行**
- **每个 Controller 遵循单一职责**: ConnectionController、SendController、NavigationController、RecordingController、ToolbarController、SettingsController

```
// MainWindow 应该长这样:
int main() {
    init_objects();
    setup_ui();
    connect_signals();
    load_settings();
}
// 就这样，没有其他东西了
```

---

## 三、注释规范（铁律）

**注释是强制性的，必须详细到让 C++/Qt 初学者也能完全理解。**

| 元素 | 注释格式 | 必须包含 |
|------|---------|---------|
| 文件头 | `/** @file 文件名 @brief 一行描述 */` | 文件用途、设计思路 |
| 类 | `/** @brief 类描述 ... */` | 职责、协作关系、设计模式 |
| 公开方法 | `/** @brief 描述 @param 参数说明 @return 返回值说明 */` | 功能、参数含义、返回值 |
| 私有方法 | `/** @brief 描述 */` 或 `// 一行说明` | 功能说明 |
| 成员变量 | `///< 行内说明` 或 `/** @brief 说明 */` | 用途、取值范围 |
| 信号 | `/** @brief 信号描述 @param 参数说明 */` | 何时发射、参数含义 |
| 代码块 | `// ---- 分组标题 ----` | 逻辑分组 |

**禁止**:
- 禁止无注释的公开方法
- 禁止无注释的成员变量
- 禁止"// TODO"或"// FIXME"式注释长期存在

---

## 四、命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `SerialConnection` |
| 方法 | camelCase | `setBaudRate()` |
| 成员变量 | m_ 前缀 + camelCase | `m_portName` |
| 常量 | k 前缀 + PascalCase | `kMaxBufferSize` |
| 枚举值 | PascalCase | `ConnectionState::Connected` |
| 文件名 | PascalCase.h/cpp | `SerialConnection.h` |
| 头文件卫士 | 全大写 | `#ifndef SERIAL_CONNECTION_H` |
| 命名空间 | camelCase | `namespace hexConvert` |
| 宏 | UPPER_SNAKE_CASE | `#define EMBEDDEBUG_VERSION` |

---

## 五、头文件规则

```cpp
#ifndef NAMESPACE_CLASS_NAME_H     // 头文件卫士
#define NAMESPACE_CLASS_NAME_H

#include <Qt先>                     // Qt头文件在前
#include <STL次>                     // STL头文件次之
#include "项目头文件最后"            // 项目头文件最后

class ClassName : public QObject {  // 继承用public
    Q_OBJECT

public:
    explicit ClassName(QObject* parent = nullptr);
    ~ClassName() override;

    // 禁止拷贝和赋值（QObject派生类）
    ClassName(const ClassName&) = delete;
    ClassName& operator=(const ClassName&) = delete;

signals:
    void dataReady(const QByteArray& data);

private slots:
    void onInternalEvent();

private:
    QString m_memberVar;             // 成员变量在底部
};
```

---

## 六、内存管理

- QObject父子树管理生命周期，优先用 `new Xxx(parent)`
- 非QObject对象用 `std::unique_ptr` / `std::shared_ptr`
- **禁止裸 `new` 不配对 `delete`** — 必须有明确的拥有者
- 大缓冲区用 `QByteArray` 或 `std::vector`，不要手动 `malloc`

---

## 七、错误处理规范

### 错误严重等级

| 等级 | 枚举值 | 含义 | 处理方式 | 示例 |
|------|--------|------|---------|------|
| **INFO** | `ErrorLevel::Info` | 正常提示信息 | 日志记录，不中断流程 | "连接已断开" |
| **WARNING** | `ErrorLevel::Warning` | 可恢复的异常 | 日志记录 + 用户提示（ToastWidget） | "数据帧校验失败，已丢弃" |
| **ERROR** | `ErrorLevel::Error` | 功能不可用 | 日志记录 + 用户提示 + 禁用相关功能 | "串口打开失败" |
| **FATAL** | `ErrorLevel::Fatal` | 应用无法继续 | 日志记录 + 弹窗通知 + 安全退出 | "配置文件损坏" |

### 错误传播规则

1. **基础设施层向上传播** — 底层错误通过信号 `errorOccurred(ErrorLevel, QString)` 逐层上报
2. **禁止吞没错误** — catch块中至少记录日志，不允许空catch
3. **错误信息用中文** — 面向用户的所有错误提示必须是中文（tr()包裹）
4. **错误码规范** — 业务错误用枚举类定义，不使用裸数字

```cpp
// 正确的错误传播模式
emit errorOccurred(ErrorLevel::Error, tr("串口 %1 打开失败: %2").arg(m_portName, error));

// 禁止的做法
catch (...) { /* 吞没错误 */ }
```

### 错误处理检查清单

- [ ] 所有可能失败的IO操作有错误处理（open/send/read/connect）
- [ ] 错误信号已连接到UI提示
- [ ] catch块不为空
- [ ] 面向用户的错误信息已tr()包裹

---

## 八、线程规范

### 主线程约束（铁律）

| 对象类型 | 线程要求 | 原因 |
|---------|---------|------|
| 所有 QWidget 及其子类 | **必须在主线程** | Qt GUI操作不是线程安全的 |
| ThemeManager 操作 | **必须在主线程** | QSS应用涉及QWidget |
| PanelManager 操作 | **必须在主线程** | 面板增删涉及UI |

### 线程安全模式

1. **数据生产者-消费者** — 使用 `RingBuffer<T>`（已内置线程安全）
2. **跨线程信号通信** — 使用 Qt 自动连接类型（`Qt::AutoConnection`），跨线程自动变为队列连接
3. **后台任务** — 串口IO/协议解析等耗时操作放在QThread中，通过信号返回结果

```cpp
// 正确: 后台线程发信号，主线程更新UI
// Worker线程中:
emit dataParsed(result);  // 自动队列连接到主线程

// 主线程slot中:
void onUpdateUI(const ParsedResult& result) {
    m_chartWidget->updateData(result);  // 安全，在主线程
}
```

### 禁止事项

| 禁止 | 原因 | 正确做法 |
|------|------|---------|
| **禁止在非主线程操作QWidget** | Qt GUI不是线程安全的 | 信号/槽跨线程通信 |
| **禁止在非主线程调用setStyleSheet** | 触发QWidget重绘 | 通过信号通知主线程 |
| **禁止使用QThread::sleep阻塞主线程** | 冻结UI | QTimer或QThread worker |

---

## 九、单元测试标准

### 测试文件命名与位置

```
tests/
├── test_crc.cpp                    # 测试 CRC 计算正确性
├── test_hexconverter.cpp           # 测试 HEX 编解码
├── test_ringbuffer.cpp             # 测试环形缓冲区
├── test_frameparser.cpp            # 测试协议帧解析
└── test_<模块名>_<功能>.cpp        # 命名规则
```

**命名规则**: `test_<被测模块>_<被测功能>.cpp`

### 测试编写要求

| 要求 | 说明 |
|------|------|
| **测试框架** | Qt Test（QTest） |
| **每个公开方法至少1个测试** | 正常路径 + 边界条件 |
| **独立可运行** | 每个测试文件可独立编译运行 |
| **不依赖外部状态** | 不依赖具体串口/网络连接，使用mock数据 |
| **中文测试描述** | QTest的描述信息用中文 |

### 覆盖率期望

| 模块类型 | 期望覆盖率 | 优先测试 |
|---------|-----------|---------|
| utils/（工具类） | ≥ 80% | CRC, HexConverter, RingBuffer |
| protocol/（协议解析） | ≥ 70% | FrameParser, ModbusEngine |
| connection/（连接层） | ≥ 50% | IConnection接口, SerialConnection |
| UI层（core/widgets/） | ≥ 30% | 关键交互逻辑 |

### 测试运行

```bash
# 构建并运行全部测试
cmake --build build --target test
cd build && ctest --output-on-failure
```
