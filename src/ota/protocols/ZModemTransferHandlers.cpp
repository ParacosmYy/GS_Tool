/**
 * @file ZModemTransferHandlers.cpp
 * @brief ZMODEM协议状态处理器 - 协议状态机的各状态处理方法
 *
 * 从ZModemTransfer.cpp拆分，包含6个状态处理方法。
 * 由processReceivedData()根据协议状态分发调用。
 */
#include "ota/protocols/ZModemTransfer.h"
#include "utils/CRC.h"

// ---- 状态处理方法 ----
/** @brief 处理WaitingRinit状态: 收到ZRINIT后发送ZFILE开始文件传输
 *  @param type 解析到的帧类型 */
void ZModemTransfer::handleStateWaitingRinit(int type)
{
    if (type == ZRINIT) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_zmodemState = State::SendingFile;
        sendZFILE();
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in WaitingRinit";
    }
}
/** @brief 处理SendingFile状态: 解析ZRPOS断点续传/ZSKIP跳过/ZRINIT重发
 *  @param type 解析到的帧类型
 *  @param headerData 帧头数据(含ZRPOS偏移量) */
void ZModemTransfer::handleStateSendingFile(int type, const QByteArray& headerData){
    if (type == ZRPOS) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        if (headerData.size() >= 4) {
            m_fileOffset = 0;
            for (int i = 3; i >= 0; --i)
                m_fileOffset = (m_fileOffset << 8) | static_cast<quint8>(headerData[i]);
        }
        qWarning() << "ZModem: ZRPOS resume at offset" << m_fileOffset;
        m_bytesSent = m_fileOffset;
        m_zmodemState = State::SendingData;
        sendDataSubpackets();
    } else if (type == ZSKIP) {
        m_timeoutTimer->stop();
        qWarning() << "ZModem: receiver skipped file";
        m_zmodemState = State::SendingFin;
        sendZFIN();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (type == ZRINIT) {
        m_timeoutTimer->stop();
        sendZFILE();
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingFile";
    }
}
/** @brief 处理SendingData状态: 解析ZRPOS重传请求/ZACK确认，超过重试上限则取消
 *  @param type 解析到的帧类型
 *  @param headerData 帧头数据(含ZRPOS偏移量) */
void ZModemTransfer::handleStateSendingData(int type, const QByteArray& headerData)
{
    if (type == ZRPOS) {
        m_timeoutTimer->stop();
        m_retryCount++;
        if (m_retryCount > m_maxRetries) {
            sendCancelBytes();
            m_zmodemState = State::Error;
            emit transferError(
                tr("重传请求次数过多 (已重试 %1 次, 上限 %2 次), "
                   "当前偏移: %3 字节, 文件大小: %4 字节")
                    .arg(m_retryCount).arg(m_maxRetries)
                    .arg(m_fileOffset).arg(m_fileData.size()));
            return;
        }
        if (headerData.size() >= 4) {
            m_fileOffset = 0;
            for (int i = 3; i >= 0; --i)
                m_fileOffset = (m_fileOffset << 8) | static_cast<quint8>(headerData[i]);
        }
        qWarning() << "ZModem: ZRPOS retransmit at offset" << m_fileOffset
                   << "retry" << m_retryCount << "/" << m_maxRetries;
        m_bytesSent = m_fileOffset;
        sendDataSubpackets();
    } else if (type == ZACK) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingData, offset:" << m_fileOffset;
    }
}
/** @brief 处理WaitingZAck状态: 收到ZACK/ZRPOS后发送ZEOF结束文件数据传输
 *  @param type 解析到的帧类型 */
void ZModemTransfer::handleStateWaitingZAck(int type)
{
    if (type == ZACK || type == ZRPOS) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_zmodemState = State::SendingEof;
        sendZEOF();
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in WaitingZAck, bytes:" << m_bytesSent;
    }
}
/** @brief 处理SendingEof状态: 收到ZRINIT/ZSKIP后发送ZFIN结束会话
 *  @param type 解析到的帧类型 */
void ZModemTransfer::handleStateSendingEof(int type)
{
    if (type == ZRINIT || type == ZSKIP) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_zmodemState = State::SendingFin;
        sendZFIN();
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingEof";
    }
}
/** @brief 处理SendingFin状态: 收到ZFIN后发送"OO"结束序列并完成传输
 *  @param type 解析到的帧类型 */
void ZModemTransfer::handleStateSendingFin(int type)
{
    if (type == ZFIN) {
        m_timeoutTimer->stop();
        if (m_conn) m_conn->write(QByteArray("OO"));
        emit progress(100, m_fileData.size(), m_fileData.size());
        finishTransfer();
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingFin";
    }
}
