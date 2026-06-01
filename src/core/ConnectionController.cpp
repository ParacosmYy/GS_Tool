/**
 * @file ConnectionController.cpp
 * @brief 连接控制器实现 - 管理串口/网络连接的创建、断开和状态分发
 *
 * 完整的连接生命周期管理:
 *   - 串口连接: connectSerial() -> 配置 -> 打开 -> DTR/RTS设置 -> 注入下游
 *   - 网络连接: connectNetwork() -> 配置 -> 打开 -> 注入下游
 *   - 断开连接: disconnectCurrent() -> 关闭 -> 清理 -> 清空下游
 *   - 超时保护: 5秒连接超时自动中断
 *   - 自动重连: 意外断开时可选自动重连
 *   - 热插拔: PortWatcher 检测端口拔出 -> 自动断开当前连接
 */

#include "core/ConnectionController.h"

#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>

#include "core/SendController.h"
#include "ota/OtaManager.h"
#include "core/RecordingController.h"
#include "serial/PortWatcher.h"
#include "utils/DataLogger.h"
#include "terminal/TerminalModel.h"

/**
 * @brief 构造连接控制器
 *
 * 初始化超时定时器、自动重连定时器和 PortWatcher，
 * 连接各自的信号到槽。PortWatcher 在构造时即启动轮询。
 *
 * @param connMgr 连接管理器（工厂），负责创建和销毁 IConnection 实例
 * @param parent 父对象
 */
ConnectionController::ConnectionController(ConnectionManager* connMgr, QObject* parent)
    : QObject(parent)
    , m_connManager(connMgr)
    , m_portWatcher(new PortWatcher(this))
{
    // 连接超时定时器：单次触发，超时后中断连接
    m_connectionTimer.setSingleShot(true);
    connect(&m_connectionTimer, &QTimer::timeout,
            this, &ConnectionController::onConnectionTimeout);

    // 自动重连定时器：间隔触发，每次检查是否需要重连
    connect(&m_reconnectTimer, &QTimer::timeout,
            this, &ConnectionController::onAutoReconnect);

    // 连接健康检测定时器: 每5秒检查一次连接状态和数据活跃度
    m_healthTimer.setInterval(5000);
    connect(&m_healthTimer, &QTimer::timeout, this, [this]() {
        if (!m_currentConn) {
            emit connectionHealth(false, -1);
            return;
        }
        // 判断连接是否仍处于Connected状态
        bool alive = (m_currentConn->state() == ConnectionState::Connected);
        // 计算距上次收到数据的时间间隔
        qint64 ageMs = -1;
        if (m_lastDataTimestamp > 0) {
            ageMs = QDateTime::currentMSecsSinceEpoch() - m_lastDataTimestamp;
        }
        emit connectionHealth(alive, ageMs);
    });

    // PortWatcher 信号: 端口拔出时自动断开，端口接入时通知上层
    connect(m_portWatcher, &PortWatcher::portRemoved,
            this, &ConnectionController::onPortRemoved);
    connect(m_portWatcher, &PortWatcher::portAdded,
            this, &ConnectionController::onPortAdded);

    // 启动热插拔检测（应用运行期间持续监控）
    m_portWatcher->start();
    // 信号线状态轮询定时器(200ms)，仅当信号线实际变化时才发射通知
    m_pinoutPollTimer = new QTimer(this);
    m_pinoutPollTimer->setInterval(200);
    connect(m_pinoutPollTimer, &QTimer::timeout, this, [this]() {
        if (!m_currentConn) return;
        auto current = m_currentConn->pinoutSignals();
        if (memcmp(&current, &m_lastPinout, sizeof(PinoutSignals)) != 0) {
            m_lastPinout = current;
            emit pinoutSignalsChanged(current);
        }
    });
}

/** @brief 析构函数，停止所有定时器和 PortWatcher */
ConnectionController::~ConnectionController()
{
    stopConnectionTimeout();
    m_reconnectTimer.stop();
    m_healthTimer.stop();
    if (m_pinoutPollTimer) m_pinoutPollTimer->stop();
    if (m_portWatcher) m_portWatcher->stop();
}

void ConnectionController::setSendController(SendController* ctrl) { m_sendController = ctrl; }
void ConnectionController::setOtaManager(OtaManager* mgr) { m_otaManager = mgr; }
void ConnectionController::setRecordingController(RecordingController* ctrl) { m_recordingController = ctrl; }

