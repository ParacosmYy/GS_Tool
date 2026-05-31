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
 * @brief 连接控制器 - 作为连接层的中介者，协调各模块的连接实例传递和状态同步
 *
 * 核心流程:
 *   连接: connectSerial() → 工厂创建 → 配置 → 打开 → DTR/RTS → 注入下游
 *   断开: disconnectCurrent() → 关闭 → 移除 → 清空下游引用
 *   超时: 定时器守护，超时自动中断并报告失败
 *   重连: 意外断开时可选自动重连
 *   热插拔: PortWatcher 检测端口拔出 → 自动断开
 */
class ConnectionController : public QObject {
    Q_OBJECT

public:
    /** @brief 构造，初始化超时定时器、重连定时器和 PortWatcher */
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr);
    ~ConnectionController() override;

    /** @brief 注入发送控制器，连接建立/断开时传递 IConnection */
    void setSendController(SendController* ctrl);
    /** @brief 注入 OTA 管理器，连接建立/断开时传递 IConnection */
    void setOtaManager(OtaManager* mgr);
    /** @brief 注入录制控制器，连接状态变化时通知启用/禁用 */
    void setRecordingController(RecordingController* ctrl);

    /**
     * @brief 创建并打开串口连接
     * @param serialParams 串口参数(portName/baudRate/dataBits/parity/stopBits/flowControl/dtr/rts)
     */
    void connectSerial(const QVariantMap& serialParams);

    /** @brief 关闭当前连接并清除下游引用，不触发自动重连 */
    void disconnectCurrent();

    /** @brief 创建网络连接(TcpClient/TcpServer/Udp) */
    void connectNetwork(ConnectionType type);

    /** @brief 获取当前活跃连接实例，无连接时返回 nullptr */
    IConnection* currentConnection() const;

    /** @brief 运行时控制 DTR 线路信号(仅串口有效) */
    void setDtr(bool enabled);
    /** @brief 运行时控制 RTS 线路信号(仅串口有效) */
    void setRts(bool enabled);

    /** @brief 启用/禁用自动重连，默认间隔 3000ms */
    void enableAutoReconnect(bool enabled, int intervalMs = 3000);
    /** @brief 查询自动重连是否已启用 */
    bool isAutoReconnectEnabled() const;

    /** @brief 获取 PortWatcher 实例(所有权归本控制器) */
    PortWatcher* portWatcher() const;

signals:
    /** @brief 连接状态变化(Connected/Disconnected/Connecting/Error) */
    void connectionStateChanged(ConnectionState state, const QString& connName);
    /** @brief 接收数据 */
    void dataReceived(const QByteArray& data);
    /** @brief 请求刷新状态栏 */
    void statusBarUpdateRequested();
    /** @brief 连接失败通知(title=对话框标题, message=详细信息) */
    void connectionFailed(const QString& title, const QString& message);
    /** @brief 检测到新串口设备接入(不自动连接，仅通知) */
    void portAdded(const QString& portName);

private slots:
    void onConnectionStateChanged(ConnectionState state);  ///< 内部状态处理
    void onDataReceived(const QByteArray& data);           ///< 内部数据转发
    void onConnectionTimeout();                            ///< 连接超时处理
    void onAutoReconnect();                                ///< 自动重连定时触发
    void onPortRemoved(const QString& portName);           ///< 端口拔出处理
    void onPortAdded(const QString& portName);             ///< 端口接入转发

private:
    void connectSignals(IConnection* conn);       ///< 连接 IConnection 信号到内部槽
    void clearDownstreamConnections();             ///< 清除下游控制器的连接引用
    void stopConnectionTimeout();                  ///< 停止连接超时定时器

    // ---- 成员变量 ----
    ConnectionManager* m_connManager;              ///< 连接管理器(工厂)
    IConnection* m_currentConn = nullptr;          ///< 当前活跃连接
    SendController* m_sendController = nullptr;    ///< 发送控制器引用
    OtaManager* m_otaManager = nullptr;            ///< OTA 管理器引用
    RecordingController* m_recordingController = nullptr; ///< 录制控制器引用
    PortWatcher* m_portWatcher = nullptr;          ///< 热插拔检测器

    QTimer m_connectionTimer;                      ///< 连接超时定时器
    static constexpr int kConnectionTimeoutMs = 5000; ///< 超时阈值 5s

    QTimer m_reconnectTimer;                       ///< 自动重连定时器
    bool m_autoReconnectEnabled = false;           ///< 自动重连开关
    bool m_userInitiatedDisconnect = false;        ///< 用户主动断开标记
    QVariantMap m_lastConnectParams;               ///< 上次连接参数
    ConnectionType m_lastConnectType = ConnectionType::Serial; ///< 上次连接类型
    QString m_connectedPortName;                   ///< 当前连接的串口名
};

#endif // CONNECTIONCONTROLLER_H
