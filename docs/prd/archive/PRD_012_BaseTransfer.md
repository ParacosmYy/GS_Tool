# PRD-012: BaseTransfer模板方法提取

## 背景

当前项目中 `XModemTransfer`、`YModemTransfer`、`ZModemTransfer` 三个OTA协议类各自独立实现，存在大量重复代码。经过逐行对比分析，以下代码模式在三个类中完全相同或高度相似：

**完全相同的代码（逐字重复）：**
- `setConnection(IConnection*)` -- 连接绑定/解绑逻辑，含 `disconnect`/`connect` 信号槽
- 构造函数 -- `QTimer` 初始化、`setSingleShot(true)`、`connect(timeout)`
- `onConnectionReadyRead()` -- 接收缓冲区追加 + 调用 `processReceivedData()`
- `isRunning()` -- 状态判断 `!= Idle && != Done && != Error`
- `finishTransfer()` -- `setState(Done)` + `emit progress(100, ...)` + `emit transferComplete()`

**结构相同、细节不同的代码：**
- `start()` -- 状态检查(`!= Idle`)、连接检查、文件加载、成员变量重置、状态机启动
- `cancel()` -- 置 `m_cancelled`、发取消字节、停定时器、`setState(Idle)`、发错误信号
- `onTimeout()` -- `m_retryCount++`、超过 `kMaxRetries` 报错、按当前状态重发
- `setState()` -- 简单赋值

**重复的成员变量（每个类各自声明一套）：**
- `m_conn` / `m_receiveBuffer` / `m_state` / `m_retryCount` / `m_cancelled` / `m_timeoutTimer`

**重复的信号：**
- `progress(int, qint64, qint64)` / `transferComplete()` / `transferError(const QString&)`

三个类共计约 820 行 `.cpp` 代码，其中重复部分约 180 行（占比约 22%）。提取公共基类后，每个子类可减少约 100-120 行代码，总代码量预计从约 820 行减少至约 650 行，同时消除未来新增协议时（如 Kermit）的重复劳动。

CLAUDE.md 第 4.1 节明确要求 BaseTransfer 使用模板方法模式：`BaseTransfer::execute()` 骨架。第 4.4 节公共组件清单已登记 `BaseTransfer.h/cpp`。本次PRD即落实该架构规划。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 创建 `BaseTransfer` 抽象基类，继承 `QObject`，定义模板方法和纯虚钩子 | P0 | ota/protocols/ |
| R2 | 将 `setConnection` 完全提升到基类，子类不再覆写 | P0 | ota/protocols/ |
| R3 | 将 `start()` 骨架提升到基类，子类通过钩子 `onStartInit()` 自定义初始化 | P0 | ota/protocols/ |
| R4 | 将 `cancel()` 骨架提升到基类，子类通过钩子 `sendCancelBytes()` 自定义取消帧 | P0 | ota/protocols/ |
| R5 | 将 `isRunning()` 提升到基类（使用通用 `State` 枚举判断） | P0 | ota/protocols/ |
| R6 | 将 `onConnectionReadyRead()` 提升到基类，子类实现 `processReceivedData()` | P0 | ota/protocols/ |
| R7 | 将 `onTimeout()` 骨架提升到基类，子类实现 `handleTimeout()` | P0 | ota/protocols/ |
| R8 | 将 `finishTransfer()` 提升到基类 | P0 | ota/protocols/ |
| R9 | 将公共成员变量（`m_conn`, `m_receiveBuffer`, `m_retryCount`, `m_cancelled`, `m_timeoutTimer`）和公共信号提升到基类 | P0 | ota/protocols/ |
| R10 | 定义通用 `State` 枚举（`Idle`, `Done`, `Error` 为公共终止态），子类通过 `using` 引入并扩展 | P0 | ota/protocols/ |
| R11 | 将 `setState()` 提升到基类，内联实现 | P0 | ota/protocols/ |
| R12 | 重构 `XModemTransfer` 继承 `BaseTransfer`，删除重复代码 | P0 | ota/protocols/ |
| R13 | 重构 `YModemTransfer` 继承 `BaseTransfer`，删除重复代码 | P0 | ota/protocols/ |
| R14 | 重构 `ZModemTransfer` 继承 `BaseTransfer`，删除重复代码 | P0 | ota/protocols/ |
| R15 | 更新 `OtaManager` 使用 `BaseTransfer*` 指针统一管理三个协议实例 | P1 | ota/ |
| R16 | 编译通过且功能回归测试通过 | P0 | 全局 |

