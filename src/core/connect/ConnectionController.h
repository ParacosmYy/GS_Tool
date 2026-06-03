/**
 * @file ConnectionController.h
 * @brief 连接控制器 - 管理串口/网络连接的完整生命周期
 *
 * 职责: 创建/断开连接、超时检测、自动重连、热插拔监控
 * 设计模式: 中介者模式(协调上下游) + 观察者模式(信号/槽通知)
 *
 * 协作: ConnectionManager(工厂) / SendController(发送) / OtaManager(OTA)
 *       RecordingController(录制) / PortWatcher(热插拔)
 */

#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include "connection/interface/IConnection.h"
#include "core/connect/ConnectionManager.h"

class SendController;
class OtaManager;
class RecordingController;
class PortWatcher;

/**
 * @brief 连接控制器 - 连接层的中介者，协调各模块的连接实例传递和状态同步
 *
 * 核心流程:
 *   连接: connectSerial() -> 工厂创建 -> 配置 -> 打开 -> DTR/RTS -> 注入下游
 *   断开: disconnectCurrent() -> 关闭 -> 移除 -> 清空下游引用
 *   超时: 定时器守护，超时自动中断并报告失败
 *   重连: 意外断开时可选自动重连
 */
class ConnectionController : public QObject {
    Q_OBJECT

public:
    /** @brief 构造连接控制器 @param connMgr 连接管理器(工厂) @param parent 父对象 */
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr);
    /** @brief 析构函数，停止所有定时器和PortWatcher */
    ~ConnectionController() override;

    /** @brief 注入发送控制器，连接成功后自动注入IConnection @param ctrl 发送控制器指针 */
    void setSendController(SendController* ctrl);
    /** @brief 注入OTA管理器 @param mgr OTA管理器指针 */
    void setOtaManager(OtaManager* mgr);
    /** @brief 注入录制控制器 @param ctrl 录制控制器指针 */
    void setRecordingController(RecordingController* ctrl);

    /** @brief 创建并打开串口连接 @param serialParams 端口/波特率等参数 */
    void connectSerial(const QVariantMap& serialParams);
    /** @brief 关闭当前连接(用户主动断开，不触发自动重连) */
    void disconnectCurrent();
    /** @brief 创建网络连接(默认参数) @param type TcpClient/TcpServer/Udp */
    void connectNetwork(ConnectionType type);
    /** @brief 创建网络连接(指定参数，用于自动重连) @param type 连接类型 @param params host/port等参数 */
    void connectNetwork(ConnectionType type, const QVariantMap& params);
    /** @brief 获取当前活跃连接，无连接时返回nullptr */
    IConnection* currentConnection() const;

    /** @brief 控制当前连接的DTR线路信号 @param enabled true=高电平, false=低电平 */
    void setDtr(bool enabled);
    /** @brief 控制当前连接的RTS线路信号 @param enabled true=高电平, false=低电平 */
    void setRts(bool enabled);
    /** @brief 发送Break信号(用于STM32/ESP32进入Bootloader) @param duration Break持续时间(毫秒) */
    void sendBreak(int duration = 100);

    /** @brief 启用/禁用自动重连 @param enabled 启用标志 @param intervalMs 重连间隔(毫秒) @param maxRetries 最大次数(0=无限) */
    void enableAutoReconnect(bool enabled, int intervalMs = 3000, int maxRetries = 0);
    /** @brief 查询自动重连是否启用 */
    bool isAutoReconnectEnabled() const;
    /** @brief 获取端口热插拔监控器实例 */
    PortWatcher* portWatcher() const;

    // ---- 连接统计 getter ----
    /** @brief 获取累计成功连接次数 */
    quint64 totalConnections() const;
    /** @brief 获取累计断开连接次数 */
    quint64 totalDisconnections() const;
    /** @brief 获取累计自动重连次数 */
    quint64 totalReconnects() const;
    /** @brief 获取累计连接错误次数 */
    quint64 errorCount() const;
    /** @brief 获取累计发送数据字节数 */
    quint64 totalDataSent() const;
    /** @brief 获取累计接收数据字节数 */
    quint64 totalDataReceived() const;
    /** @brief 重置连接统计计数器为初始值 */
    void resetConnectionStatistics();

