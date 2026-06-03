/**
 * @file BaseTransfer.cpp
 * @brief OTA传输基类实现 — 模板方法模式的核心骨架
 *
 * 提供超时管理、取消机制和传输状态控制。
 * 具体协议（XMODEM/YMODEM/ZMODEM）通过重写纯虚方法实现各自流程。
 */
#include "ota/protocols/base/BaseTransfer.h"

/** @brief 构造传输基类，创建单次超时定时器 @param parent 父对象 */
BaseTransfer::BaseTransfer(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &BaseTransfer::onTimeout);
}

/** @brief 注入连接并绑定dataReceived信号(旧连接自动断开) @param conn IConnection指针 */
void BaseTransfer::setConnection(IConnection* conn)
{
    if (m_conn) {
        disconnect(m_conn, nullptr, this, nullptr);
    }
    m_conn = conn;
    if (m_conn) {
        connect(m_conn, &IConnection::dataReceived,
                this, &BaseTransfer::onConnectionReadyRead);
    }
}

/** @brief 启动传输(检查空闲+连接→重置状态→委托子类onStartInit→激活) @return true=启动成功 */
bool BaseTransfer::start()
{
    if (!isIdle()) return false;
    if (!m_conn) {
        emit transferError(tr("无可用连接"));
        return false;
    }

    // 重置公共状态
    m_retryCount = 0;
    m_cancelled = false;
    m_receiveBuffer.clear();

    // 委托给子类做协议特有初始化
    if (!onStartInit()) {
        return false;
    }

    // 初始化成功，激活传输状态
    setTransferState(TransferState::Active);
    ++m_totalPacketsSent;  ///< 统计: 每次启动传输视为一次发送
    return true;
}

/** @brief 用户取消传输：发送CAN字节→停止定时器→标记空闲→发射错误信号 */
void BaseTransfer::cancel()
{
    m_cancelled = true;
    if (m_conn && !isIdle()) {
        sendCancelBytes();
    }
    m_timeoutTimer->stop();
    markIdle();
    emit transferError(tr("用户取消传输"));
}

/** @brief 查询传输是否正在进行(非空闲/非完成/非错误) @return true=正在传输 */
bool BaseTransfer::isRunning() const
{
    return !isIdle() && !isDone() && !isError();
}

/** @brief 传输成功完成：标记Done状态并发射transferComplete信号 */
void BaseTransfer::finishTransfer()
{
    markDone();
    emit transferComplete();
}

/** @brief 连接数据到达回调：终止态丢弃→溢出保护→缓冲→委托子类processReceivedData @param data 接收到的原始字节 */
void BaseTransfer::onConnectionReadyRead(const QByteArray& data)
{
    // 终止态不再处理数据，防止cancel/error后残留数据触发状态机
    if (isIdle() || isDone() || isError()) return;

    // 缓冲区溢出保护: 超过最大容量时中止传输，防止恶意数据耗尽内存
    if (m_receiveBuffer.size() + data.size() > kMaxReceiveBufferSize) {
        qWarning() << "BaseTransfer: receive buffer overflow, size:"
                   << m_receiveBuffer.size() << "+ incoming:" << data.size()
                   << "exceeds max:" << kMaxReceiveBufferSize;
        sendCancelBytes();
        markError();
        ++m_totalErrors;  ///< 统计: 缓冲区溢出错误递增
        emit transferError(tr("接收缓冲区溢出: 连接可能异常"));
        return;
    }

    m_receiveBuffer.append(data);
    ++m_totalPacketsReceived;  ///< 统计: 每次收到数据递增
    processReceivedData();
}

/** @brief 全局超时回调：超过最大重试次数则中止，否则委托子类handleTimeout重试 */
void BaseTransfer::onTimeout()
{
    if (isIdle()) return;

    /* 注意: m_retryCount仅作为全局安全阀，子类handleTimeout()
     * 负责在成功处理后将m_retryCount重置，避免跨块累积 */
    m_retryCount++;
    ++m_totalRetries;  ///< 统计: 每次超时重试递增
    if (m_retryCount > m_maxRetries) {
        sendCancelBytes();
        markError();
        ++m_totalErrors;  ///< 统计: 重试耗尽错误递增
        emit transferError(tr("传输超时: 全局重试次数耗尽 (%1次)").arg(m_maxRetries));
        return;
    }

    handleTimeout();
}

/** @brief 设置内部传输状态 @param state 目标TransferState */
void BaseTransfer::setTransferState(TransferState state)
{
    m_transferState = state;
}

/** @brief 标记传输为空闲状态(Idle) */
void BaseTransfer::markIdle()
{
    setTransferState(TransferState::Idle);
}

/** @brief 标记传输为完成状态(Done) */
void BaseTransfer::markDone()
{
    setTransferState(TransferState::Done);
}

/** @brief 标记传输为错误状态(Error) */
void BaseTransfer::markError()
{
    setTransferState(TransferState::Error);
}

// ── 统计计数器实现 ──

/** @brief 获取已发送数据包总数 @return 累计发送包数 */
quint64 BaseTransfer::totalPacketsSent() const
{
    return m_totalPacketsSent;
}

/** @brief 获取已接收数据包总数 @return 累计接收包数 */
quint64 BaseTransfer::totalPacketsReceived() const
{
    return m_totalPacketsReceived;
}

/** @brief 获取重试总次数 @return 累计重试次数 */
quint64 BaseTransfer::totalRetries() const
{
    return m_totalRetries;
}

/** @brief 获取传输错误总次数 @return 累计错误次数 */
quint64 BaseTransfer::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 重置传输统计计数器(不影响传输状态) */
void BaseTransfer::resetTransferStatistics()
{
    m_totalPacketsSent = 0;
    m_totalPacketsReceived = 0;
    m_totalRetries = 0;
    m_totalErrors = 0;
}