## 接口设计

### BaseTransfer.h

```cpp
#ifndef BASE_TRANSFER_H
#define BASE_TRANSFER_H

#include <QObject>
#include <QTimer>
#include "connection/IConnection.h"

// OTA传输基类 - 模板方法模式
// 封装所有OTA协议共享的基础设施: 连接管理、超时重试、取消机制、
// 接收缓冲区、状态判断。子类只需实现协议相关的钩子方法。
class BaseTransfer : public QObject {
    Q_OBJECT

public:
    // 通用传输状态 - 所有协议共享的终止态判断
    // 子类可定义自己的 State 枚举，但 Idle/Done/Error 必须存在且语义一致
    enum class TransferState {
        Idle,
        Active,  // 泛指所有"正在工作"的中间状态
        Done,
        Error
    };

    explicit BaseTransfer(QObject* parent = nullptr);
    virtual ~BaseTransfer() = default;

    // 禁止拷贝和赋值（QObject派生类）
    BaseTransfer(const BaseTransfer&) = delete;
    BaseTransfer& operator=(const BaseTransfer&) = delete;

    // === 公共接口（最终方法，子类不覆写） ===

    // 设置传输连接（串口/TCP/UDP/RTT）
    // 完全在基类实现: 断开旧连接的信号槽、绑定新连接的 dataReceived
    void setConnection(IConnection* conn);

    // 开始传输（模板方法）
    // 骨架流程: 状态检查 -> 连接检查 -> 子类初始化 -> 启动超时
    // 子类通过 onStartInit() 注入协议特有逻辑
    bool start();

    // 取消传输（模板方法）
    // 骨架流程: 置标志 -> 发取消字节 -> 停定时器 -> 重置状态 -> 发错误信号
    // 子类通过 sendCancelBytes() 注入协议特有的取消帧
    void cancel();

    // 是否正在运行
    // 默认实现: 不是 Idle/Done/Error 即为运行中
    // 子类可覆写（如需更精确判断）
    virtual bool isRunning() const;

signals:
    // 三个协议完全相同的信号，提升到基类
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();
    void transferError(const QString& reason);

protected:
    // === 子类可访问的公共资源 ===
    IConnection* m_conn = nullptr;
    QByteArray m_receiveBuffer;
    int m_retryCount = 0;
    bool m_cancelled = false;
    QTimer* m_timeoutTimer;

    // 超时和重试常量（子类可在构造函数中修改）
    int m_maxRetries = 10;
    int m_timeoutMs = 5000;

    // === 钩子方法（子类必须实现） ===

    // start() 的协议特有初始化阶段
    // 负责加载文件数据、重置协议计数器、设置初始状态、发送首帧
    // 返回 true 表示初始化成功可继续，false 表示失败（基类会发 transferError）
    virtual bool onStartInit() = 0;

    // cancel() 的协议特有取消字节发送
    // 如 XModem/YModem 发 2x CAN，ZModem 发 8x Backspace + 2x CAN
    virtual void sendCancelBytes() = 0;

    // 处理接收数据（协议状态机核心）
    // 由基类 onConnectionReadyRead() 调用，每次有新数据追加到 m_receiveBuffer 后触发
    virtual void processReceivedData() = 0;

    // 处理超时（协议特有重发逻辑）
    // 基类已处理 m_retryCount++ 和超过上限的错误，子类只需处理当前状态的重发
    // 参数 exceededMaxRetry: true 表示已超过最大重试次数，子类应放弃
    virtual void handleTimeout(bool exceededMaxRetry) = 0;

    // === 基类提供的工具方法 ===

    // 完成传输（设置 Done + 发 progress(100) + 发 transferComplete）
    void finishTransfer();

    // 子类查询自身是否处于终止态
    // 默认实现使用 m_transferState，子类有自定义枚举时应覆写
    virtual bool isTerminalState() const;

private slots:
    void onConnectionReadyRead(const QByteArray& data);
    void onTimeout();

private:
    TransferState m_transferState = TransferState::Idle;

    void setTransferState(TransferState state);

    // 子类通过以下方法控制 m_transferState
    // 这些方法让基类统一管理 Idle/Done/Error 三态
    // 子类不应直接操作 m_transferState，而是调用这三个方法
protected:
    void markIdle();
    void markDone();
    void markError();
    bool isIdle() const { return m_transferState == TransferState::Idle; }
    bool isDone() const { return m_transferState == TransferState::Done; }
    bool isError() const { return m_transferState == TransferState::Error; }
};

#endif // BASE_TRANSFER_H
```

