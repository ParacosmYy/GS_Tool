#ifndef BASE_TRANSFER_H
#define BASE_TRANSFER_H

#include <QObject>
#include <QTimer>
#include "connection/IConnection.h"

// OTA传输抽象基类 - 模板方法模式
// 封装XMODEM/YMODEM/ZMODEM共享的基础设施:
// 连接管理、超时重试、取消机制、接收缓冲区、状态判断
// 子类只需实现4个纯虚钩子: onStartInit/sendCancelBytes/processReceivedData/handleTimeout
class BaseTransfer : public QObject {
    Q_OBJECT

public:
    // 通用传输状态(双状态机策略: 基类管理终止态，子类管理协议详细状态)
    enum class TransferState {
        Idle,
        Active,
        Done,
        Error
    };

    explicit BaseTransfer(QObject* parent = nullptr);
    virtual ~BaseTransfer() = default;

    BaseTransfer(const BaseTransfer&) = delete;
    BaseTransfer& operator=(const BaseTransfer&) = delete;

    // === 公共接口(模板方法，子类不覆写) ===

    // 设置传输连接(串口/TCP/UDP)
    void setConnection(IConnection* conn);

    // 开始传输(模板方法)
    // 骨架: 状态检查 -> 连接检查 -> 公共重置 -> 子类初始化 -> 激活
    bool start();

    // 取消传输(模板方法)
    // 骨架: 置标志 -> 子类取消字节 -> 停定时器 -> 重置状态 -> 发错误信号
    void cancel();

    // 是否正在运行
    virtual bool isRunning() const;

signals:
    // 三个协议完全相同的信号，提升到基类
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();
    void transferError(const QString& reason);

protected:
    // === 子类可访问的公共资源 ===
    IConnection* m_conn = nullptr;
    QByteArray m_receiveBuffer;
    int m_retryCount = 0;
    bool m_cancelled = false;
    QTimer* m_timeoutTimer;

    // 超时和重试常量(子类可在构造函数中修改)
    int m_maxRetries = 10;
    int m_timeoutMs = 5000;

    // === 纯虚钩子(子类必须实现) ===

    // start()的协议特有初始化阶段
    // 负责加载文件、重置协议计数器、设置初始状态、发送首帧
    virtual bool onStartInit() = 0;

    // cancel()的协议特有取消字节(XModem: 2xCAN, ZModem: 8xBS+2xCAN)
    virtual void sendCancelBytes() = 0;

    // 处理接收数据(协议状态机核心)
    virtual void processReceivedData() = 0;

    // 处理超时(基类已处理重试计数和上限，子类只处理重发逻辑)
    virtual void handleTimeout() = 0;

    // === 基类提供的工具方法 ===

    // 完成传输(设置Done + 发transferComplete)
    // 子类应在调用前自行emit progress(100, ...)
    void finishTransfer();

    // 子类状态控制(双状态机同步)
    void markIdle();
    void markDone();
    void markError();

    bool isIdle() const { return m_transferState == TransferState::Idle; }
    bool isDone() const { return m_transferState == TransferState::Done; }
    bool isError() const { return m_transferState == TransferState::Error; }

private slots:
    void onConnectionReadyRead(const QByteArray& data);
    void onTimeout();

private:
    TransferState m_transferState = TransferState::Idle;
    void setTransferState(TransferState state);
};

#endif // BASE_TRANSFER_H
