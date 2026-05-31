/**
 * @file ConnectionController.cpp
 * @brief 连接控制器实现 - 管理串口/网络连接的创建、断开和状态分发
 *
 * 完整的连接生命周期管理:
 *   - 串口连接: connectSerial() → 配置 → 打开 → DTR/RTS设置 → 注入下游
 *   - 网络连接: connectNetwork() → 配置 → 打开 → 注入下游
 *   - 断开连接: disconnectCurrent() → 关闭 → 清理 → 清空下游
 *   - 超时保护: 5秒连接超时自动中断
 *   - 自动重连: 意外断开后自动尝试恢复连接
 *   - 热插拔: PortWatcher 检测端口拔出 → 自动断开当前连接
 */

#include "core/ConnectionController.h"
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

    // PortWatcher 信号: 端口拔出时自动断开，端口接入时通知上层
    connect(m_portWatcher, &PortWatcher::portRemoved,
            this, &ConnectionController::onPortRemoved);
    connect(m_portWatcher, &PortWatcher::portAdded,
            this, &ConnectionController::onPortAdded);

    // 启动热插拔检测（应用运行期间持续监控）
    m_portWatcher->start();
}

/** @brief 析构函数，停止所有定时器和 PortWatcher */
ConnectionController::~ConnectionController()
{
    stopConnectionTimeout();
    m_reconnectTimer.stop();
    if (m_portWatcher) {
        m_portWatcher->stop();
    }
}

/** @brief 注入发送控制器引用 */
void ConnectionController::setSendController(SendController* ctrl)
{
    m_sendController = ctrl;
}

/** @brief 注入 OTA 管理器引用 */
void ConnectionController::setOtaManager(OtaManager* mgr)
{
    m_otaManager = mgr;
}

/** @brief 注入录制控制器引用 */
void ConnectionController::setRecordingController(RecordingController* ctrl)
{
    m_recordingController = ctrl;
}

/**
 * @brief 创建并打开串口连接
 *
 * 完整流程:
 *   1. 关闭已有连接（避免资源泄漏）
 *   2. 提取 DTR/RTS 参数（在 open 后单独设置）
 *   3. 通过工厂创建 SerialConnection 实例
 *   4. 配置串口参数（波特率/数据位/校验/停止位/流控）
 *   5. 连接 IConnection 信号到内部槽
 *   6. 启动连接超时定时器
 *   7. 尝试 open() 打开端口
 *   8. 成功后设置 DTR/RTS 并注入到下游控制器
 *
 * DTR/RTS 必须在 open() 之后设置: 某些 USB 转串口芯片(CH340/CP2102)
 * 在 open() 时会重置线路信号为默认值，覆盖 configure() 中的设置。
 *
 * @param serialParams 串口参数（portName/baudRate/dataBits/parity/stopBits/flowControl/dtr/rts）
 */
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
    if (m_sendController) {
        m_sendController->setConnection(m_currentConn);
    }
    if (m_otaManager) {
        m_otaManager->setConnection(m_currentConn);
    }

    // 记录成功的连接参数，用于自动重连和端口拔出检测
    m_lastConnectParams = serialParams;
    m_lastConnectType = ConnectionType::Serial;
    m_connectedPortName = serialParams.value("portName").toString();
}

/**
 * @brief 关闭当前活跃连接（串口或网络）
 *
 * 流程: 清空指针 → 断开信号 → 从管理器移除(close+delete) → 清除下游引用
 * 设置 m_userInitiatedDisconnect 标志防止触发自动重连。
 *
 * 不手动调用 conn->close()，由 removeConnection() 统一负责 close+delete，
 * 避免双重 close 和 stateChanged 信号的重复发射。
 */
void ConnectionController::disconnectCurrent()
{
    m_userInitiatedDisconnect = true;
    stopConnectionTimeout();
    m_reconnectTimer.stop();

    if (m_currentConn) {
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
}

/**
 * @brief 创建并打开网络连接（TCP/UDP）
 *
 * 流程与串口类似，区别在于参数由内部构建默认值。
 * @param type 连接类型: TcpClient, TcpServer, Udp
 */
void ConnectionController::connectNetwork(ConnectionType type)
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

    // 构建默认网络参数
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
    if (m_sendController) {
        m_sendController->setConnection(m_currentConn);
    }
    if (m_otaManager) {
        m_otaManager->setConnection(m_currentConn);
    }

    m_lastConnectType = type;
    m_connectedPortName.clear();  // 网络连接无串口端口名
}

/** @brief 获取当前活跃的连接实例 */
IConnection* ConnectionController::currentConnection() const
{
    return m_currentConn;
}

/**
 * @brief 运行时控制 DTR 线路信号
 * 通过 IConnection 虚方法调用，非串口连接为空操作
 */
void ConnectionController::setDtr(bool enabled)
{
    if (m_currentConn) {
        m_currentConn->setDtr(enabled);
    }
}

/**
 * @brief 运行时控制 RTS 线路信号
 * 通过 IConnection 虚方法调用，非串口连接为空操作
 */
void ConnectionController::setRts(bool enabled)
{
    if (m_currentConn) {
        m_currentConn->setRts(enabled);
    }
}

