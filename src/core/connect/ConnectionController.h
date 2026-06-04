/** @file ConnectionController.h @brief 连接控制器 - 管理串口/网络连接的完整生命周期。职责: 创建/断开/超时/自动重连/热插拔。中介者+观察者模式 */

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

/** @brief 连接控制器 - 连接层中介者。核心流程: connectSerial()->工厂创建->配置->打开->DTR/RTS->注入下游。断开->关闭->移除->清空下游。超时守护+自动重连 */
class ConnectionController : public QObject {
    Q_OBJECT

public:
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr); ///< 构造
    ~ConnectionController() override;       ///< 析构(停止定时器和PortWatcher)
    void setSendController(SendController* ctrl); ///< 注入发送控制器
    void setOtaManager(OtaManager* mgr);    ///< 注入OTA管理器
    void setRecordingController(RecordingController* ctrl); ///< 注入录制控制器
    void connectSerial(const QVariantMap& serialParams); ///< 创建并打开串口连接
    void disconnectCurrent();               ///< 关闭当前连接(用户主动, 不触发重连)
    void connectNetwork(ConnectionType type); ///< 创建网络连接(默认参数)
    void connectNetwork(ConnectionType type, const QVariantMap& params); ///< 创建网络连接(指定参数)
    IConnection* currentConnection() const; ///< 获取当前活跃连接
    void setDtr(bool enabled);              ///< 控制DTR线路信号
    void setRts(bool enabled);              ///< 控制RTS线路信号
    void sendBreak(int duration = 100);     ///< 发送Break信号(STM32/ESP32进Bootloader)
    void enableAutoReconnect(bool enabled, int intervalMs = 3000, int maxRetries = 0); ///< 启用/禁用自动重连
    bool isAutoReconnectEnabled() const;    ///< 查询自动重连是否启用
    PortWatcher* portWatcher() const;       ///< 获取端口热插拔监控器

    // ---- 连接统计 ----
    quint64 totalConnections() const;        ///< 累计成功连接次数
    quint64 totalDisconnections() const;     ///< 累计断开连接次数
    quint64 totalReconnects() const;         ///< 累计自动重连次数
    quint64 errorCount() const;              ///< 累计连接错误次数
    quint64 totalDataSent() const;           ///< 累计发送数据字节数
    quint64 totalDataReceived() const;       ///< 累计接收数据字节数
    void resetConnectionStatistics();        ///< 重置连接统计计数器

signals:
    void connectionStateChanged(ConnectionState state, const QString& connName); ///< 连接状态变化
    void dataReceived(const QByteArray& data); ///< 接收到数据(转发自IConnection)
    void statusBarUpdateRequested();          ///< 请求状态栏刷新
    void connectionFailed(const QString& title, const QString& message); ///< 连接失败(带标题和详情)
    void portAdded(const QString& portName);  ///< 检测到新端口接入
    void connectionSucceeded(const QString& portName); ///< 连接成功(Toast)
    void connectionDisconnected(const QString& portName); ///< 连接断开(Toast)
    void connectionError(const QString& portName, const QString& error); ///< 连接错误(Toast)
    void reconnectAttempt(int attempt, int maxRetries); ///< 自动重连尝试
    void reconnectProgress(int attempt, int maxRetries, int nextIntervalMs); ///< 重连进度(含退避间隔)
    void reconnectSucceeded(const QString& connName); ///< 自动重连成功
    void reconnectFailed(const QString& reason); ///< 自动重连失败(达最大次数)
    void connectionHealth(bool alive, qint64 lastDataAgeMs); ///< 连接健康状态(每5秒)
    void pinoutSignalsChanged(const PinoutSignals& pinout); ///< 信号线状态变化(200ms轮询)
    void errorCountersUpdated(int framingErrors, int parityErrors, int overrunErrors); ///< 通信错误计数

private slots:
    void onConnectionStateChanged(ConnectionState state); ///< 底层连接状态变化处理
    void onDataReceived(const QByteArray& data); ///< 转发接收数据
    void onConnectionTimeout();              ///< 连接超时处理
    void onAutoReconnect();                   ///< 自动重连定时器触发
    void onPortRemoved(const QString& portName); ///< 端口拔出处理
    void onPortAdded(const QString& portName); ///< 端口接入处理

private:
    void connectSignals(IConnection* conn);  ///< 连接IConnection信号到内部槽
    void clearDownstreamConnections();        ///< 清除下游连接引用(防悬空指针)
    void stopConnectionTimeout();             ///< 停止连接超时定时器
    void teardownConnection(const QString& reason); ///< 统一断开清理
    int calcBackoffInterval(int attempt) const; ///< 指数退避重连间隔(base*2^min(attempt,maxShift), 上限30s)

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
