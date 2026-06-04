/**
 * @file ConnectionControllerLifecycle.cpp
 * @brief 连接控制器生命周期与配置方法 - 串口/网络连接的创建、断开和参数构建
 *
 * 从 ConnectionController.cpp 拆分而来，包含:
 *   - connectSerial(): 创建并打开串口连接(关闭旧连接->提取DTR/RTS->工厂创建->配置->打开->注入下游)
 *   - disconnectCurrent(): 用户主动断开当前连接(不触发自动重连)
 *   - connectNetwork(ConnectionType): 创建网络连接(使用默认参数)
 *   - connectNetwork(ConnectionType, QVariantMap): 创建网络连接(带参数，用于手动连接和自动重连)
 */

#include "core/connect/ConnectionController.h"
#include "core/connect/ConnectionPresetBuilder.h"

#include "core/send/SendController.h"
#include "ota/manager/OtaManager.h"

/** @brief 创建并打开串口连接，完整流程: 关闭旧连接->提取DTR/RTS->工厂创建->配置->连接信号->超时保护->打开->注入下游 @param serialParams 串口参数映射(含portName/baudRate/dtr/rts等) */
void ConnectionController::connectSerial(const QVariantMap& serialParams)
{
    // 步骤1: 关闭已有连接
    if (m_currentConn) {
        // 统计: 如果上次连接类型不是串口，则为协议切换
        if (m_lastConnectType != ConnectionType::Serial) {
            ++m_totalProtocolSwitches;
        }
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
    ++m_totalConnectAttempts;  ///< 统计: 每次连接尝试递增(含后续失败)
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

/** @brief 关闭当前连接，设置用户主动断开标记防止自动重连，停止重连定时器并递增断开计数 */
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

/** @brief 创建网络连接(使用默认参数)，根据连接类型构建不同的默认host/port参数 @param type 连接类型枚举 */
void ConnectionController::connectNetwork(ConnectionType type)
{
    connectNetwork(type, ConnectionPresetBuilder::build(type));
}

/** @brief 创建网络连接(带参数，用于手动连接和自动重连)，完整流程: 关闭旧连接->工厂创建->配置->连接信号->超时保护->打开->注入下游 @param type 连接类型枚举 @param params 网络连接参数(host/port等) */
void ConnectionController::connectNetwork(ConnectionType type, const QVariantMap& params)
{
    // 关闭已有连接
    if (m_currentConn) {
        // 统计: 如果上次连接类型不同于当前类型，则为协议切换
        if (m_lastConnectType != type) {
            ++m_totalProtocolSwitches;
        }
        m_userInitiatedDisconnect = true;
        disconnectCurrent();
    }
    m_userInitiatedDisconnect = false;

    ++m_totalConnectAttempts;  ///< 统计: 每次网络连接尝试递增(含后续失败)

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
