/**
 * @file ConnectionController.cpp
 * @brief 连接控制器实现 - 管理串口/网络连接的创建、断开和状态分发
 *
 * 完整的连接生命周期管理:
 *   - 串口连接: connectSerial() -> 配置 -> 打开 -> DTR/RTS设置 -> 注入下游
 *   - 网络连接: connectNetwork() -> 配置 -> 打开 -> 注入下游
 *   - 断开连接: disconnectCurrent() -> 关闭 -> 清理 -> 清空下游
 *   - 超时保护: 5秒连接超时自动中断
 *   - 热插拔: PortWatchHandler 检测端口拔出 -> 自动断开
 *   - 自动重连: ReconnectController 管理意外断开后的重连策略
 */

#include "core/ConnectionController.h"
#include "core/SendController.h"
#include "core/PortWatchHandler.h"
#include "core/ReconnectController.h"
#include "ota/OtaManager.h"
#include "core/RecordingController.h"
#include "serial/PortWatcher.h"
#include "utils/DataLogger.h"
#include "terminal/TerminalModel.h"

/** @brief 构造: 初始化超时定时器、PortWatchHandler和ReconnectController */
ConnectionController::ConnectionController(ConnectionManager* connMgr, QObject* parent)
    : QObject(parent)
    , m_connManager(connMgr)
    , m_portWatchHandler(new PortWatchHandler(this))
    , m_reconnectController(new ReconnectController(this))
{
    m_connectionTimer.setSingleShot(true);
    connect(&m_connectionTimer, &QTimer::timeout,
            this, &ConnectionController::onConnectionTimeout);

    connect(m_portWatchHandler, &PortWatchHandler::portRemovedDuringConnection,
            this, &ConnectionController::onPortRemovedDuringConnection);
    connect(m_portWatchHandler, &PortWatchHandler::portDetected,
            this, &ConnectionController::portAdded);

    connect(m_reconnectController, &ReconnectController::reconnectSerialRequested,
            this, &ConnectionController::connectSerial);
    connect(m_reconnectController, &ReconnectController::reconnectNetworkRequested,
            this, &ConnectionController::connectNetwork);
}

/** @brief 析构: 停止所有定时器 */
ConnectionController::~ConnectionController()
{
    stopConnectionTimeout();
    m_reconnectController->stopReconnect();
}

void ConnectionController::setSendController(SendController* ctrl) { m_sendController = ctrl; }
void ConnectionController::setOtaManager(OtaManager* mgr) { m_otaManager = mgr; }
void ConnectionController::setRecordingController(RecordingController* ctrl) { m_recordingController = ctrl; }

/**
 * @brief 创建并打开串口连接
 * 流程: 关闭旧连接 -> 提取DTR/RTS -> 工厂创建 -> 配置 -> 打开 -> 设置DTR/RTS -> 注入下游
 */
void ConnectionController::connectSerial(const QVariantMap& serialParams)
{
    if (m_currentConn) {
        m_reconnectController->markUserInitiatedDisconnect();
        disconnectCurrent();
    }
    m_reconnectController->clearUserInitiatedDisconnect();

    bool dtrEnabled = serialParams.value("dtr", true).toBool();
    bool rtsEnabled = serialParams.value("rts", true).toBool();

    QVariantMap configParams = serialParams;
    configParams.remove("dtr");
    configParams.remove("rts");

    m_currentConn = m_connManager->createConnection(ConnectionType::Serial);
    if (!m_currentConn) {
        emit connectionFailed(tr("Not Supported"), tr("Serial connection not available"));
        return;
    }

    m_currentConn->configure(configParams);
    connectSignals(m_currentConn);
    m_connectionTimer.start(kConnectionTimeoutMs);

    if (!m_currentConn->open()) {
        stopConnectionTimeout();
        emit connectionFailed(tr("Connection Failed"), tr("Cannot open serial port"));
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        m_connectedPortName.clear();
        return;
    }

    stopConnectionTimeout();
    m_currentConn->setDtr(dtrEnabled);
    m_currentConn->setRts(rtsEnabled);

    if (m_sendController) m_sendController->setConnection(m_currentConn);
    if (m_otaManager) m_otaManager->setConnection(m_currentConn);

    m_connectedPortName = serialParams.value("portName").toString();
    m_portWatchHandler->setConnectedPortName(m_connectedPortName);
    m_reconnectController->saveConnectParams(serialParams, ConnectionType::Serial);
}

/** @brief 关闭当前连接, 标记为用户主动断开以阻止自动重连 */
void ConnectionController::disconnectCurrent()
{
    m_reconnectController->markUserInitiatedDisconnect();
    stopConnectionTimeout();
    m_reconnectController->stopReconnect();

    if (m_currentConn) {
        IConnection* conn = m_currentConn;
        m_currentConn = nullptr;
        m_connectedPortName.clear();
        m_portWatchHandler->setConnectedPortName(QString());
        disconnect(conn, nullptr, this, nullptr);
        m_connManager->removeConnection(conn);
        clearDownstreamConnections();
    }
}