### BaseTransfer.cpp

```cpp
#include "ota/protocols/BaseTransfer.h"

BaseTransfer::BaseTransfer(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &BaseTransfer::onTimeout);
}

void BaseTransfer::setConnection(IConnection* conn)
{
    if (m_conn) {
        disconnect(m_conn, nullptr, this, nullptr);
    }
    m_conn = conn;
    if (m_conn) {
        connect(m_conn, &IConnection::dataReceived,
                this, &BaseTransfer::onConnectionReadyRead);
    }
}

bool BaseTransfer::start()
{
    if (!isIdle()) return false;
    if (!m_conn) {
        emit transferError("No connection set");
        return false;
    }

    // 重置公共状态
    m_retryCount = 0;
    m_cancelled = false;
    m_receiveBuffer.clear();

    // 委托给子类做协议特有初始化
    return onStartInit();
}

void BaseTransfer::cancel()
{
    m_cancelled = true;
    if (m_conn && !isIdle()) {
        sendCancelBytes();
    }
    m_timeoutTimer->stop();
    markIdle();
    emit transferError("Transfer cancelled by user");
}

bool BaseTransfer::isRunning() const
{
    return !isIdle() && !isDone() && !isError();
}

bool BaseTransfer::isTerminalState() const
{
    return isIdle() || isDone() || isError();
}

void BaseTransfer::finishTransfer()
{
    markDone();
    // 子类应传递正确的 bytesSent/totalBytes，这里发 100% 仅作为最终通知
    emit transferComplete();
}

void BaseTransfer::onConnectionReadyRead(const QByteArray& data)
{
    m_receiveBuffer.append(data);
    processReceivedData();
}

void BaseTransfer::onTimeout()
{
    if (isIdle()) return;

    m_retryCount++;
    bool exceeded = (m_retryCount > m_maxRetries);

    if (exceeded) {
        sendCancelBytes();
        markError();
        emit transferError("Transfer timeout: max retries exceeded");
        return;
    }

    handleTimeout(false);
}

void BaseTransfer::setTransferState(TransferState state)
{
    m_transferState = state;
}

void BaseTransfer::markIdle()
{
    setTransferState(TransferState::Idle);
}

void BaseTransfer::markDone()
{
    setTransferState(TransferState::Done);
}

void BaseTransfer::markError()
{
    setTransferState(TransferState::Error);
}
```

### 子类重构示例: XModemTransfer.h（重构后）

