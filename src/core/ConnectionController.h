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
#include "connection/IConnection.h"
#include "core/ConnectionManager.h"

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
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr); ///< @param connMgr 连接管理器(工厂)
    ~ConnectionController() override; ///< 停止所有定时器和 PortWatcher

    void setSendController(SendController* ctrl);       ///< 注入发送控制器，连接成功后注入 IConnection
    void setOtaManager(OtaManager* mgr);                ///< 注入 OTA 管理器
    void setRecordingController(RecordingController* ctrl); ///< 注入录制控制器

    void connectSerial(const QVariantMap& serialParams); ///< 创建并打开串口连接 @param serialParams 端口/波特率等参数
    void disconnectCurrent();                            ///< 关闭当前连接(用户主动，不触发自动重连)
    void connectNetwork(ConnectionType type);            ///< 创建网络连接(默认参数) @param type TcpClient/TcpServer/Udp
    void connectNetwork(ConnectionType type, const QVariantMap& params); ///< 创建网络连接(指定参数，用于自动重连) @param type 连接类型 @param params host/port等参数
    IConnection* currentConnection() const;              ///< 获取当前活跃连接，无连接时返回 nullptr

    void setDtr(bool enabled);                           ///< 控制当前连接的 DTR 线路信号
    void setRts(bool enabled);                           ///< 控制当前连接的 RTS 线路信号
    void sendBreak(int duration = 100);                  ///< 发送Break信号(用于STM32/ESP32进入Bootloader)

    void enableAutoReconnect(bool enabled, int intervalMs = 3000, int maxRetries = 0); ///< 启用/禁用自动重连 @param intervalMs 重连间隔(毫秒) @param maxRetries 最大重连次数(0=无限制)
    bool isAutoReconnectEnabled() const;                 ///< 查询自动重连状态
    PortWatcher* portWatcher() const;                    ///< 获取 PortWatcher 实例

signals:
    /** @brief 连接状态变化通知 @param state 新状态 @param connName 连接名称 */
    void connectionStateChanged(ConnectionState state, const QString& connName);

    /** @brief 接收到数据，转发自底层 IConnection::dataReceived */
    void dataReceived(const QByteArray& data);

    /** @brief 请求状态栏刷新 */
    void statusBarUpdateRequested();

    /** @brief 连接失败通知(带标题和详细信息)，由端口打开失败/超时/错误/拔出触发 @param title 错误标题 @param message 详细中文描述 */
    void connectionFailed(const QString& title, const QString& message);

    /** @brief 检测到新端口接入，由 PortWatcher 转发 @param portName 新端口名称 */
    void portAdded(const QString& portName);

    /** @brief 连接成功通知(Toast)，串口/网络open()成功后发射 @param portName 端口名或连接描述 */
    void connectionSucceeded(const QString& portName);

    /** @brief 连接正常断开通知(Toast)，用户主动断开时发射 @param portName 端口名 */
    void connectionDisconnected(const QString& portName);

    /** @brief 连接错误通知(Toast)，通信错误/超时/端口拔出时发射 @param portName 端口名 @param error 简短错误描述 */
    void connectionError(const QString& portName, const QString& error);

    /** @brief 自动重连尝试通知 @param attempt 当前第几次尝试 @param maxRetries 最大重连次数(0=无限制) */
    void reconnectAttempt(int attempt, int maxRetries);

    /**
     * @brief 自动重连进度通知(含指数退避间隔)
     *
     * 每次重连尝试前发射，携带下次重连的等待间隔，供UI显示倒计时或进度。
     *
     * @param attempt 当前第几次尝试(从1开始)
     * @param maxRetries 最大重连次数(0=无限制)
     * @param nextIntervalMs 下次重连的等待间隔(毫秒，经指数退避计算)
     */
    void reconnectProgress(int attempt, int maxRetries, int nextIntervalMs);

    /** @brief 自动重连成功通知 @param connName 重连成功的连接名称 */
    void reconnectSucceeded(const QString& connName);

    /** @brief 自动重连失败通知(达到最大重连次数) @param reason 失败原因描述 */
    void reconnectFailed(const QString& reason);

    /**
     * @brief 连接健康状态通知
     *
     * 每5秒由健康检测定时器触发，报告当前连接是否存活以及距上次收到数据的时间间隔。
     * UI可据此显示"连接空闲"警告或"数据停滞"提示。
     *
     * @param alive true=连接仍处于Connected状态，false=连接已断开或异常
     * @param lastDataAgeMs 距上次收到数据的毫秒数，-1表示从未收到数据
     */
    void connectionHealth(bool alive, qint64 lastDataAgeMs);

    /** @brief 信号线状态变化通知(由200ms轮询定时器触发，仅变化时发射)
     *  @param pinout 当前6个信号线的电平状态(CTS/DSR/DCD/RI/DTR/RTS) */
    void pinoutSignalsChanged(const PinoutSignals& pinout);

    /**
     * @brief 通信错误计数器更新通知
     * @param framingErrors 帧错误计数
     * @param parityErrors 校验错误计数
     * @param overrunErrors 溢出错误计数
     */
    void errorCountersUpdated(int framingErrors, int parityErrors, int overrunErrors);

