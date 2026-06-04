/**
 * @file ConnectionController.cpp
 * @brief 连接控制器核心实现 - 构造/析构/依赖注入/定时器初始化
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

#include "shared/TimerConstants.h"
#include "core/send/SendController.h"
#include "ota/manager/OtaManager.h"
#include "core/recording/RecordingController.h"
#include "serial/port/PortWatcher.h"
#include "utils/log/DataLogger.h"
#include "terminal/model/TerminalModel.h"

/** @brief 构造连接控制器，初始化超时定时器、自动重连定时器、健康检测定时器和PortWatcher @param connMgr 连接管理器(工厂)，负责创建和销毁IConnection实例 @param parent 父对象 */
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

/** @brief 析构函数，停止所有定时器(连接超时/重连/健康检测/信号线轮询)和PortWatcher */
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

// connectSerial() / disconnectCurrent() / connectNetwork()
// → ConnectionControllerLifecycle.cpp

// currentConnection() / setDtr() / setRts() / sendBreak()
// portWatcher() / totalConnections() / totalDisconnections() / totalReconnects()
// errorCount() / totalDataSent() / totalDataReceived() / resetConnectionStatistics()
// → ConnectionControllerQuery.cpp

// enableAutoReconnect() / isAutoReconnectEnabled() → ConnectionControllerReconnect.cpp

// onConnectionStateChanged() / onDataReceived() / onConnectionTimeout()
// onPortRemoved() / onPortAdded() / connectSignals()
// teardownConnection() / clearDownstreamConnections() / stopConnectionTimeout()
// → ConnectionControllerHandlers.cpp

// onAutoReconnect() → ConnectionControllerReconnect.cpp
// calcBackoffInterval() → ConnectionControllerReconnect.cpp
