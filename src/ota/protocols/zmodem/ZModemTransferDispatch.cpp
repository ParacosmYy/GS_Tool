/**
 * @file ZModemTransferDispatch.cpp
 * @brief ZMODEM协议 - 接收数据分发、连接写入与状态工具方法
 *
 * 从ZModemTransferHandlers.cpp拆分而来，包含:
 *   - processReceivedData(): 接收缓冲区数据解析与状态分发
 *   - writeChecked(): 安全连接写入(含错误检测)
 *   - stateToString(): 状态枚举转可读字符串
 *   - setState(): 状态设置接口
 *
 * 状态处理方法见ZModemTransferHandlers.cpp。
 * 帧构建与工具方法见ZModemTransferFrames.cpp。
 */

#include "ota/protocols/zmodem/ZModemTransfer.h"

/** @brief 处理接收缓冲区数据，解析HEX帧并分发给对应状态处理方法 */
void ZModemTransfer::processReceivedData()
{
    while (!m_receiveBuffer.isEmpty()) {
        if (m_cancelled) return;
        int padIdx = m_receiveBuffer.indexOf(ZPAD);
        if (padIdx < 0) { m_receiveBuffer.clear(); return; }
        if (padIdx > 0) m_receiveBuffer.remove(0, padIdx);
        int type = -1;
        QByteArray headerData;
        if (parseHexFrame(m_receiveBuffer, type, headerData)) {
            switch (m_zmodemState) {
            case State::Idle:
            case State::Done:
            case State::Error:
                return;
            case State::WaitingRinit:
                handleStateWaitingRinit(type);
                break;
            case State::SendingFile:
                handleStateSendingFile(type, headerData);
                break;
            case State::SendingData:
                handleStateSendingData(type, headerData);
                if (m_zmodemState == State::Error) return;
                break;
            case State::WaitingZAck:
                handleStateWaitingZAck(type);
                break;
            case State::SendingEof:
                handleStateSendingEof(type);
                break;
            case State::SendingFin:
                handleStateSendingFin(type);
                break;
            default:
                qWarning() << "ZModem: unknown state" << static_cast<int>(m_zmodemState);
                return;
            }
        } else {
            if (m_receiveBuffer.size() > 4096) {
                qWarning() << "ZModem: buffer overflow (>4096) in state"
                           << stateToString(m_zmodemState) << "- clearing";
                m_receiveBuffer.clear();
            }
            return;
        }
    }
}

/** @brief 设置ZMODEM状态机状态 @param s 目标状态 */
void ZModemTransfer::setState(State s) { m_zmodemState = s; }

/** @brief 将连接写入数据并检查错误 @param data 待写入数据 @return true成功 false失败 */
bool ZModemTransfer::writeChecked(const QByteArray& data)
{
    if (!m_conn) {
        m_zmodemState = State::Error;
        markError();
        ++m_errorCount;
        emit transferError(tr("连接中断: 连接对象无效"));
        return false;
    }
    qint64 written = m_conn->write(data);
    if (written < 0) {
        m_zmodemState = State::Error;
        markError();
        ++m_errorCount;
        emit transferError(
            tr("连接中断: 写入失败, 已传输 %1/%2 字节")
                .arg(m_bytesSent)
                .arg(m_fileData.size()));
        return false;
    }
    return true;
}

/** @brief 将状态枚举转为可读字符串 @param s 状态枚举值 @return 状态名称字符串 */
QString ZModemTransfer::stateToString(State s)
{
    switch (s) {
    case State::Idle:         return QStringLiteral("Idle");
    case State::WaitingRinit: return QStringLiteral("WaitingRinit");
    case State::SendingFile:  return QStringLiteral("SendingFile");
    case State::SendingData:  return QStringLiteral("SendingData");
    case State::WaitingZAck:  return QStringLiteral("WaitingZAck");
    case State::SendingEof:   return QStringLiteral("SendingEof");
    case State::SendingFin:   return QStringLiteral("SendingFin");
    case State::Done:         return QStringLiteral("Done");
    case State::Error:        return QStringLiteral("Error");
    }
    return QStringLiteral("Unknown");
}

/** @brief 重置ZModem统计计数器(不影响传输状态) */
void ZModemTransfer::resetZmodemStatistics()
{
    m_totalBlocksSent = 0;
    m_totalRetries = 0;
    m_totalCrcErrors = 0;
    m_errorCount = 0;
    m_totalTimeouts = 0;
    m_totalZrposReceived = 0;
    m_totalZdataFrames = 0;
    m_totalZfileSent = 0;
    m_totalZfinSent = 0;
}