private slots:
    void onConnectionStateChanged(ConnectionState state); ///< 底层状态变化: 清除下游/触发重连/发射Toast
    void onDataReceived(const QByteArray& data);          ///< 转发数据并请求状态栏刷新
    void onConnectionTimeout();                           ///< 连接超时: 中断连接并通知失败
    void onAutoReconnect();                               ///< 自动重连定时器触发
    void onPortRemoved(const QString& portName);          ///< 端口拔出: 匹配当前连接则断开 @param portName 被拔出的端口
    void onPortAdded(const QString& portName);            ///< 端口接入: 转发 portAdded 信号

private:
    void connectSignals(IConnection* conn);       ///< 连接 IConnection 信号到内部槽
    void clearDownstreamConnections();             ///< 清除下游控制器的连接引用(防止悬空指针)
    void stopConnectionTimeout();                  ///< 停止连接超时定时器

    /** @brief 统一的连接断开清理流程: 停超时→缓存清空→断信号→移除连接→清下游 @param reason 断开原因(用于日志) */
    void teardownConnection(const QString& reason);

    /**
     * @brief 计算指数退避重连间隔
     *
     * 策略: actualInterval = baseInterval * 2^min(attempt, maxShift)，上限30秒
     * 例: base=3s → 3s→6s→12s→24s→30s→30s...
     *
     * @param attempt 当前重连次数(从1开始)
     * @return 本次重连后等待的间隔(毫秒)
     */
    int calcBackoffInterval(int attempt) const;

    ConnectionManager* m_connManager;              ///< 连接管理器(工厂)
    IConnection* m_currentConn = nullptr;          ///< 当前活跃连接实例
    SendController* m_sendController = nullptr;    ///< 发送控制器引用
    OtaManager* m_otaManager = nullptr;            ///< OTA 管理器引用
    RecordingController* m_recordingController = nullptr; ///< 录制控制器引用
    PortWatcher* m_portWatcher = nullptr;          ///< 端口热插拔监控器

    QTimer* m_pinoutPollTimer = nullptr;           ///< 信号线轮询定时器(200ms)
    PinoutSignals m_lastPinout;                    ///< 上次轮询的信号线状态(用于变化检测)

    QTimer m_connectionTimer;                      ///< 连接超时定时器(单次触发)
    static constexpr int kConnectionTimeoutMs = 5000; ///< 超时阈值 5 秒

    QTimer m_reconnectTimer;                       ///< 自动重连定时器(间隔触发)
    bool m_autoReconnectEnabled = false;           ///< 是否启用自动重连
    bool m_userInitiatedDisconnect = false;        ///< 用户主动断开标志(不触发自动重连)
    int m_reconnectMaxRetries = 0;                 ///< 最大重连次数(0=无限制)
    int m_reconnectAttemptCount = 0;               ///< 当前已重连次数
    int m_reconnectBaseIntervalMs = 3000;          ///< 重连基础间隔(毫秒)，实际间隔=baseInterval * 2^min(attempt,4)，上限30秒
    QVariantMap m_lastConnectParams;               ///< 上次连接参数(用于重连)
    ConnectionType m_lastConnectType = ConnectionType::Serial; ///< 上次连接类型
    QString m_connectedPortName;                   ///< 当前连接的串口名称

    QTimer m_healthTimer;                          ///< 连接健康检测定时器(5秒间隔)
    qint64 m_lastDataTimestamp = 0;                ///< 上次收到数据时的epoch毫秒时间戳(0=从未收到)
};

#endif // CONNECTIONCONTROLLER_H
