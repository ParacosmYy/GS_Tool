/**
 * @file BaseTransfer.h
 * @brief OTA传输抽象基类 - 模板方法模式
 *
 * 子类实现4个纯虚钩子: onStartInit, sendCancelBytes, processReceivedData, handleTimeout
 * 设计模式: 模板方法(start/cancel驱动骨架)  协作: IConnection(通道)
 */
#ifndef BASE_TRANSFER_H
#define BASE_TRANSFER_H

#include <QObject>
#include <QTimer>
#include "connection/IConnection.h"

/**
 * @brief OTA传输抽象基类 - 模板方法模式
 *
 * 双状态机: 基类管理终止态(Idle/Done/Error)+全局重试上限, 子类管理协议详细状态
 * 生命周期: setConnection→start→dataReceived→processReceivedData→finishTransfer/cancel
 */
class BaseTransfer : public QObject {
    Q_OBJECT

public:
    /** @brief 通用传输状态(基类管理) */
    enum class TransferState {
        Idle,   ///< 空闲
        Active, ///< 传输中
        Done,   ///< 完成
        Error   ///< 出错
    };

    explicit BaseTransfer(QObject* parent = nullptr); ///< @param parent 父对象
    virtual ~BaseTransfer() = default;
    BaseTransfer(const BaseTransfer&) = delete;
    BaseTransfer& operator=(const BaseTransfer&) = delete;

    /** @brief 设置传输连接 @param conn 连接实例(外部管理生命周期) */
    void setConnection(IConnection* conn);

    /** @brief 开始传输(模板方法) @return true=已启动 */
    bool start();

    /** @brief 取消传输(模板方法) */
    void cancel();

    /** @brief 查询是否正在运行 @return true=传输中 */
    virtual bool isRunning() const;

signals:
    /** @brief 进度更新 @param percent 百分比 @param sent 已发送 @param total 总字节 */
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();                       ///< 传输成功完成
    void transferError(const QString& reason);     ///< @brief 传输出错 @param reason 原因(tr()已国际化)

protected:
    IConnection* m_conn = nullptr;      ///< 传输连接(外部注入,不拥有)
    QByteArray m_receiveBuffer;         ///< 接收缓冲区(子类消费)
    int m_retryCount = 0;               ///< 当前重试次数
    bool m_cancelled = false;           ///< 取消标志
    QTimer* m_timeoutTimer;             ///< 超时定时器
    int m_maxRetries = 10;              ///< 最大全局重试次数
    int m_timeoutMs = 5000;             ///< 单次超时(ms)
    static constexpr int kMaxReceiveBufferSize = 1024 * 1024; ///< 缓冲区上限(1MB)

    // === 纯虚钩子 ===
    /** @brief start()的协议特有初始化 @return true=成功 */
    virtual bool onStartInit() = 0;
    /** @brief cancel()的协议特有取消字节序列 */
    virtual void sendCancelBytes() = 0;
    /** @brief 处理接收数据(协议状态机核心) */
    virtual void processReceivedData() = 0;
    /** @brief 处理超时(子类只处理重发逻辑) */
    virtual void handleTimeout() = 0;

    // === 工具方法 ===
    void finishTransfer();              ///< 完成传输(Done+transferComplete)
    void markIdle();                    ///< 重置为Idle
    void markDone();                    ///< 设置为Done
    void markError();                   ///< 设置为Error

    bool isIdle() const { return m_transferState == TransferState::Idle; }
    bool isDone() const { return m_transferState == TransferState::Done; }
    bool isError() const { return m_transferState == TransferState::Error; }

private slots:
    /** @brief 连接数据就绪 @param data 原始字节 */
    void onConnectionReadyRead(const QByteArray& data);
    /** @brief 超时: 递增重试计数, 超限中止, 否则委托子类 */
    void onTimeout();

private:
    TransferState m_transferState = TransferState::Idle; ///< 基类传输状态
    void setTransferState(TransferState state);           ///< 内部状态设置
};

#endif // BASE_TRANSFER_H
