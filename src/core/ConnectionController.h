/**
 * @file ConnectionController.h
 * @brief 连接控制器 - 管理串口/网络连接的完整生命周期
 *
 * 职责:
 *   1. 创建/断开串口连接（从 SerialConfigPanel 读取参数）
 *   2. 创建网络连接（TCP 客户端/服务端、UDP）
 *   3. 将连接实例同步给 SendController/OtaManager/RecordingController
 *   4. 转发连接状态变化和数据接收信号到 MainWindow 用于 UI 更新
 *   5. 连接超时检测：防止 open() 卡住或无响应
 *   6. 自动重连支持：意外断开时可选自动重连
 *
 * 设计模式:
 *   - 中介者模式: 作为连接层的中介，协调上下游模块
 *   - 观察者模式: 通过 Qt 信号/槽通知状态变化
 *
 * 协作关系:
 *   - ConnectionManager: 工厂，负责创建和销毁 IConnection 实例
 *   - SendController: 数据发送需要 IConnection 指针
 *   - OtaManager: OTA 传输需要 IConnection 指针
 *   - RecordingController: 录制需要感知连接/断开状态
 *   - MainWindow: 接收连接状态变化信号并更新 UI
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

/**
 * @brief 连接控制器 - 管理串口/网络连接的完整生命周期
 *
 * 作为连接层的中介者，协调 ConnectionManager（工厂）、SendController（发送）、
 * OtaManager（OTA）、RecordingController（录制）之间的连接实例传递。
 *
 * 核心流程:
 *   连接: connectSerial() → 工厂创建 → 配置参数 → 打开 → 注入下游控制器
 *   断开: disconnectCurrent() → 关闭端口 → 从工厂移除 → 清空下游引用
 *   超时: 连接开始后启动定时器，超时未完成则自动中断并报告失败
 *   重连: 意外断开时可选自动重连，通过 enableAutoReconnect() 开启
 */
class ConnectionController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造连接控制器
     * @param connMgr 连接管理器（工厂），负责创建具体连接实例
     * @param parent 父对象
     */
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~ConnectionController() override;

    /**
     * @brief 注入发送控制器引用
     * 在连接建立/断开时，将 IConnection 指针传递给 SendController
     * @param ctrl 发送控制器指针
     */
    void setSendController(SendController* ctrl);

    /**
     * @brief 注入 OTA 管理器引用
     * 在连接建立/断开时，将 IConnection 指针传递给 OtaManager
     * @param mgr OTA 管理器指针
     */
    void setOtaManager(OtaManager* mgr);

    /**
     * @brief 注入录制控制器引用
     * 在连接状态变化时，通知 RecordingController 启用/禁用录制
     * @param ctrl 录制控制器指针
     */
    void setRecordingController(RecordingController* ctrl);

    /**
     * @brief 创建并打开串口连接
     *
     * 完整流程:
     *   1. 关闭已有连接（避免资源泄漏）
     *   2. 通过工厂创建 SerialConnection 实例
     *   3. 使用 configure() 配置串口参数
     *   4. 连接 IConnection 信号到内部槽
     *   5. 启动连接超时定时器
     *   6. 尝试 open() 打开端口
     *   7. 成功后注入 IConnection 到 SendController/OtaManager
     *
     * @param serialParams 串口参数（portName/baudRate/dataBits/parity/stopBits/flowControl/dtr/rts）
     */
    void connectSerial(const QVariantMap& serialParams);

    /**
     * @brief 关闭当前活跃连接（串口或网络）并清除下游控制器的连接引用
     *
     * 流程: 关闭端口 → 从管理器移除 → 清空当前连接指针 → 清除下游引用
     * 注意: 不会触发自动重连（用户主动断开视为有意行为）
     */
    void disconnectCurrent();

    /**
     * @brief 创建并打开网络连接
     * 支持类型: TcpClient, TcpServer, Udp
     * @param type 连接类型枚举
     */
    void connectNetwork(ConnectionType type);

    /**
     * @brief 获取当前活跃的连接实例
     * @return 当前 IConnection 指针，无连接时返回 nullptr
     */
    IConnection* currentConnection() const;

    /**
     * @brief 运行时控制 DTR 线路信号（仅串口连接有效）
     * @param enabled true=拉高 DTR, false=拉低 DTR
     */
    void setDtr(bool enabled);

    /**
     * @brief 运行时控制 RTS 线路信号（仅串口连接有效）
     * @param enabled true=拉高 RTS, false=拉低 RTS
     */
    void setRts(bool enabled);

    /**
     * @brief 启用或禁用自动重连
     *
     * 启用后，当连接意外断开（非用户主动断开）时，会自动尝试重新连接。
     * 重连使用上一次成功的连接参数。
     *
     * @param enabled true=启用自动重连, false=禁用
     * @param intervalMs 重连间隔（毫秒），默认 3000ms
     */
    void enableAutoReconnect(bool enabled, int intervalMs = 3000);

    /**
     * @brief 查询自动重连是否已启用
     * @return true=已启用
     */
    bool isAutoReconnectEnabled() const;

