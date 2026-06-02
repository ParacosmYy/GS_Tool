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
