/**
 * @file ZModemTransfer.cpp
 * @brief ZMODEM协议传输器 - 核心生命周期实现
 *
 * 实现ZMODEM Sender端的构造/析构、文件校验初始化和取消序列。
 *
 * 状态机处理(stateToString/writeChecked/handleTimeout/processReceivedData/
 * setState/resetZmodemStatistics)见 ZModemTransferHandlers.cpp。
 * 状态回调处理(handleState*)见 ZModemTransferHandlers.cpp。
 * 帧构建(parseHexFrame/sendZRQINIT/sendZFILE/ZDATA/ZEOF/ZFIN)见 ZModemTransferProtocol.cpp。
 */
#include "ota/protocols/zmodem/ZModemTransfer.h"
#include <QFile>
#include <QFileInfo>
#include <QTimer>

/** @brief 构造函数，初始化ZMODEM传输器并设置默认超时10秒 */
ZModemTransfer::ZModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
    m_timeoutMs = 10000;
}
/** @brief 设置传输文件路径
 *  @param path 文件绝对路径 */
void ZModemTransfer::setFilePath(const QString& path) { m_filePath = path; }

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

// 状态机处理方法(stateToString/writeChecked/handleTimeout/processReceivedData/
// setState/resetZmodemStatistics)见 ZModemTransferHandlers.cpp
// 帧解析与发送流程方法见 ZModemTransferProtocol.cpp