signals:
    /**
     * @brief 连接状态变化信号
     * @param state 新状态（Connected/Disconnected/Connecting/Error）
     * @param connName 连接名称（如 "COM3"、"TCP 127.0.0.1:8080"）
     */
    void connectionStateChanged(ConnectionState state, const QString& connName);

    /**
     * @brief 接收数据信号
     * @param data 从连接读取的原始字节数据
     */
    void dataReceived(const QByteArray& data);

    /** @brief 请求刷新状态栏（数据收发后触发） */
    void statusBarUpdateRequested();

    /**
     * @brief 连接失败通知
     * @param title 错误对话框标题
     * @param message 错误详细信息
     */
    void connectionFailed(const QString& title, const QString& message);

private slots:
    /**
     * @brief 连接状态变化内部处理
     * 同步更新下游控制器的连接引用，然后转发状态变化信号
     * @param state 新连接状态
     */
    void onConnectionStateChanged(ConnectionState state);

    /**
     * @brief 接收数据内部处理
     * 转发数据到 MainWindow 并请求状态栏刷新
     * @param data 接收到的原始字节
     */
    void onDataReceived(const QByteArray& data);

    /**
     * @brief 连接超时处理
     *
     * 当 open() 调用后，如果在 kConnectionTimeoutMs 时间内
     * 状态未变为 Connected，则判定为超时，中断连接并报告失败。
     */
    void onConnectionTimeout();

    /**
     * @brief 自动重连定时器触发
     *
     * 检查是否仍在断开状态且未由用户主动断开，若是则尝试重新连接。
     * 重连失败时不立即重试，等待下一次定时器触发。
     */
    void onAutoReconnect();

private:
    /**
     * @brief 连接 IConnection 的信号到内部槽
     * @param conn 需要连接信号的 IConnection 实例
     */
    void connectSignals(IConnection* conn);

    /**
     * @brief 清除所有下游控制器的连接引用
     *
     * 在连接断开或发生错误时调用，防止下游控制器持有悬空的 IConnection 指针。
     */
    void clearDownstreamConnections();

    /**
     * @brief 停止连接超时定时器
     *
     * 在连接成功、连接失败或用户主动断开时调用。
     */
    void stopConnectionTimeout();

    /** @brief 连接管理器（工厂），负责创建和销毁 IConnection 实例 */
    ConnectionManager* m_connManager;

    /** @brief 当前活跃的连接实例，无连接时为 nullptr */
    IConnection* m_currentConn = nullptr;

    /** @brief 发送控制器引用，连接建立后注入 IConnection 指针 */
    SendController* m_sendController = nullptr;

    /** @brief OTA 管理器引用，连接建立后注入 IConnection 指针 */
    OtaManager* m_otaManager = nullptr;

    /** @brief 录制控制器引用，连接状态变化时通知启用/禁用 */
    RecordingController* m_recordingController = nullptr;

    // ---- 连接超时机制 ----

    /** @brief 连接超时定时器，防止 open() 无响应 */
    QTimer m_connectionTimer;

    /** @brief 连接超时时间（毫秒），默认 5 秒 */
    static constexpr int kConnectionTimeoutMs = 5000;

    // ---- 自动重连机制 ----

    /** @brief 自动重连定时器 */
    QTimer m_reconnectTimer;

    /** @brief 自动重连是否启用 */
    bool m_autoReconnectEnabled = false;

    /** @brief 标记是否由用户主动断开（用户主动断开不触发自动重连） */
    bool m_userInitiatedDisconnect = false;

    /** @brief 上一次成功的连接参数，用于自动重连 */
    QVariantMap m_lastConnectParams;

    /** @brief 上一次连接类型（Serial 或网络），用于判断重连使用哪个方法 */
    ConnectionType m_lastConnectType = ConnectionType::Serial;
};

#endif // CONNECTIONCONTROLLER_H