/** @brief 创建并打开网络连接 */
void ConnectionController::connectNetwork(ConnectionType type)
{
    if (m_currentConn) {
        m_reconnectController->markUserInitiatedDisconnect();
        disconnectCurrent();
    }
    m_reconnectController->clearUserInitiatedDisconnect();

    m_currentConn = m_connManager->createConnection(type);
    if (!m_currentConn) {
        emit connectionFailed(tr("Not Supported"), tr("This connection type is not yet available"));
        return;
    }

    QVariantMap params;
    if (type == ConnectionType::TcpClient) {
        params["mode"] = "client"; params["host"] = "127.0.0.1"; params["port"] = 8080;
    } else if (type == ConnectionType::TcpServer) {
        params["mode"] = "server"; params["port"] = 8080;
    } else if (type == ConnectionType::Udp) {
        params["localPort"] = 8888; params["remoteHost"] = "127.0.0.1"; params["remotePort"] = 8080;
    }
    m_currentConn->configure(params);
    connectSignals(m_currentConn);
    m_connectionTimer.start(kConnectionTimeoutMs);

    if (!m_currentConn->open()) {
        stopConnectionTimeout();
        emit connectionFailed(tr("Connection Failed"), tr("Cannot establish network connection"));
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        m_connectedPortName.clear();
        return;
    }

    stopConnectionTimeout();
    if (m_sendController) m_sendController->setConnection(m_currentConn);
    if (m_otaManager) m_otaManager->setConnection(m_currentConn);

    m_connectedPortName.clear();
    m_portWatchHandler->setConnectedPortName(QString());
    m_reconnectController->saveConnectParams(QVariantMap(), type);
}

IConnection* ConnectionController::currentConnection() const { return m_currentConn; }
void ConnectionController::setDtr(bool enabled) { if (m_currentConn) m_currentConn->setDtr(enabled); }
void ConnectionController::setRts(bool enabled) { if (m_currentConn) m_currentConn->setRts(enabled); }

/** @brief 委托给ReconnectController启用/禁用自动重连 */
void ConnectionController::enableAutoReconnect(bool enabled, int intervalMs)
{
    m_reconnectController->enableAutoReconnect(enabled, intervalMs);
}

/** @brief 获取PortWatcher实例(通过PortWatchHandler) */
PortWatcher* ConnectionController::portWatcher() const
{
    return m_portWatchHandler->portWatcher();
}

/** @brief 连接状态变化: 更新下游引用, 委托ReconnectController判断重连 */
void ConnectionController::onConnectionStateChanged(ConnectionState state)
{
    QString connName = m_currentConn ? m_currentConn->name() : "";
    switch (state) {
    case ConnectionState::Connected:
        stopConnectionTimeout();
        if (m_recordingController) m_recordingController->setConnected(true);
        break;
    case ConnectionState::Disconnected:
    case ConnectionState::Error:
        stopConnectionTimeout();
        clearDownstreamConnections();
        m_connectedPortName.clear();
        m_portWatchHandler->setConnectedPortName(QString());
        break;
    case ConnectionState::Connecting:
        break;
    }
    m_reconnectController->handleConnectionStateChanged(state);
    emit connectionStateChanged(state, connName);
}

void ConnectionController::onDataReceived(const QByteArray& data)
{
    emit dataReceived(data);
    emit statusBarUpdateRequested();
}

/** @brief 连接超时: 中断连接, 不触发自动重连 */
void ConnectionController::onConnectionTimeout()
{
    qWarning() << "Connection timeout for" << (m_currentConn ? m_currentConn->name() : "unknown");
    m_reconnectController->markUserInitiatedDisconnect();
    m_reconnectController->stopReconnect();

    if (m_currentConn) {
        IConnection* conn = m_currentConn;
        m_currentConn = nullptr;
        m_connectedPortName.clear();
        m_portWatchHandler->setConnectedPortName(QString());
        disconnect(conn, nullptr, this, nullptr);
        m_connManager->removeConnection(conn);
        clearDownstreamConnections();
    }
    emit connectionFailed(tr("Connection Timeout"),
                         tr("Connection timed out after %1 seconds.").arg(kConnectionTimeoutMs / 1000));
}

/** @brief 端口物理拔出: 断开连接, 标记为意外断开允许自动重连 */
void ConnectionController::onPortRemovedDuringConnection(const QString& portName)
{
    m_reconnectController->clearUserInitiatedDisconnect();

    if (m_currentConn) {
        stopConnectionTimeout();
        IConnection* conn = m_currentConn;
        m_currentConn = nullptr;
        m_connectedPortName.clear();
        m_portWatchHandler->setConnectedPortName(QString());
        disconnect(conn, nullptr, this, nullptr);
        m_connManager->removeConnection(conn);
        clearDownstreamConnections();
    }

    emit connectionStateChanged(ConnectionState::Disconnected, portName);
    emit connectionFailed(tr("Port Removed"),
                         tr("Serial port %1 was disconnected.").arg(portName));
    m_reconnectController->handleConnectionStateChanged(ConnectionState::Disconnected);
}

/** @brief 连接IConnection信号到内部槽 */
void ConnectionController::connectSignals(IConnection* conn)
{
    connect(conn, &IConnection::dataReceived, this, &ConnectionController::onDataReceived);
    connect(conn, &IConnection::stateChanged, this, &ConnectionController::onConnectionStateChanged);
    connect(conn, &IConnection::errorOccurred, this, [this](const QString& msg) {
        qWarning() << "Connection error:" << msg;
        emit connectionFailed(tr("Connection Error"), msg);
    });
}

/** @brief 清除下游控制器的连接引用, 防止悬空指针 */
void ConnectionController::clearDownstreamConnections()
{
    if (m_sendController) m_sendController->setConnection(nullptr);
    if (m_otaManager) m_otaManager->setConnection(nullptr);
    if (m_recordingController) m_recordingController->setConnected(false);
}

void ConnectionController::stopConnectionTimeout()
{
    if (m_connectionTimer.isActive()) m_connectionTimer.stop();
}
