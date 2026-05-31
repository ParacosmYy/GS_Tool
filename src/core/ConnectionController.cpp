/**
 * @file ConnectionController.cpp
 * @brief 连接控制器实现 - 管理串口/网络连接的创建、断开和状态分发
 */

#include "core/ConnectionController.h"
#include "core/SendController.h"
#include "ota/OtaManager.h"
#include "core/RecordingController.h"
#include "utils/DataLogger.h"
#include "terminal/TerminalModel.h"
#include "connection/SerialConnection.h"

/**
 * @brief 构造连接控制器
 * @param connMgr 连接管理器（工厂），负责创建和销毁 IConnection 实例
 * @param parent 父对象
 */
ConnectionController::ConnectionController(ConnectionManager* connMgr, QObject* parent)
    : QObject(parent)
    , m_connManager(connMgr)
{
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
 * 流程: 关闭已有连接 → 工厂创建 SerialConnection → 配置参数 → 连接信号 → 打开端口 → 注入到下游控制器
 * @param serialParams 串口参数 QMap（portName/baudRate/dataBits/parity/stopBits/flowControl/dtr/rts）
 */
void ConnectionController::connectSerial(const QVariantMap& serialParams)
{
    // 关闭已有连接，避免资源泄漏
    if (m_currentConn) {
        disconnectSerial();
    }

    // 通过工厂创建连接（不依赖具体类型，由 ConnectionManager 根据 ConnectionType 选择）
    m_currentConn = m_connManager->createConnection(ConnectionType::Serial);
    if (!m_currentConn) {
        emit connectionFailed(tr("Not Supported"), tr("Serial connection not available"));
        return;
    }

    // 使用 IConnection::configure() 统一配置（消除强转，由具体实现类解析参数）
    m_currentConn->configure(serialParams);

    // 连接 IConnection 的数据接收/状态变化/错误信号到内部槽
    connectSignals(m_currentConn);

    // 尝试打开端口
    if (!m_currentConn->open()) {
        emit connectionFailed(tr("Connection Failed"), tr("Cannot open serial port"));
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        return;
    }

    // 同步连接到 SendController 和 OTA 管理器（它们需要 IConnection 指针进行数据读写）
    if (m_sendController) {
        m_sendController->setConnection(m_currentConn);
    }
    if (m_otaManager) {
        m_otaManager->setConnection(m_currentConn);
    }
}

/**
 * @brief 关闭当前串口连接
 * 关闭端口 → 从管理器移除 → 清空当前连接指针 → 清除下游控制器的连接引用
 */
void ConnectionController::disconnectSerial()
{
    if (m_currentConn) {
        m_currentConn->close();
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;

        // 清除下游控制器的连接引用，防止悬空指针
        if (m_sendController) {
            m_sendController->setConnection(nullptr);
        }
    }
}

/**
 * @brief 创建并打开网络连接（TCP/UDP）
 *
 * 流程与串口类似，区别在于参数由内部构建默认值（host/port/mode）
 * @param type 连接类型: TcpClient, TcpServer, Udp
 */
void ConnectionController::connectNetwork(ConnectionType type)
{
    // 关闭已有连接
    if (m_currentConn) {
        disconnectSerial();
    }

    m_currentConn = m_connManager->createConnection(type);
    if (!m_currentConn) {
        emit connectionFailed(tr("Not Supported"), tr("This connection type is not yet available"));
        return;
    }

    // 构建默认网络参数（后续可由设置面板覆盖）
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

    // 连接数据信号
    connectSignals(m_currentConn);

    if (!m_currentConn->open()) {
        emit connectionFailed(tr("Connection Failed"),
                             tr("Cannot establish network connection"));
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        return;
    }

    // 同步连接到 SendController
    if (m_sendController) {
        m_sendController->setConnection(m_currentConn);
    }
    // 同步连接到 OtaManager（网络连接也支持 OTA 传输）
    if (m_otaManager) {
        m_otaManager->setConnection(m_currentConn);
    }
}

/** @brief 获取当前活跃的连接实例 */
IConnection* ConnectionController::currentConnection() const
{
    return m_currentConn;
}

/**
 * @brief 运行时控制 DTR 线路信号
 * 仅串口连接有效，通过 qobject_cast 安全转换后调用
 * @param enabled true=拉高 DTR, false=拉低 DTR
 */
void ConnectionController::setDtr(bool enabled)
{
    if (m_currentConn && m_currentConn->type() == ConnectionType::Serial) {
        auto* serial = qobject_cast<SerialConnection*>(m_currentConn);
        if (serial) serial->setDtr(enabled);
    }
}

/**
 * @brief 运行时控制 RTS 线路信号
 * 仅串口连接有效，通过 qobject_cast 安全转换后调用
 * @param enabled true=拉高 RTS, false=拉低 RTS
 */
void ConnectionController::setRts(bool enabled)
{
    if (m_currentConn && m_currentConn->type() == ConnectionType::Serial) {
        auto* serial = qobject_cast<SerialConnection*>(m_currentConn);
        if (serial) serial->setRts(enabled);
    }
}

/**
 * @brief 连接状态变化内部处理
 * 在断开/错误状态下同步清除下游控制器的连接引用，防止悬空指针
 * @param state 新连接状态
 */
void ConnectionController::onConnectionStateChanged(ConnectionState state)
{
    // 先缓存连接名称，避免后续操作导致指针失效
    QString connName = m_currentConn ? m_currentConn->name() : "";

    switch (state) {
    case ConnectionState::Connected:
        if (m_recordingController) {
            m_recordingController->setConnected(true);
        }
        break;
    case ConnectionState::Disconnected:
    case ConnectionState::Error:
        // 同步清除下游控制器的连接引用，防止悬空指针
        if (m_sendController) {
            m_sendController->setConnection(nullptr);
        }
        if (m_otaManager) {
            m_otaManager->setConnection(nullptr);
        }
        if (m_recordingController) {
            m_recordingController->setConnected(false);
        }
        break;
    case ConnectionState::Connecting:
        break;
    }

    // 转发状态变化信号到 MainWindow 用于 UI 更新
    emit connectionStateChanged(state, connName);
}

/**
 * @brief 接收数据内部处理
 * 转发数据到 MainWindow 并请求状态栏刷新
 * @param data 接收到的原始字节
 */
void ConnectionController::onDataReceived(const QByteArray& data)
{
    emit dataReceived(data);
    emit statusBarUpdateRequested();
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
    // 连接错误信号，仅打印日志（错误状态通过 stateChanged 处理）
    connect(conn, &IConnection::errorOccurred,
            this, [](const QString& msg) {
        qWarning() << "Connection error:" << msg;
    });
}
