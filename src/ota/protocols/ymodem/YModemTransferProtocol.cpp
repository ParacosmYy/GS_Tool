/**
 * @file YModemTransferProtocol.cpp
 * @brief YMODEM协议数据包构建与发送方法
 *
 * 从YModemTransfer.cpp拆分，包含Block 0/数据块/EOT/最终Block 0的
 * 构建与发送方法，以及CRC16-CCITT数据包组装逻辑。
 * 协议状态处理方法见YModemTransferHandlers.cpp。
 */
#include "ota/protocols/ymodem/YModemTransfer.h"
#include <QFile>
#include <QFileInfo>

/** @brief 构建并发送Block 0(文件信息块)，包含文件名、大小、修改时间和权限 */
void YModemTransfer::sendBlock0()
{
    // 获取文件修改时间
    qint64 modTime = 0;
    if (m_fileIndex < m_filePaths.size()) {
        modTime = QFileInfo(m_filePaths[m_fileIndex])
                      .lastModified().toSecsSinceEpoch();
    }
    QByteArray block0 = buildBlock0(m_currentFileName, m_currentData.size(),
                                    modTime);
    QByteArray packet = buildBlock(0, block0);
    if (m_conn) {
        writeChecked(packet);
        ++m_totalBlocksSent;
    }
}

/** @brief 构建并发送数据块，不足128字节时用0x1A填充 */
void YModemTransfer::sendBlock()
{
    // 使用累计已发送字节数作为偏移(块序号1-255会回绕，无法从序号反推偏移)
    qint64 offset = m_bytesSent;
    int dataSize = qMin(static_cast<int>(m_currentData.size() - offset),
                        kBlockSize);
    if (dataSize <= 0) {
        m_ymodemState = State::SendingEOT;
        m_blockRetryCount = 0;
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
        return;
    }
    QByteArray blockData = m_currentData.mid(offset, dataSize);
    if (blockData.size() < kBlockSize) {
        blockData.append(QByteArray(kBlockSize - blockData.size(), 0x1A));
    }
    QByteArray packet = buildBlock(m_blockNumber, blockData);
    if (m_conn) {
        writeChecked(packet);
        ++m_totalBlocksSent;
    }
    /* 仅在首次发送该块时更新进度(NAK重传时不重复推进)
     * m_bytesSent指向当前块起始偏移，首次发送时sent > m_bytesSent
     * 重传时m_bytesSent已被ACK回调推进，sent == m_bytesSent */
    qint64 sent = qMin(offset + dataSize,
                        static_cast<qint64>(m_currentData.size()));
    if (sent > m_bytesSent) {
        m_totalBytesSent += (sent - m_bytesSent);
        m_bytesSent = sent;
    }
}

/** @brief 发送EOT(End of Transmission)字节通知接收方当前文件传输结束 */
void YModemTransfer::sendEOT()
{
    if (m_conn) {
        writeChecked(QByteArray(1, EOT));
    }
}

/** @brief 发送最终空Block 0，表示整个YMODEM传输会话结束 */
void YModemTransfer::sendFinalBlock0()
{
    // 空Block 0表示传输会话结束
    QByteArray emptyBlock0(kBlockSize, 0x00);
    QByteArray packet = buildBlock(0, emptyBlock0);
    if (m_conn) {
        writeChecked(packet);
    }
}

/** @brief 构建完整数据块包(SOH+块号+数据+CRC16)
 *  @param blockNum 块编号(0=文件信息块, 1-255=数据块)
 *  @param blockData 块数据载荷
 *  @return 完整的数据包字节数组 */
QByteArray YModemTransfer::buildBlock(int blockNum, const QByteArray& blockData)
{
    QByteArray packet;
    packet.append(SOH);
    char bn = static_cast<char>(blockNum & 0xFF);
    packet.append(bn);
    packet.append(static_cast<char>(~bn & 0xFF));
    packet.append(blockData);
    // YMODEM固定使用CRC16-CCITT
    quint16 crc = CRC::crc16Ccitt(blockData);
    packet.append(static_cast<char>((crc >> 8) & 0xFF));
    packet.append(static_cast<char>(crc & 0xFF));
    return packet;
}

/** @brief 构建Block 0文件信息块(文件名+大小+修改时间+权限)
 *  @param fileName 文件名
 *  @param fileSize 文件大小(字节)
 *  @param modTime 文件修改时间(Unix时间戳)
 *  @return 128字节的Block 0数据 */
QByteArray YModemTransfer::buildBlock0(const QString& fileName,
                                        qint64 fileSize, qint64 modTime)
{
    QByteArray block0;
    // 文件名(ASCII, null-terminated)
    block0.append(fileName.toUtf8());
    block0.append('\0');
    // 文件大小(ASCII十进制, null-terminated)
    block0.append(QString::number(fileSize).toUtf8());
    block0.append('\0');
    // 修改时间(Octal格式的Unix时间戳, null-terminated)
    block0.append(QString::number(modTime, 8).toUtf8());
    block0.append('\0');
    // 文件权限(简化为0o100644 = 普通文件, rw-r--r--)
    block0.append("100644");
    block0.append('\0');
    // 填充到128字节(不足用0x00填充，超过截断)
    if (block0.size() < kBlockSize) {
        block0.append(QByteArray(kBlockSize - block0.size(), 0x00));
    } else if (block0.size() > kBlockSize) {
        block0 = block0.left(kBlockSize);
    }
    return block0;
}