/** @brief 创建并打开串口连接 @param serialParams 串口参数 */
void ConnectionController::connectSerial(const QVariantMap& serialParams)
{
    // 步骤1: 关闭已有连接
    if (m_currentConn) {
        m_userInitiatedDisconnect = true;
        disconnectCurrent();
    }
    m_userInitiatedDisconnect = false;

    // 步骤2: 提取 DTR/RTS 参数，在 open() 成功后再设置
    bool dtrEnabled = serialParams.value("dtr", true).toBool();
    bool rtsEnabled = serialParams.value("rts", true).toBool();

    // 构建 configure() 参数（排除 DTR/RTS）
    QVariantMap configParams = serialParams;
    configParams.remove("dtr");
    configParams.remove("rts");

    // 步骤3: 通过工厂创建连接
    m_currentConn = m_connManager->createConnection(ConnectionType::Serial);
    if (!m_currentConn) {
        emit connectionFailed(tr("Not Supported"), tr("Serial connection not available"));
        return;
    }

    // 步骤4: 配置串口参数
    m_currentConn->configure(configParams);

    // 步骤5: 连接信号
    connectSignals(m_currentConn);

    // 步骤6: 启动超时定时器
    m_connectionTimer.start(kConnectionTimeoutMs);

    // 步骤7: 尝试打开端口
    if (!m_currentConn->open()) {
        stopConnectionTimeout();
        emit connectionFailed(tr("Connection Failed"), tr("Cannot open serial port"));
        disconnect(m_currentConn, nullptr, this, nullptr);  // 断开信号，防止 removeConnection 触发已连接的槽
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        m_connectedPortName.clear();
        return;
    }

    // 步骤8: 打开成功，停止超时定时器
    stopConnectionTimeout();
    // 设置 DTR/RTS（某些芯片在 open 时会重置线路信号）
    m_currentConn->setDtr(dtrEnabled);
    m_currentConn->setRts(rtsEnabled);
    // 注入到下游控制器
    if (m_sendController) m_sendController->setConnection(m_currentConn);
    if (m_otaManager) m_otaManager->setConnection(m_currentConn);
    // 记录成功的连接参数，用于自动重连和端口拔出检测
    m_lastConnectParams = serialParams;
    m_lastConnectType = ConnectionType::Serial;
    m_connectedPortName = serialParams.value("portName").toString();

    // 通知 Toast: 串口连接成功
    emit connectionSucceeded(m_connectedPortName);
}

/** @brief 关闭当前连接, 设置用户主动断开标记防止自动重连 */
void ConnectionController::disconnectCurrent()
{
    m_userInitiatedDisconnect = true;
    m_reconnectTimer.stop();
    m_reconnectAttemptCount = 0;

    if (m_currentConn) {
        // 缓存端口名称，断开后 m_connectedPortName 会被清空
        const QString portName = m_connectedPortName;
        teardownConnection(tr("user disconnect"));
        // 通知 Toast: 用户主动断开连接
        emit connectionDisconnected(portName);
    }
}

/** @brief 创建网络连接 @param type 连接类型 */
void ConnectionController::connectNetwork(ConnectionType type)
{
    // 构建默认网络参数(首次连接使用)
    QVariantMap params;
    if (type == ConnectionType::TcpClient) {
        params["mode"] = "client";
        params["host"] = "127.0.0.1";
        params["port"] = 8080;
    } else if (type == ConnectionType::TcpServer) {
        params["mode"] = "server";
        params["port"] = 8080;
    } else if (type == ConnectionType::Udp) {
        params["localPort"] = 8888;
        params["remoteHost"] = "127.0.0.1";
        params["remotePort"] = 8080;
    }
    connectNetwork(type, params);
}

/**
 * @brief 创建网络连接(带参数，用于自动重连)
 * @param type 连接类型
 * @param params 网络连接参数(host/port等)
 */
