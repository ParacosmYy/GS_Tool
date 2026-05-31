#include "core/ConnectionController.h"
#include "core/SendController.h"
#include "ota/OtaManager.h"
#include "core/RecordingController.h"
#include "utils/DataLogger.h"
#include "terminal/TerminalModel.h"

ConnectionController::ConnectionController(ConnectionManager* connMgr, QObject* parent)
    : QObject(parent)
    , m_connManager(connMgr)
{
}

void ConnectionController::setSendController(SendController* ctrl)
{
    m_sendController = ctrl;
}

void ConnectionController::setOtaManager(OtaManager* mgr)
{
    m_otaManager = mgr;
}

void ConnectionController::setRecordingController(RecordingController* ctrl)
{
    m_recordingController = ctrl;
}

void ConnectionController::connectSerial(const QVariantMap& serialParams)
{
    // 关闭已有连接
    if (m_currentConn) {
        disconnectSerial();
    }

    // 通过工厂创建连接（不依赖具体类型）
    m_currentConn = m_connManager->createConnection(ConnectionType::Serial);
    if (!m_currentConn) {
        emit connectionFailed(tr("Not Supported"), tr("Serial connection not available"));
        return;
    }

    // 使用 IConnection::configure() 统一配置（消除强转）
    m_currentConn->configure(serialParams);

    // 连接数据信号
    connectSignals(m_currentConn);

    // 尝试连接
    if (!m_currentConn->open()) {
        emit connectionFailed(tr("Connection Failed"), tr("Cannot open serial port"));
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        return;
    }

    // 同步连接到SendController和OTA管理器
    if (m_sendController) {
        m_sendController->setConnection(m_currentConn);
    }
    if (m_otaManager) {
        m_otaManager->setConnection(m_currentConn);
    }
}

void ConnectionController::disconnectSerial()
{
    if (m_currentConn) {
        m_currentConn->close();
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;

        if (m_sendController) {
            m_sendController->setConnection(nullptr);
        }
    }
}

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

    // 默认网络参数配置
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

    // 同步连接到SendController
    if (m_sendController) {
        m_sendController->setConnection(m_currentConn);
    }
}

IConnection* ConnectionController::currentConnection() const
{
    return m_currentConn;
}

void ConnectionController::onConnectionStateChanged(ConnectionState state)
{
    QString connName = m_currentConn ? m_currentConn->name() : "";

    // 同步连接状态到RecordingController
    switch (state) {
    case ConnectionState::Connected:
        if (m_recordingController) {
            m_recordingController->setConnected(true);
        }
        break;
    case ConnectionState::Disconnected:
    case ConnectionState::Error:
        if (m_recordingController) {
            m_recordingController->setConnected(false);
        }
        break;
    case ConnectionState::Connecting:
        break;
    }

    emit connectionStateChanged(state, connName);
}

void ConnectionController::onDataReceived(const QByteArray& data)
{
    emit dataReceived(data);
    emit statusBarUpdateRequested();
}

void ConnectionController::connectSignals(IConnection* conn)
{
    connect(conn, &IConnection::dataReceived,
            this, &ConnectionController::onDataReceived);
    connect(conn, &IConnection::stateChanged,
            this, &ConnectionController::onConnectionStateChanged);
    connect(conn, &IConnection::errorOccurred,
            this, [](const QString& msg) {
        qWarning() << "Connection error:" << msg;
    });
}
