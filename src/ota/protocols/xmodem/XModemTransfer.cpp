/**
 * @file XModemTransfer.cpp
 * @brief XMODEM协议传输器实现 - 初始化、接收处理与基础接口
 *
 * XMODEM Sender端实现，包含构造/析构、模式设置、数据加载、
 * 传输初始化、接收缓冲区状态分发和取消帧发送。
 * 数据包构建与发送方法拆分至 XModemTransferProtocol.cpp。
 * 协议状态处理方法拆分至 XModemTransferHandlers.cpp。
 */

#include "ota/protocols/xmodem/XModemTransfer.h"
#include <QFile>
#include <QFileInfo>

/** @brief 构造函数，初始化XMODEM传输器基类 @param parent 父对象 */
XModemTransfer::XModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
}

/** @brief 设置XMODEM传输模式 @param mode 传输模式: Checksum/CRC/OneK */
void XModemTransfer::setMode(Mode mode)
{
    m_mode = mode;
}

/** @brief 设置传输文件路径 @param path 文件绝对路径 */
void XModemTransfer::setFilePath(const QString& path)
{
    m_filePath = path;
}

/** @brief 直接设置传输数据，优先于文件路径 @param data 待传输的原始字节数据 */
void XModemTransfer::setData(const QByteArray& data)
{
    m_data = data;
    m_filePath.clear();
}

/** @brief 获取当前传输速率 @return 传输速率，单位: 字节/秒 */
double XModemTransfer::transferRate() const
{
    return m_currentRate;
}

/** @brief 计算剩余传输时间 @return 预计剩余秒数，无法计算时返回-1 */
double XModemTransfer::etaSeconds() const
{
    if (m_currentRate <= 0.0 || m_data.isEmpty()) {
        return -1.0;
    }
    qint64 remaining = m_data.size() - m_bytesSent;
    if (remaining <= 0) {
        return 0.0;
    }
    return static_cast<double>(remaining) / m_currentRate;
}

// ---- BaseTransfer钩子实现 ----

/** @brief 传输启动初始化，加载文件数据并等待接收方启动信号 @return 初始化成功返回true，文件/连接错误返回false */
bool XModemTransfer::onStartInit()
{
    // 加载文件数据
    if (m_data.isEmpty() && !m_filePath.isEmpty()) {
        // 文件大小校验: 拒绝超过kMaxFileSize的文件，防止内存耗尽
        QFileInfo fileInfo(m_filePath);
        if (fileInfo.size() > BaseTransfer::kMaxFileSize) {
            qWarning() << "XModem: file too large:" << fileInfo.size()
                       << "bytes (max" << BaseTransfer::kMaxFileSize << "bytes)";
            emit transferError(tr("文件过大: %1 (%2 字节, 上限 %3 字节)")
                                   .arg(fileInfo.fileName())
                                   .arg(fileInfo.size())
                                   .arg(BaseTransfer::kMaxFileSize));
            return false;
        }

        QFile file(m_filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            emit transferError(tr("无法打开文件: %1").arg(m_filePath));
            return false;
        }
        m_data = file.readAll();
        if (m_data.size() != fileInfo.size()) {
            emit transferError(tr("读取文件失败"));
            return false;
        }
        file.close();
    }

    if (m_data.isEmpty()) {
        emit transferError(tr("无传输数据"));
        return false;
    }

    // 连接有效性检查: 防止空连接下启动传输
    if (!m_conn) {
        emit transferError(tr("传输启动失败: 连接未就绪"));
        return false;
    }

    // 重置传输状态
    m_blockNumber = 1;
    m_bytesSent = 0;
    m_blockRetryCount = 0;
    m_currentRate = 0.0;
    m_lastStatsBytes = 0;
    m_transferTimer.start();

    // 等待接收方发送启动信号(NAK=Checksum模式, C=CRC模式)
    m_xmodemState = State::WaitingForStart;
    m_timeoutTimer->start(m_timeoutMs * kStartTimeoutMultiplier);
    return true;
}

/** @brief 发送CAN取消字节，连续发送2个CAN通知接收方终止传输 */
void XModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(2, CAN));
    }
}

/** @brief 处理接收缓冲区数据，按字节逐个分发给对应状态处理方法 @note 状态处理方法实现见 XModemTransferHandlers.cpp */
void XModemTransfer::processReceivedData()
{
    int readIdx = 0;
    const int len = m_receiveBuffer.size();

    while (readIdx < len) {
        char ch = m_receiveBuffer.at(readIdx);
        readIdx++;

        if (m_cancelled) {
            m_receiveBuffer.remove(0, readIdx);
            return;
        }

        switch (m_xmodemState) {
        case State::Idle:
        case State::Done:
        case State::Error:
            m_receiveBuffer.remove(0, readIdx);
            return;
        case State::WaitingForStart:
            handleStateWaitingForStart(ch);
            break;
        case State::SendingBlock:
            handleStateSendingBlock(ch, readIdx);
            break;
        case State::SendingEOT:
            handleStateSendingEOT(ch, readIdx);
            break;
        default:
            qWarning() << "XModem: unknown state" << static_cast<int>(m_xmodemState);
            m_receiveBuffer.remove(0, readIdx);
            return;
        }

        // 处理函数可能导致early return, 重新检查
        if (m_xmodemState == State::Error || m_cancelled) {
            m_receiveBuffer.remove(0, readIdx);
            return;
        }
    }

    // 单次O(n)压缩，替代循环中每次O(n)的remove
    m_receiveBuffer.remove(0, readIdx);
}

// 数据包构建、发送、CRC校验和统计方法见 XModemTransferProtocol.cpp