```cpp
#ifndef XMODEMTRANSFER_H
#define XMODEMTRANSFER_H

#include "ota/protocols/BaseTransfer.h"
#include "utils/CRC.h"

class XModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    enum Mode {
        Checksum,
        CRC,
        OneK
    };

    explicit XModemTransfer(QObject* parent = nullptr);

    void setMode(Mode mode);
    void setFilePath(const QString& path);
    void setData(const QByteArray& data);

protected:
    // === BaseTransfer 钩子实现 ===
    bool onStartInit() override;
    void sendCancelBytes() override;
    void processReceivedData() override;
    void handleTimeout(bool exceededMaxRetry) override;

private:
    // XMODEM协议控制字节
    static constexpr char SOH = 0x01;
    static constexpr char STX = 0x02;
    static constexpr char EOT = 0x04;
    static constexpr char ACK = 0x06;
    static constexpr char NAK = 0x15;
    static constexpr char CAN = 0x18;
    static constexpr char CRC_CHAR = 'C';

    // XMODEM内部状态（独立于 BaseTransfer 的 TransferState）
    enum class State {
        Idle,
        WaitingForStart,
        SendingBlock,
        SendingEOT,
        Done,
        Error
    };

    void setState(State newState);
    void sendBlock();
    void sendEOT();
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);

    Mode m_mode = CRC;
    QString m_filePath;
    QByteArray m_data;
    State m_xmodemState = State::Idle;
    int m_blockNumber = 1;
    qint64 m_bytesSent = 0;

    int blockSize() const {
        return (m_mode == OneK) ? 1024 : 128;
    }
};

#endif // XMODEMTRANSFER_H
```

### 子类重构示例: XModemTransfer.cpp（关键方法）

```cpp
bool XModemTransfer::onStartInit()
{
    // 加载文件数据
    if (m_data.isEmpty() && !m_filePath.isEmpty()) {
        QFile file(m_filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            emit transferError(QString("Cannot open file: %1").arg(m_filePath));
            return false;
        }
        m_data = file.readAll();
        file.close();
    }
    if (m_data.isEmpty()) {
        emit transferError("No data to transfer");
        return false;
    }

    m_blockNumber = 1;
    m_bytesSent = 0;

    // 等待接收方发送启动信号
    m_xmodemState = State::WaitingForStart;
    m_timeoutTimer->start(m_timeoutMs * 3);
    return true;
}

void XModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(2, CAN));
    }
}

void XModemTransfer::handleTimeout(bool exceededMaxRetry)
{
    Q_UNUSED(exceededMaxRetry);
    // 基类已处理超过重试上限的情况，这里只处理正常超时重发
    if (m_xmodemState == State::SendingBlock) {
        sendBlock();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (m_xmodemState == State::SendingEOT) {
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (m_xmodemState == State::WaitingForStart) {
        m_timeoutTimer->start(m_timeoutMs * 3);
    }
}
```

### 子类重构示例: ZModemTransfer.h（关键差异）

```cpp
class ZModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    explicit ZModemTransfer(QObject* parent = nullptr);

    void setFilePath(const QString& path);

protected:
    bool onStartInit() override;
    void sendCancelBytes() override;
    void processReceivedData() override;
    void handleTimeout(bool exceededMaxRetry) override;

private:
    // ZMODEM特有的超时配置（在构造函数中修改基类默认值）
    // m_timeoutMs = 10000;  (在构造函数中设置)
    // m_maxRetries = 10;    (使用基类默认值)

    // ... ZMODEM帧类型、内部状态、私有方法 ...
};
```

```cpp
ZModemTransfer::ZModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
    m_timeoutMs = 10000;  // ZMODEM使用更长的超时
}

void ZModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(8, 0x08));          // 8次backspace
        m_conn->write(QByteArray(2, static_cast<char>(0x18))); // 2次CAN
    }
}
```

### OtaManager 重构（使用 BaseTransfer* 统一接口）

```cpp
// OtaManager.h 重构后
class OtaManager : public QObject {
    Q_OBJECT

public:
    explicit OtaManager(QObject* parent = nullptr);
    void setConnection(IConnection* conn);
    bool startTransfer(const QString& filePath, const QString& protocol = "xmodem-crc");
    void cancelTransfer();
    bool isTransferring() const;

signals:
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();
    void transferError(const QString& reason);

private:
    IConnection* m_conn = nullptr;
    XModemTransfer* m_xmodem = nullptr;
    YModemTransfer* m_ymodem = nullptr;
    ZModemTransfer* m_zmodem = nullptr;

    // 统一绑定 BaseTransfer 的三个信号到 OtaManager 的转发
    void connectTransferSignals(BaseTransfer* transfer);
};
```