void ConnectionController::connectNetwork(ConnectionType type, const QVariantMap& params)
{
    // 关闭已有连接
    if (m_currentConn) {
        m_userInitiatedDisconnect = true;
        disconnectCurrent();
    }
    m_userInitiatedDisconnect = false;

    m_currentConn = m_connManager->createConnection(type);
    if (!m_currentConn) {
        emit connectionFailed(tr("Not Supported"), tr("This connection type is not yet available"));
        return;
    }

    m_currentConn->configure(params);

    // 连接信号
    connectSignals(m_currentConn);

    // 启动超时定时器
    m_connectionTimer.start(kConnectionTimeoutMs);

    if (!m_currentConn->open()) {
        stopConnectionTimeout();
        emit connectionFailed(tr("Connection Failed"),
                             tr("Cannot establish network connection"));
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        m_connectedPortName.clear();
        return;
    }

    stopConnectionTimeout();
    // 注入到下游控制器
    if (m_sendController) m_sendController->setConnection(m_currentConn);
    if (m_otaManager) m_otaManager->setConnection(m_currentConn);

    m_lastConnectType = type;
    m_lastConnectParams = params;  // 保存网络连接参数，用于自动重连
    m_connectedPortName.clear();  // 网络连接无串口端口名

    // 通知 Toast: 网络连接成功
    emit connectionSucceeded(m_currentConn ? m_currentConn->name() : tr("Network"));
}

IConnection* ConnectionController::currentConnection() const { return m_currentConn; }
void ConnectionController::setDtr(bool enabled) { if (m_currentConn) m_currentConn->setDtr(enabled); }
void ConnectionController::setRts(bool enabled) { if (m_currentConn) m_currentConn->setRts(enabled); }
/** @brief 发送Break信号(用于STM32/ESP32进入Bootloader) @param duration Break持续时间(毫秒) */
void ConnectionController::sendBreak(int duration) { if (m_currentConn) m_currentConn->sendBreak(duration); }
/** @brief 启用/禁用自动重连 @param enabled 是否启用 @param intervalMs 重连基础间隔(毫秒) @param maxRetries 最大重连次数(0=无限制) */
void ConnectionController::enableAutoReconnect(bool enabled, int intervalMs, int maxRetries)
{
    m_autoReconnectEnabled = enabled;
    m_reconnectMaxRetries = maxRetries;
    m_reconnectBaseIntervalMs = intervalMs;
    if (enabled) m_reconnectTimer.setInterval(intervalMs);
    else m_reconnectTimer.stop();
}

bool ConnectionController::isAutoReconnectEnabled() const { return m_autoReconnectEnabled; }
PortWatcher* ConnectionController::portWatcher() const { return m_portWatcher; }

/**
 * @brief 连接状态变化内部处理
 *
 * 在断开/错误状态下:
 *   - 清除下游控制器的连接引用
 *   - 如果启用了自动重连且非用户主动断开，启动重连定时器
 */
void ConnectionController::onConnectionStateChanged(ConnectionState state)
{
    // 先缓存连接名称
    QString connName = m_currentConn ? m_currentConn->name() : "";

    switch (state) {
    case ConnectionState::Connected:
        // 连接成功，停止超时定时器和重连定时器
        stopConnectionTimeout();
        m_reconnectTimer.stop();
        // 如果是重连成功，发出通知并重置计数
        if (m_reconnectAttemptCount > 0) {
            emit reconnectSucceeded(connName);
            m_reconnectAttemptCount = 0;
        }
        if (m_recordingController) {
            m_recordingController->setConnected(true);
        }
        m_pinoutPollTimer->start();
        // 启动连接健康检测定时器
        m_lastDataTimestamp = QDateTime::currentMSecsSinceEpoch();
        m_healthTimer.start();
        break;

    case ConnectionState::Disconnected:
    case ConnectionState::Error:
        stopConnectionTimeout();
        m_pinoutPollTimer->stop();
        m_healthTimer.stop();
        clearDownstreamConnections();

        // 清除已连接端口名（连接已断开）
        m_connectedPortName.clear();

        // 错误状态: 发送 Toast 错误通知（区分 Error 和普通 Disconnected）
        if (state == ConnectionState::Error) {
            emit connectionError(connName, tr("Connection error occurred"));
        }

        // 自动重连: 仅在非用户主动断开且已启用时触发
        if (m_autoReconnectEnabled && !m_userInitiatedDisconnect) {
            m_reconnectTimer.start();
        }
        break;

    case ConnectionState::Connecting:
        break;
    }

    // 转发状态变化信号
    emit connectionStateChanged(state, connName);
}

