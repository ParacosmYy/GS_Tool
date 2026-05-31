#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include <QVariantMap>
#include "connection/IConnection.h"
#include "core/ConnectionManager.h"

class SendController;
class OtaManager;
class RecordingController;

/**
 * @brief 连接控制器 - 管理串口/网络连接的完整生命周期
 *
 * 职责:
 *   1. 创建/断开串口连接（从 SerialConfigPanel 读取参数）
 *   2. 创建网络连接（TCP 客户端/服务端、UDP）
 *   3. 将连接实例同步给 SendController/OtaManager/RecordingController
 *   4. 转发连接状态变化和数据接收信号到 MainWindow 用于 UI 更新
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
class ConnectionController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造连接控制器
     * @param connMgr 连接管理器（工厂），负责创建具体连接实例
     * @param parent 父对象
     */
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr);

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
     * 流程: 关闭已有连接 → 工厂创建 SerialConnection → 配置参数 → 打开 → 注入到下游控制器
     * @param serialParams 串口参数（portName/baudRate/dataBits/parity/stopBits/flowControl/dtr/rts）
     */
    void connectSerial(const QVariantMap& serialParams);

    /** @brief 关闭当前串口连接并清除下游控制器的连接引用 */
    void disconnectSerial();

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

private:
    /**
     * @brief 连接 IConnection 的信号到内部槽
     * @param conn 需要连接信号的 IConnection 实例
     */
    void connectSignals(IConnection* conn);

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
};

#endif // CONNECTIONCONTROLLER_H