```cpp
// OtaManager.cpp 关键变更
OtaManager::OtaManager(QObject* parent)
    : QObject(parent)
    , m_xmodem(new XModemTransfer(this))
    , m_ymodem(new YModemTransfer(this))
    , m_zmodem(new ZModemTransfer(this))
{
    connectTransferSignals(m_xmodem);
    connectTransferSignals(m_ymodem);
    connectTransferSignals(m_zmodem);
}

void OtaManager::connectTransferSignals(BaseTransfer* transfer)
{
    connect(transfer, &BaseTransfer::progress,
            this, &OtaManager::progress);
    connect(transfer, &BaseTransfer::transferComplete,
            this, &OtaManager::transferComplete);
    connect(transfer, &BaseTransfer::transferError,
            this, &OtaManager::transferError);
}
```

## 依赖的公共组件

| 组件 | 文件 | 用途 |
|------|------|------|
| `IConnection` | `connection/IConnection.h` | 连接抽象接口，`dataReceived` 信号 |
| `CRC` | `utils/CRC.h` | 子类使用 CRC16-CCITT / CRC32 / checksum |
| `QTimer` | Qt Core | 超时重试定时器 |

## 设计模式

### 模板方法模式 (Template Method)

**意图**: 定义算法骨架，将某些步骤延迟到子类。子类在不改变算法结构的前提下重定义某些步骤。

**本项目应用**:

```
BaseTransfer::start()          <-- 模板方法（骨架，final）
  ├─ 检查状态 (!isIdle)
  ├─ 检查连接 (!m_conn)
  ├─ 重置公共状态 (m_retryCount, m_cancelled, m_receiveBuffer)
  └─ onStartInit()             <-- 钩子（子类实现）
       ├─ XModem: 加载文件 -> 等待NAK/'C'
       ├─ YModem: 加载文件列表 -> 等待'C'
       └─ ZModem: 加载文件 -> 发送ZRQINIT

BaseTransfer::cancel()         <-- 模板方法（骨架，final）
  ├─ m_cancelled = true
  ├─ sendCancelBytes()         <-- 钩子（子类实现）
  │    ├─ XModem: 2x CAN
  │    ├─ YModem: 2x CAN
  │    └─ ZModem: 8x Backspace + 2x CAN
  ├─ m_timeoutTimer->stop()
  ├─ markIdle()
  └─ emit transferError(...)

BaseTransfer::onTimeout()      <-- 模板方法（骨架，final）
  ├─ m_retryCount++
  ├─ 超过上限? -> sendCancelBytes() + markError() + emit error
  └─ handleTimeout(false)      <-- 钩子（子类实现）
       ├─ XModem: 按状态重发 sendBlock / sendEOT
       ├─ YModem: 按状态重发 sendBlock0 / sendBlock / sendEOT
       └─ ZModem: 按状态重发 sendZRQINIT / sendZFILE / sendZFIN

BaseTransfer::onConnectionReadyRead()  <-- 模板方法（骨架，final）
  ├─ m_receiveBuffer.append(data)
  └─ processReceivedData()     <-- 钩子（子类实现）
       ├─ XModem: 逐字节解析 NAK/ACK/CAN
       ├─ YModem: 逐字节解析 NAK/ACK/CAN/'C'
       └─ ZModem: 帧头定位 + HEX帧解析
```

**双状态机策略**:

基类维护一个轻量级 `TransferState`（Idle/Active/Done/Error），仅用于判断 `isRunning()` 和 `isTerminalState()`。子类维护各自详细的协议状态枚举（如 `XModemTransfer::State`），用于驱动协议状态机。两者独立运行，不冲突。

```
基类 TransferState:   Idle ──────── Active ──────── Done/Error
                        ↑             ↑
子类 XModem State:    Idle → WaitingForStart → SendingBlock → SendingEOT → Done/Error
子类 YModem State:    Idle → WaitingStart → SendingBlock0 → SendingData → ... → Done/Error
子类 ZModem State:    Idle → WaitingRinit → SendingFile → SendingData → ... → Done/Error
```

子类在 `onStartInit()` 中调用 `markActive()`（将基类状态从 Idle 切到 Active）来激活 `isRunning()`。子类的 `markDone()` / `markError()` / `markIdle()` 调用同时更新基类状态。

**为什么用模板方法而不是策略模式**:

策略模式（`IProtocol` 接口）已经用于 OTA 协议的外部切换（OtaManager 持有不同协议实例）。模板方法解决的是内部代码复用问题 -- 三个协议类之间的纵向共性提取。两者不冲突：策略管"切换"，模板方法管"复用"。

## 影响范围

### 新增文件
| 文件 | 说明 |
|------|------|
| `src/ota/protocols/BaseTransfer.h` | 抽象基类头文件 |
| `src/ota/protocols/BaseTransfer.cpp` | 抽象基类实现 |

### 修改文件
| 文件 | 变更内容 |
|------|---------|
| `src/ota/protocols/XModemTransfer.h` | 继承改为 `BaseTransfer`，删除重复声明 |
| `src/ota/protocols/XModemTransfer.cpp` | 删除 `setConnection`/`cancel`/`isRunning`/`onConnectionReadyRead`/`finishTransfer`/构造函数的重复实现，改为覆写钩子 |
| `src/ota/protocols/YModemTransfer.h` | 同上 |
| `src/ota/protocols/YModemTransfer.cpp` | 同上 |
| `src/ota/protocols/ZModemTransfer.h` | 同上 |
| `src/ota/protocols/ZModemTransfer.cpp` | 同上 |
| `src/ota/OtaManager.h` | 新增 `connectTransferSignals(BaseTransfer*)` 方法 |
| `src/ota/OtaManager.cpp` | 简化信号连接为统一方法调用 |
| `CMakeLists.txt` | 添加 `BaseTransfer.h/cpp` 到源文件列表 |

### 不受影响
- `IConnection` 接口不变
- `CRC` 工具不变
- `OtaWidget` UI层不变（通过 OtaManager 间接使用，接口不变）
- 三个协议的公开方法签名（`setFilePath`/`start`/`cancel`/`setMode`）不变，行为不变

## 验收标准

### 功能验收
1. `BaseTransfer` 编译通过，无警告
2. 三个子类重构后编译通过，无警告
3. XMODEM-Checksum 传输功能与重构前行为一致
4. XMODEM-CRC 传输功能与重构前行为一致
5. XMODEM-1K 传输功能与重构前行为一致
6. YMODEM 单文件传输功能与重构前行为一致
7. YMODEM 批量传输功能与重构前行为一致
8. ZMODEM 传输功能与重构前行为一致
9. 三种协议的取消操作均正确发送各自的取消帧
10. 三种协议的超时重试机制均正常工作
11. 三种协议的超时上限触发后均正确报错
12. `OtaManager` 通过 `BaseTransfer*` 统一连接信号后，UI 层进度/完成/错误信号正确传递
13. `isRunning()` 在传输中返回 `true`，在 Idle/Done/Error 状态返回 `false`

### 代码质量验收
14. 三个子类中不再存在 `setConnection` 的重复实现
15. 三个子类中不再存在 `isRunning` 的重复实现
16. 三个子类中不再存在 `onConnectionReadyRead` 的重复实现
17. 三个子类中不再存在 `finishTransfer` 的重复实现
18. 基类中无协议特有逻辑（无 XModem/YModem/ZModem 的字节常量或状态判断）
19. 每个子类的 `.cpp` 文件只包含协议特有逻辑（帧构建、状态机解析）
20. `OtaManager.cpp` 中三个协议的信号连接代码使用 `connectTransferSignals` 统一，无重复的 `connect` 调用

### 架构验收
21. `BaseTransfer` 位于业务层（`ota/protocols/`），依赖基础设施层（`IConnection`），不依赖表现层
22. 子类继承 `BaseTransfer` 的 `protected` 成员和方法，不破坏封装
23. `start()` / `cancel()` / `onTimeout()` / `onConnectionReadyRead()` 四个模板方法在基类中是 `final`（非 virtual），子类无法覆写
24. `onStartInit()` / `sendCancelBytes()` / `processReceivedData()` / `handleTimeout()` 四个钩子是纯虚函数，子类必须实现
25. 代码总行数减少（三个子类 `.cpp` 合计行数下降，基类代码量小于减少量之和）
