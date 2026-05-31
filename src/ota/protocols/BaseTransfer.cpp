#include "ota/protocols/BaseTransfer.h"

BaseTransfer::BaseTransfer(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &BaseTransfer::onTimeout);
}

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

bool BaseTransfer::start()
{
    if (!isIdle()) return false;
    if (!m_conn) {
        emit transferError("No connection set");
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
    return true;
}

void BaseTransfer::cancel()
{
    m_cancelled = true;
    if (m_conn && !isIdle()) {
        sendCancelBytes();
    }
    m_timeoutTimer->stop();
    markIdle();
    emit transferError("Transfer cancelled by user");
}

bool BaseTransfer::isRunning() const
{
    return !isIdle() && !isDone() && !isError();
}

void BaseTransfer::finishTransfer()
{
    markDone();
    emit transferComplete();
}

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
        emit transferError("Receive buffer overflow: connection may be malicious");
        return;
    }

    m_receiveBuffer.append(data);
    processReceivedData();
}

void BaseTransfer::onTimeout()
{
    if (isIdle()) return;

    m_retryCount++;
    if (m_retryCount > m_maxRetries) {
        sendCancelBytes();
        markError();
        emit transferError("Transfer timeout: max retries exceeded");
        return;
    }

    handleTimeout();
}

void BaseTransfer::setTransferState(TransferState state)
{
    m_transferState = state;
}

void BaseTransfer::markIdle()
{
    setTransferState(TransferState::Idle);
}

void BaseTransfer::markDone()
{
    setTransferState(TransferState::Done);
}

void BaseTransfer::markError()
{
    setTransferState(TransferState::Error);
}
