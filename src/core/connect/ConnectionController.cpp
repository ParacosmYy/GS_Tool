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

#include "core/connect/ConnectionController.h"

#include <QTimer>
#include <QDateTime>

#include "core/theme/Constants.h"
#include "core/send/SendController.h"
#include "ota/manager/OtaManager.h"
#include "core/recording/RecordingController.h"
#include "serial/port/PortWatcher.h"
#include "utils/log/DataLogger.h"
#include "terminal/model/TerminalModel.h"

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
    m_healthTimer.setInterval(Timers::kHealthCheckMs);
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
    m_pinoutPollTimer->setInterval(Timers::kPinoutPollMs);
    connect(m_pinoutPollTimer, &QTimer::timeout, this, [this]() {
        if (!m_currentConn) return;
        auto current = m_currentConn->pinoutSignals();
        if (current.cts != m_lastPinout.cts || current.dsr != m_lastPinout.dsr ||
            current.dcd != m_lastPinout.dcd || current.ri != m_lastPinout.ri ||
            current.dtr != m_lastPinout.dtr || current.rts != m_lastPinout.rts) {
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

/** @brief 注入SendController依赖(用于发送数据时的字节数追踪) @param ctrl SendController指针 */
void ConnectionController::setSendController(SendController* ctrl) { m_sendController = ctrl; }
/** @brief 注入OtaManager依赖(用于OTA传输时的连接注入) @param mgr OtaManager指针 */
void ConnectionController::setOtaManager(OtaManager* mgr) { m_otaManager = mgr; }
/** @brief 注入RecordingController依赖(用于录制数据转发) @param ctrl RecordingController指针 */
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
        emit connectionFailed(tr("不支持"), tr("串口连接不可用"));
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
        emit connectionFailed(tr("连接失败"), tr("无法打开串口"));
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
    ++m_totalConnections;
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
        teardownConnection(tr("用户主动断开"));
        // 通知 Toast: 用户主动断开连接
        emit connectionDisconnected(portName);
        ++m_totalDisconnections;
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
    } else if (type == ConnectionType::WebSocket) {
        params["url"] = "ws://127.0.0.1:8080";
    } else if (type == ConnectionType::Mqtt) {
        params["host"] = "127.0.0.1";
        params["port"] = 1883;
    } else if (type == ConnectionType::Tls) {
        params["host"] = "127.0.0.1";
        params["port"] = 443;
    } else if (type == ConnectionType::Ble) {
        params["deviceName"] = "";
    } else if (type == ConnectionType::Can) {
        params["adapter"] = "can0";
        params["bitrate"] = 500000;
    } else if (type == ConnectionType::Spi) {
        params["device"] = "/dev/spidev0.0";
        params["speed"] = 1000000;
    } else if (type == ConnectionType::I2c) {
        params["device"] = "/dev/i2c-0";
        params["address"] = 0x50;
    } else if (type == ConnectionType::Usb) {
        params["vid"] = 0;
        params["pid"] = 0;
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
        emit connectionFailed(tr("不支持"), tr("该连接类型尚未实现"));
        return;
    }

    m_currentConn->configure(params);

    // 连接信号
    connectSignals(m_currentConn);

    // 启动超时定时器
    m_connectionTimer.start(kConnectionTimeoutMs);

    if (!m_currentConn->open()) {
        stopConnectionTimeout();
        emit connectionFailed(tr("连接失败"),
                             tr("无法建立网络连接"));
        // 先断开信号，防止 removeConnection 触发 close() 导致的 stateChanged 信号
        // 回调到 onConnectionStateChanged 产生重复错误通知
        disconnect(m_currentConn, nullptr, this, nullptr);
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
    emit connectionSucceeded(m_currentConn ? m_currentConn->name() : tr("网络"));
    ++m_totalConnections;
}

/** @brief 返回当前活动连接指针 @return IConnection指针，无连接时为nullptr */
IConnection* ConnectionController::currentConnection() const { return m_currentConn; }
/** @brief 设置DTR信号电平 @param enabled true=高电平 */
void ConnectionController::setDtr(bool enabled) { if (m_currentConn) m_currentConn->setDtr(enabled); }
/** @brief 设置RTS信号电平 @param enabled true=高电平 */
void ConnectionController::setRts(bool enabled) { if (m_currentConn) m_currentConn->setRts(enabled); }
/** @brief 发送Break信号(用于STM32/ESP32进入Bootloader) @param duration Break持续时间(毫秒) */
void ConnectionController::sendBreak(int duration) { if (m_currentConn) m_currentConn->sendBreak(duration); }
// enableAutoReconnect() / isAutoReconnectEnabled() → ConnectionControllerReconnect.cpp
/** @brief 返回端口监听器 @return PortWatcher指针 */
PortWatcher* ConnectionController::portWatcher() const { return m_portWatcher; }

// onConnectionStateChanged() / onDataReceived() / onConnectionTimeout()
// onPortRemoved() / onPortAdded() / connectSignals()
// teardownConnection() / clearDownstreamConnections() / stopConnectionTimeout()
// → ConnectionControllerHandlers.cpp

// onAutoReconnect() → ConnectionControllerReconnect.cpp
// calcBackoffInterval() → ConnectionControllerReconnect.cpp

/** @brief 获取累计成功连接次数 @return 连接总次数 */
quint64 ConnectionController::totalConnections() const { return m_totalConnections; }

/** @brief 获取累计断开连接次数 @return 断开总次数 */
quint64 ConnectionController::totalDisconnections() const { return m_totalDisconnections; }

/** @brief 获取累计自动重连次数 @return 重连总次数 */
quint64 ConnectionController::totalReconnects() const { return m_totalReconnects; }

/** @brief 获取累计连接错误次数 @return 错误总次数 */
quint64 ConnectionController::errorCount() const { return m_errorCount; }

/** @brief 获取累计发送数据字节数 @return 发送总字节数 */
quint64 ConnectionController::totalDataSent() const { return m_totalDataSent; }

/** @brief 获取累计接收数据字节数 @return 接收总字节数 */
quint64 ConnectionController::totalDataReceived() const { return m_totalDataReceived; }

/** @brief 重置连接统计计数器为初始值 */
void ConnectionController::resetConnectionStatistics()
{
    m_totalConnections = 0;
    m_totalDisconnections = 0;
    m_totalReconnects = 0;
    m_errorCount = 0;
    m_totalDataSent = 0;
    m_totalDataReceived = 0;
}