/**
 * @brief 接收数据内部处理
 *
 * 转发数据到 MainWindow 并请求状态栏刷新，
 * 同时更新最近收到数据的时间戳（用于连接健康检测）。
 */
void ConnectionController::onDataReceived(const QByteArray& data)
{
    m_lastDataTimestamp = QDateTime::currentMSecsSinceEpoch();
    emit dataReceived(data);
    emit statusBarUpdateRequested();
}

/**
 * @brief 连接超时处理
 *
 * 当 open() 后超过 kConnectionTimeoutMs 仍未变为 Connected 时触发。
 * 中断当前连接并通知用户。
 */
void ConnectionController::onConnectionTimeout()
{
    // 标记为非用户主动断开但禁止自动重连（超时重连毫无意义）
    m_userInitiatedDisconnect = true;
    m_reconnectTimer.stop();

    // 缓存端口名称，清理后 m_connectedPortName 会被清空
    const QString timeoutName = m_connectedPortName.isEmpty()
        ? (m_currentConn ? m_currentConn->name() : tr("Unknown"))
        : m_connectedPortName;

    qWarning() << "Connection timeout for" << timeoutName;

    teardownConnection(tr("connection timeout"));

    emit connectionFailed(tr("Connection Timeout"),
                         tr("Connection timed out after %1 seconds. "
                            "Please check the device and try again.")
                             .arg(kConnectionTimeoutMs / 1000));

    // 通知 Toast: 连接超时
    emit connectionError(timeoutName, tr("Connection timed out"));
}

/**
 * @brief 自动重连定时器触发（支持指数退避）
 *
 * 检查是否仍在断开状态且未由用户主动断开，若是则尝试重新连接。
 * 支持最大重连次数限制: 达到上限后停止重连并发出失败通知。
 *
 * 指数退避策略:
 *   actualInterval = baseInterval * 2^min(attempt, 4)，上限30秒
 *   例如: base=3s → 3s → 6s → 12s → 24s → 30s → 30s...
 * 每次尝试前通过 reconnectProgress 信号通知UI当前进度和下次等待时间。
 */
void ConnectionController::onAutoReconnect()
{
    // 如果已经连接或用户主动断开，停止重连
    if (m_currentConn || m_userInitiatedDisconnect) {
        m_reconnectTimer.stop();
        m_reconnectAttemptCount = 0;
        return;
    }

    // 检查是否达到最大重连次数（0 表示无限制）
    if (m_reconnectMaxRetries > 0 && m_reconnectAttemptCount >= m_reconnectMaxRetries) {
        m_reconnectTimer.stop();
        const QString reason = tr("Max reconnect attempts reached (%1)").arg(m_reconnectMaxRetries);
        emit reconnectFailed(reason);
        m_reconnectAttemptCount = 0;
        return;
    }

    m_reconnectAttemptCount++;

    // 计算指数退避间隔: baseInterval * 2^min(attempt, 4)，上限30秒
    const int maxExpShift = 4;
    const int maxIntervalMs = 30000;
    int expShift = qMin(m_reconnectAttemptCount, maxExpShift);
    int actualInterval = qMin(m_reconnectBaseIntervalMs * (1 << expShift), maxIntervalMs);

    // 通知UI当前重连进度和下次等待时间
    emit reconnectProgress(m_reconnectAttemptCount, m_reconnectMaxRetries, actualInterval);
    emit reconnectAttempt(m_reconnectAttemptCount, m_reconnectMaxRetries);

    qDebug() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz") << "]"
             << "Auto-reconnect attempt" << m_reconnectAttemptCount << "/"
             << (m_reconnectMaxRetries > 0 ? QString::number(m_reconnectMaxRetries) : "unlimited")
             << "next interval:" << actualInterval << "ms";

    if (m_lastConnectType == ConnectionType::Serial) {
        connectSerial(m_lastConnectParams);
    } else {
        // 使用保存的网络参数重连，而非硬编码默认值
        connectNetwork(m_lastConnectType, m_lastConnectParams);
    }

    // 设置下次重连的间隔（指数退避）
    if (m_reconnectTimer.isActive()) {
        m_reconnectTimer.setInterval(actualInterval);
    }
}

