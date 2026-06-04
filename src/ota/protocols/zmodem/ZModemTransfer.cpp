/**
 * @file ZModemTransfer.cpp
 * @brief ZMODEM协议传输器实现
 *
 * 实现ZMODEM Sender端状态机: HEX帧解析(16位CRC校验)、BIN32帧构建、
 * 数据子帧发送(ZCRCG/ZCRCW)、超时重发、断点续传(ZRPOS偏移处理)
 */
#include "ota/protocols/zmodem/ZModemTransfer.h"
#include <QFile>
#include <QFileInfo>
#include <QTimer>
/** @brief 将状态枚举转换为可读字符串
 *  @param s ZMODEM状态枚举值
 *  @return 状态名称字符串 */
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
    default:                  return QStringLiteral("Unknown");
    }
}
/** @brief 构造函数，初始化ZMODEM传输器并设置默认超时10秒 */
ZModemTransfer::ZModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
    m_timeoutMs = 10000;
}
/** @brief 设置传输文件路径
 *  @param path 文件绝对路径 */
void ZModemTransfer::setFilePath(const QString& path) { m_filePath = path; }

/** @brief 安全写入: 检测write返回值，连接断开时立即终止传输
 *  @param data 待写入的数据
 *  @return true=成功写入, false=连接已断开(已设置Error状态) */
bool ZModemTransfer::writeChecked(const QByteArray& data)
{
    if (!m_conn) {
        m_zmodemState = State::Error;
        markError();
        ++m_errorCount;  ///< 统计: 连接错误
        emit transferError(tr("连接中断: 连接对象无效"));
        return false;
    }
    qint64 written = m_conn->write(data);
    if (written < 0) {
        m_zmodemState = State::Error;
        markError();
        ++m_errorCount;  ///< 统计: 写入错误
        emit transferError(
            tr("连接中断: 写入失败, 已传输 %1/%2 字节")
                .arg(m_bytesSent)
                .arg(m_fileData.size()));
        return false;
    }
    return true;
}

// ---- BaseTransfer钩子实现 ----
/** @brief 传输启动初始化，校验文件并读取到内存，发送ZRQINIT开始握手
 *  @return 初始化成功返回true，文件不存在或读取失败返回false */
bool ZModemTransfer::onStartInit()
{
    // 文件路径非空校验
    if (m_filePath.isEmpty()) {
        emit transferError(tr("未设置文件路径，请先调用 setFilePath()"));
        return false;
    }
    // 文件存在性校验
    QFileInfo fileInfo(m_filePath);
    if (!fileInfo.exists()) {
        emit transferError(tr("文件不存在: %1").arg(m_filePath));
        return false;
    }
    // 文件大小校验(最大16MB)
    if (fileInfo.size() > BaseTransfer::kMaxFileSize) {
        emit transferError(tr("文件过大: %1 (%2 字节, 上限 %3 字节)")
                               .arg(m_filePath).arg(fileInfo.size()).arg(BaseTransfer::kMaxFileSize));
        return false;
    }
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit transferError(tr("无法打开文件: %1").arg(m_filePath));
        return false;
    }
    m_fileData = file.readAll();
    if (m_fileData.size() != fileInfo.size()) {
        emit transferError(tr("读取文件失败"));
        return false;
    }
    file.close();
    // 空文件无法传输
    if (m_fileData.isEmpty()) {
        emit transferError(tr("文件为空，无法传输: %1").arg(m_filePath));
        return false;
    }
    m_bytesSent = 0;
    m_fileOffset = 0;
    m_senderCrc32 = 0;
    m_zmodemState = State::WaitingRinit;
    sendZRQINIT();
    m_timeoutTimer->start(m_timeoutMs);
    return true;
}

/** @brief 发送取消序列: 8个退格符+2个CAN字节，中断ZMODEM传输 */
void ZModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(8, 0x08));
        m_conn->write(QByteArray(2, static_cast<char>(0x18)));
    }
}

/** @brief 超时处理，根据当前状态重发对应帧(ZRQINIT/ZFILE/ZDATA/ZEOF/ZFIN) */
void ZModemTransfer::handleTimeout()
{
    QString curState = stateToString(m_zmodemState);
    switch (m_zmodemState) {
    case State::WaitingRinit:
        qWarning() << "ZModem: timeout in" << curState << "- retrying ZRQINIT, attempt" << m_retryCount;
        ++m_totalRetries;  ///< 统计: ZRQINIT重试
        sendZRQINIT();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingFile:
        qWarning() << "ZModem: timeout in" << curState << "- retrying ZFILE, attempt" << m_retryCount;
        ++m_totalRetries;  ///< 统计: ZFILE重试
        sendZFILE();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::WaitingZAck:
        qWarning() << "ZModem: timeout in" << curState << "- bytes sent:" << m_bytesSent;
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingFin:
        qWarning() << "ZModem: timeout in" << curState << "- retrying ZFIN, attempt" << m_retryCount;
        ++m_totalRetries;  ///< 统计: ZFIN重试
        sendZFIN();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingData:
        qWarning() << "ZModem: timeout in" << curState << "- offset:" << m_fileOffset << "bytes:" << m_bytesSent;
        ++m_totalRetries;  ///< 统计: 数据重发
        sendDataSubpackets();  // 重发数据子包（内部已启动定时器）
        break;
    case State::SendingEof:
        qWarning() << "ZModem: timeout in" << curState << "- retrying ZEOF";
        ++m_totalRetries;  ///< 统计: ZEOF重试
        sendZEOF();
        m_timeoutTimer->start(m_timeoutMs);  // 重启定时器等待ZRINIT响应
        break;
    default:
        qWarning() << "ZModem: unexpected timeout in state" << curState;
        break;
    }
}

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
            // 缓冲区过大(>4096)且无法解析时清空，防止垃圾数据堆积
            if (m_receiveBuffer.size() > 4096) {
                qWarning() << "ZModem: buffer overflow (>4096) in state"
                           << stateToString(m_zmodemState) << "- clearing";
                m_receiveBuffer.clear();
            }
            return;
        }
    }
}

// ---- 帧解析与发送流程方法已拆分至 ZModemTransferProtocol.cpp ----
// parseHexFrame / sendZRQINIT / sendZFILE / sendZDATA / sendDataSubpackets / sendZEOF / sendZFIN
/** @brief 设置ZMODEM状态机状态
 *  @param s 目标状态 */
void ZModemTransfer::setState(State s) { m_zmodemState = s; }

/** @brief 重置ZModem统计计数器(不影响传输状态) */
void ZModemTransfer::resetZmodemStatistics()
{
    m_totalBlocksSent = 0;
    m_totalRetries = 0;
    m_totalCrcErrors = 0;
    m_errorCount = 0;
}
