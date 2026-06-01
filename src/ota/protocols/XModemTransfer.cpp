/**
 * @file XModemTransfer.cpp
 * @brief XMODEM协议传输器实现
 *
 * XMODEM Sender端实现，包含初始化、数据发送、块构建和速率统计。
 * 状态处理方法拆分至 XModemTransferHandlers.cpp。
 *
 * 功能:
 *   - 传输速率实时计算与ETA显示
 *   - 每块独立重试计数(最多10次)
 *   - CAN取消帧发送
 *   - 完善的错误状态转换
 */

#include "ota/protocols/XModemTransfer.h"
#include <QFile>
#include <QFileInfo>

/** @brief 构造函数，初始化XMODEM传输器基类 */
XModemTransfer::XModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
}

/** @brief 设置XMODEM传输模式
 *  @param mode 传输模式: Checksum/CRC/OneK */
void XModemTransfer::setMode(Mode mode)
{
    m_mode = mode;
}

/** @brief 设置传输文件路径
 *  @param path 文件绝对路径 */
void XModemTransfer::setFilePath(const QString& path)
{
    m_filePath = path;
}

/** @brief 直接设置传输数据，优先于文件路径
 *  @param data 待传输的原始字节数据 */
void XModemTransfer::setData(const QByteArray& data)
{
    m_data = data;
    m_filePath.clear();
}

/** @brief 获取当前传输速率
 *  @return 传输速率，单位: 字节/秒 */
double XModemTransfer::transferRate() const
{
    return m_currentRate;
}

/** @brief 计算剩余传输时间
 *  @return 预计剩余秒数，无法计算时返回-1 */
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

/** @brief 传输启动初始化，加载文件数据并等待接收方启动信号
 *  @return 初始化成功返回true，文件/连接错误返回false */
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

/** @brief 处理接收缓冲区数据，按字节逐个分发给对应状态处理方法
 *  状态处理方法实现见 XModemTransferHandlers.cpp */
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

// ---- 数据发送 ----

/** @brief 构建并发送一个数据块，不足块大小时用0x1A填充 */
void XModemTransfer::sendBlock()
{
    int bs = blockSize();
    // 使用累计已发送字节数作为偏移，而非从块号反算。
    // XModem 块号在 1-255 间循环，不能用于计算文件偏移。
    qint64 offset = m_bytesSent;
    int dataSize = qMin(static_cast<int>(m_data.size() - offset), bs);

    if (dataSize <= 0) {
        // 数据已全部发送，切换到EOT阶段
        m_xmodemState = State::SendingEOT;
        m_blockRetryCount = 0;
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
        return;
    }

    // 提取块数据，不足块大小时用0x1A(Ctrl+Z)填充
    QByteArray blockData = m_data.mid(offset, dataSize);
    if (blockData.size() < bs) {
        blockData.append(QByteArray(bs - blockData.size(), 0x1A));
    }

    QByteArray packet = buildBlock(m_blockNumber, blockData);
    if (m_conn) {
        qint64 written = m_conn->write(packet);
        // 检测连接断开: write返回-1表示连接已不可用
        if (written < 0) {
            m_xmodemState = State::Error;
            markError();
            emit transferError(
                tr("连接中断: 写入失败, 已传输 %1/%2 字节 (块 %3)")
                    .arg(m_bytesSent)
                    .arg(m_data.size())
                    .arg(m_blockNumber));
            return;
        }
    }
    // 注意: m_bytesSent 不在sendBlock中推进，而是在ACK确认后推进，
    // 确保NAK重传时发送相同的块（防止跳块导致数据损坏）
}

/** @brief 发送EOT(End of Transmission)字节通知接收方传输结束 */
void XModemTransfer::sendEOT()
{
    if (m_conn) {
        qint64 written = m_conn->write(QByteArray(1, EOT));
        // 检测连接断开
        if (written < 0) {
            m_xmodemState = State::Error;
            markError();
            emit transferError(
                tr("连接中断: EOT发送失败, 已传输 %1/%2 字节")
                    .arg(m_bytesSent).arg(m_data.size()));
        }
    }
}

/** @brief 构建完整数据块包(帧头+块号+数据+校验)
 *  @param blockNum 块编号(1-255循环)
 *  @param blockData 块数据载荷
 *  @return 完整的数据包字节数组 */
QByteArray XModemTransfer::buildBlock(int blockNum, const QByteArray& blockData)
{
    QByteArray packet;

    // 头部: SOH(128B)或STX(1024B)
    packet.append((m_mode == OneK) ? STX : SOH);

    // 块号: blockNum(1-255循环) + 反码
    char bn = static_cast<char>(blockNum & 0xFF);
    packet.append(bn);
    packet.append(static_cast<char>(~bn & 0xFF));

    // 数据载荷
    packet.append(blockData);

    // 校验: Checksum模式用算术和，CRC/1K模式用CRC16
    if (m_mode == Checksum) {
        packet.append(static_cast<char>(CRC::checksum(blockData)));
    } else {
        quint16 crc = xmodemCrc(blockData);
        packet.append(static_cast<char>((crc >> 8) & 0xFF));
        packet.append(static_cast<char>(crc & 0xFF));
    }

    return packet;
}

/** @brief 计算XMODEM CRC16校验值
 *  @param data 待校验数据
 *  @return CRC16校验值 */
quint16 XModemTransfer::xmodemCrc(const QByteArray& data)
{
    return CRC::crc16Xmodem(data);
}

// ---- 速率统计 ----

/** @brief 更新传输速率统计并发射transferStats信号 */
void XModemTransfer::updateTransferStats()
{
    qint64 elapsedMs = m_transferTimer.elapsed();
    if (elapsedMs <= 0) return;

    // 基于总传输量计算平均速率
    qint64 totalSent = m_bytesSent;
    double elapsedSec = static_cast<double>(elapsedMs) / 1000.0;

    if (elapsedSec > 0.0) {
        m_currentRate = static_cast<double>(totalSent) / elapsedSec;
    }

    // 计算ETA并发射统计信号
    double eta = etaSeconds();
    emit transferStats(m_currentRate, eta);

    m_lastStatsBytes = m_bytesSent;
}

/** @brief 设置XMODEM状态机状态
 *  @param newState 目标状态 */
void XModemTransfer::setState(State newState)
{
    m_xmodemState = newState;
}