signals:
    /** @brief 连接状态变化通知 @param state 新状态 @param connName 连接名称 */
    void connectionStateChanged(ConnectionState state, const QString& connName);
    /** @brief 接收到数据，转发自底层 IConnection::dataReceived */
    void dataReceived(const QByteArray& data);
    /** @brief 请求状态栏刷新 */
    void statusBarUpdateRequested();
    /** @brief 连接失败通知(带标题和详细信息) @param title 错误标题 @param message 详细中文描述 */
    void connectionFailed(const QString& title, const QString& message);
    /** @brief 检测到新端口接入 @param portName 新端口名称 */
    void portAdded(const QString& portName);
    /** @brief 连接成功通知(Toast) @param portName 端口名或连接描述 */
    void connectionSucceeded(const QString& portName);
    /** @brief 连接正常断开通知(Toast) @param portName 端口名 */
    void connectionDisconnected(const QString& portName);
    /** @brief 连接错误通知(Toast) @param portName 端口名 @param error 简短错误描述 */
    void connectionError(const QString& portName, const QString& error);
    /** @brief 自动重连尝试通知 @param attempt 当前第几次尝试 @param maxRetries 最大次数(0=无限) */
    void reconnectAttempt(int attempt, int maxRetries);
    /** @brief 自动重连进度通知(含指数退避间隔) @param attempt 尝试次数(从1开始) @param maxRetries 最大次数(0=无限) @param nextIntervalMs 下次等待间隔(毫秒) */
    void reconnectProgress(int attempt, int maxRetries, int nextIntervalMs);
    /** @brief 自动重连成功通知 @param connName 重连成功的连接名称 */
    void reconnectSucceeded(const QString& connName);
    /** @brief 自动重连失败通知(达到最大重连次数) @param reason 失败原因 */
    void reconnectFailed(const QString& reason);
    /** @brief 连接健康状态通知(每5秒触发) @param alive true=存活 false=断开/异常 @param lastDataAgeMs 距上次收到数据的毫秒数(-1=从未收到) */
    void connectionHealth(bool alive, qint64 lastDataAgeMs);
    /** @brief 信号线状态变化通知(200ms轮询，仅变化时发射) @param pinout 6个信号线电平状态 */
    void pinoutSignalsChanged(const PinoutSignals& pinout);
    /** @brief 通信错误计数器更新通知 @param framingErrors 帧错误 @param parityErrors 校验错误 @param overrunErrors 溢出错误 */
    void errorCountersUpdated(int framingErrors, int parityErrors, int overrunErrors);

private slots:
    /** @brief 底层连接状态变化处理: 清除下游/触发重连/发射Toast */
    void onConnectionStateChanged(ConnectionState state);
    /** @brief 转发接收数据并请求状态栏刷新 */
    void onDataReceived(const QByteArray& data);
    /** @brief 连接超时处理: 中断连接并通知失败 */
    void onConnectionTimeout();
    /** @brief 自动重连定时器触发 */
    void onAutoReconnect();
    /** @brief 端口拔出处理: 匹配当前连接则断开 @param portName 被拔出的端口 */
    void onPortRemoved(const QString& portName);
    /** @brief 端口接入处理: 转发portAdded信号 @param portName 新接入端口 */
    void onPortAdded(const QString& portName);

private:
    /** @brief 连接IConnection信号到内部槽 @param conn 目标连接实例 */
    void connectSignals(IConnection* conn);
    /** @brief 清除下游控制器的连接引用(防止悬空指针) */
    void clearDownstreamConnections();
    /** @brief 停止连接超时定时器 */
    void stopConnectionTimeout();
    /** @brief 统一断开清理: 停超时→缓存清空→断信号→移除连接→清下游 @param reason 断开原因 */
    void teardownConnection(const QString& reason);

    /**
     * @brief 计算指数退避重连间隔
     * 策略: actualInterval = baseInterval * 2^min(attempt, maxShift)，上限30秒
     * @param attempt 当前重连次数(从1开始)
     * @return 等待间隔(毫秒)
     */
    int calcBackoffInterval(int attempt) const;

    ConnectionManager* m_connManager;              ///< 连接管理器(工厂)
    IConnection* m_currentConn = nullptr;          ///< 当前活跃连接实例
    SendController* m_sendController = nullptr;    ///< 发送控制器引用
    OtaManager* m_otaManager = nullptr;            ///< OTA管理器引用
    RecordingController* m_recordingController = nullptr; ///< 录制控制器引用
    PortWatcher* m_portWatcher = nullptr;          ///< 端口热插拔监控器

    QTimer* m_pinoutPollTimer = nullptr;           ///< 信号线轮询定时器(200ms)
    PinoutSignals m_lastPinout;                    ///< 上次轮询的信号线状态(变化检测)

    QTimer m_connectionTimer;                      ///< 连接超时定时器(单次触发)
    static constexpr int kConnectionTimeoutMs = 5000; ///< 超时阈值5秒

    QTimer m_reconnectTimer;                       ///< 自动重连定时器(间隔触发)
    bool m_autoReconnectEnabled = false;           ///< 是否启用自动重连
    bool m_userInitiatedDisconnect = false;        ///< 用户主动断开标志(不触发自动重连)
    int m_reconnectMaxRetries = 0;                 ///< 最大重连次数(0=无限制)
    int m_reconnectAttemptCount = 0;               ///< 当前已重连次数
    int m_reconnectBaseIntervalMs = 3000;          ///< 重连基础间隔(毫秒)
    QVariantMap m_lastConnectParams;               ///< 上次连接参数(用于重连)
    ConnectionType m_lastConnectType = ConnectionType::Serial; ///< 上次连接类型
    QString m_connectedPortName;                   ///< 当前连接的串口名称

    QTimer m_healthTimer;                          ///< 连接健康检测定时器(5秒间隔)
    qint64 m_lastDataTimestamp = 0;                ///< 上次收到数据的epoch毫秒时间戳(0=从未收到)

    // 连接统计计数器
    quint64 m_totalConnections = 0;                ///< 累计成功连接次数
    quint64 m_totalDisconnections = 0;             ///< 累计断开连接次数
    quint64 m_totalReconnects = 0;                 ///< 累计自动重连次数
    quint64 m_errorCount = 0;                      ///< 累计连接错误次数
    quint64 m_totalDataSent = 0;                   ///< 累计发送数据字节数
    quint64 m_totalDataReceived = 0;               ///< 累计接收数据字节数
};

#endif // CONNECTIONCONTROLLER_H