/** @brief 端口物理拔出: 匹配当前连接则自动断开 @param portName 端口名 */
void ConnectionController::onPortRemoved(const QString& portName)
{
    // 仅在当前有活跃串口连接且端口名匹配时才断开
    if (m_currentConn && m_currentConn->type() == ConnectionType::Serial
        && m_connectedPortName == portName) {
        qWarning() << "Connected port" << portName << "was removed, disconnecting...";

        // 标记为非用户主动断开（物理拔出属于意外断开，可触发自动重连）
        m_userInitiatedDisconnect = false;

        teardownConnection(tr("port removed: %1").arg(portName));

        // 通知 UI 连接因端口拔出而断开
        emit connectionStateChanged(ConnectionState::Disconnected, portName);
        emit connectionFailed(tr("Port Removed"),
                             tr("Serial port %1 was disconnected. "
                                "Please reconnect the device.")
                                 .arg(portName));
        // 通知 Toast: 端口被物理拔出
        emit connectionError(portName, tr("Port was physically removed"));
    }
}

/** @brief 端口接入: 转发信号到上层, 不自动连接 @param portName 端口名 */
void ConnectionController::onPortAdded(const QString& portName) { emit portAdded(portName); }

/**
 * @brief 连接 IConnection 的信号到内部槽
 * @param conn 需要连接信号的 IConnection 实例
 */
void ConnectionController::connectSignals(IConnection* conn)
{
    connect(conn, &IConnection::dataReceived,
            this, &ConnectionController::onDataReceived);
    connect(conn, &IConnection::stateChanged,
            this, &ConnectionController::onConnectionStateChanged);
    // 连接错误信号，转发详细错误信息到UI，同时发送 Toast 错误通知
    connect(conn, &IConnection::errorOccurred,
            this, [this](const QString& msg) {
        qWarning() << "Connection error:" << msg;
        const QString errPortName = m_connectedPortName.isEmpty()
            ? (m_currentConn ? m_currentConn->name() : tr("Unknown"))
            : m_connectedPortName;
        emit connectionFailed(tr("Connection Error"), msg);
        emit connectionError(errPortName, msg);
    });
    // 连接错误计数更新 → 通过信号通知表现层(避免业务层直接依赖表现层)
    connect(conn, &IConnection::errorOccurred, this, [this, conn]() {
        auto counters = conn->errorCounters();
        emit errorCountersUpdated(
            counters.framingErrors, counters.parityErrors, counters.overrunErrors);
    });
}

/**
 * @brief 统一的连接断开清理流程
 *
 * 从 disconnectCurrent()、onConnectionTimeout()、onPortRemoved() 提取的公共逻辑:
 *   1. 停止超时定时器
 *   2. 缓存并清空 m_currentConn / m_connectedPortName
 *   3. 断开信号连接（防止 close() 触发 onConnectionStateChanged 回调）
 *   4. 从 ConnectionManager 移除并销毁连接实例
 *   5. 清除下游控制器的连接引用
 *
 * @param reason 断开原因描述，用于日志输出
 */
void ConnectionController::teardownConnection(const QString& reason)
{
    stopConnectionTimeout();
    if (m_pinoutPollTimer) m_pinoutPollTimer->stop();

    if (!m_currentConn) return;

    qInfo() << "Tearing down connection:" << reason;

    // 缓存指针并立即清空成员，防止信号回调中访问
    IConnection* conn = m_currentConn;
    m_currentConn = nullptr;
    m_connectedPortName.clear();

    // 先断开信号，防止 removeConnection 内部 close() 触发的
    // stateChanged 信号进入 onConnectionStateChanged
    disconnect(conn, nullptr, this, nullptr);

    // 从管理器移除并销毁（removeConnection 内部执行 close + delete）
    m_connManager->removeConnection(conn);

    // 清除下游控制器的连接引用
    clearDownstreamConnections();
}

/**
 * @brief 清除所有下游控制器的连接引用
 *
 * 在连接断开或发生错误时调用，防止下游控制器持有悬空指针。
 */
void ConnectionController::clearDownstreamConnections()
{
    if (m_sendController) {
        m_sendController->setConnection(nullptr);
    }
    if (m_otaManager) {
        m_otaManager->setConnection(nullptr);
    }
    if (m_recordingController) {
        m_recordingController->setConnected(false);
    }
}

/** @brief 停止连接超时定时器 */
void ConnectionController::stopConnectionTimeout()
{
    if (m_connectionTimer.isActive()) {
        m_connectionTimer.stop();
    }
}