/**
 * @brief 启用或禁用自动重连
 * @param enabled true=启用
 * @param intervalMs 重连间隔（毫秒），默认 3000ms
 */
void ConnectionController::enableAutoReconnect(bool enabled, int intervalMs)
{
    m_autoReconnectEnabled = enabled;
    if (enabled) {
        m_reconnectTimer.setInterval(intervalMs);
    } else {
        m_reconnectTimer.stop();
    }
}

/** @brief 查询自动重连是否已启用 */
bool ConnectionController::isAutoReconnectEnabled() const
{
    return m_autoReconnectEnabled;
}

/** @brief 获取 PortWatcher 实例指针 */
PortWatcher* ConnectionController::portWatcher() const
{
    return m_portWatcher;
}

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
        // 连接成功，停止超时定时器
        stopConnectionTimeout();
        if (m_recordingController) {
            m_recordingController->setConnected(true);
        }
        break;

    case ConnectionState::Disconnected:
    case ConnectionState::Error:
        stopConnectionTimeout();
        clearDownstreamConnections();

        // 清除已连接端口名（连接已断开）
        m_connectedPortName.clear();

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
 * 转发数据到 MainWindow 并请求状态栏刷新
 */
void ConnectionController::onDataReceived(const QByteArray& data)
{
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
    qWarning() << "Connection timeout for"
               << (m_currentConn ? m_currentConn->name() : "unknown");

    // 标记为非用户主动断开但禁止自动重连（超时重连毫无意义）
    m_userInitiatedDisconnect = true;
    m_reconnectTimer.stop();

    // 清理当前连接（不手动 close，由 removeConnection 统一处理）
    if (m_currentConn) {
        IConnection* conn = m_currentConn;
        m_currentConn = nullptr;
        m_connectedPortName.clear();
        disconnect(conn, nullptr, this, nullptr);
        m_connManager->removeConnection(conn);
        clearDownstreamConnections();
    }

    emit connectionFailed(tr("Connection Timeout"),
                         tr("Connection timed out after %1 seconds. "
                            "Please check the device and try again.")
                             .arg(kConnectionTimeoutMs / 1000));
}

/**
 * @brief 自动重连定时器触发
 *
 * 检查是否仍在断开状态且未由用户主动断开，若是则尝试重新连接。
 */
void ConnectionController::onAutoReconnect()
{
    // 如果已经连接或用户主动断开，停止重连
    if (m_currentConn || m_userInitiatedDisconnect) {
        m_reconnectTimer.stop();
        return;
    }

    qInfo() << "Auto-reconnect attempt...";

    if (m_lastConnectType == ConnectionType::Serial) {
        connectSerial(m_lastConnectParams);
    } else {
        connectNetwork(m_lastConnectType);
    }
}

/**
 * @brief 串口设备移除处理
 *
 * PortWatcher 检测到端口被物理拔出时调用。
 * 判断被拔出的端口是否为当前活跃连接所使用的端口:
 *   - 是: 自动断开当前连接（标记为非用户主动断开，允许自动重连）
 *   - 否: 忽略（与当前连接无关的端口变化）
 *
 * @param portName 被移除的端口名称（如 "COM3"）
 */
void ConnectionController::onPortRemoved(const QString& portName)
{
    // 仅在当前有活跃串口连接且端口名匹配时才断开
    if (m_currentConn && m_currentConn->type() == ConnectionType::Serial
        && m_connectedPortName == portName) {
        qWarning() << "Connected port" << portName << "was removed, disconnecting...";

        // 标记为非用户主动断开（物理拔出属于意外断开，可触发自动重连）
        m_userInitiatedDisconnect = false;

        // 执行断开流程
        if (m_currentConn) {
            stopConnectionTimeout();
            IConnection* conn = m_currentConn;
            m_currentConn = nullptr;
            m_connectedPortName.clear();
            disconnect(conn, nullptr, this, nullptr);
            m_connManager->removeConnection(conn);
            clearDownstreamConnections();
        }

        // 通知 UI 连接因端口拔出而断开
        emit connectionStateChanged(ConnectionState::Disconnected, portName);
        emit connectionFailed(tr("Port Removed"),
                             tr("Serial port %1 was disconnected. "
                                "Please reconnect the device.")
                                 .arg(portName));
    }
}

/**
 * @brief 串口设备接入处理
 *
 * PortWatcher 检测到新串口设备时调用。
 * 不自动连接新发现的端口（安全考虑，避免意外连接错误设备），
 * 仅转发信号到上层以通知用户或刷新端口列表。
 *
 * @param portName 新增的端口名称（如 "COM5"）
 */
void ConnectionController::onPortAdded(const QString& portName)
{
    // 转发到上层（MainWindow 可显示通知，SerialConfigPanel 可刷新列表）
    emit portAdded(portName);
}

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
    // 连接错误信号，转发详细错误信息到UI（状态栏已显示Error状态，此处补充详细信息）
    connect(conn, &IConnection::errorOccurred,
            this, [this](const QString& msg) {
        qWarning() << "Connection error:" << msg;
        // 将详细错误信息通过 connectionFailed 信号转发到 MainWindow 状态栏
        emit connectionFailed(tr("连接错误"), msg);
    });
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
