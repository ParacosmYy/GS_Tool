/**
 * @file OtaManagerSignals.cpp
 * @brief OTA管理器 - 信号连接与传输控制实现
 *
 * 从 OtaManager.cpp 拆分而来，包含统一信号绑定、
 * XModem/YModem统计信号转发和传输控制方法。
 */

#include "ota/manager/OtaManager.h"
#include "protocol/hex/IntelHexParser.h"

#include <QFileInfo>
#include <QFile>
#include <QTemporaryFile>
#include <QDir>
#include <QDebug>

/** @brief 统一绑定BaseTransfer基础信号到OtaManager转发 @param transfer 传输协议实例 */
void OtaManager::connectTransferSignals(BaseTransfer* transfer)
{
    connect(transfer, &BaseTransfer::progress,
            this, &OtaManager::progress);
    connect(transfer, &BaseTransfer::transferComplete,
            this, [this]() {
                setOtaState(OtaState::Complete);
                ++m_transferCount;
                ++m_successfulTransfers;
                m_totalBytesTransferred += static_cast<quint64>(m_currentFileSize);
                m_lastTransferSuccess = true;

                // 记录本次传输速率到历史记录
                if (m_transferTimer.elapsed() > 0) {
                    double elapsedSec = static_cast<double>(m_transferTimer.elapsed()) / 1000.0;
                    if (elapsedSec > 0.0 && m_currentFileSize > 0) {
                        double speed = static_cast<double>(m_currentFileSize) / elapsedSec;
                        m_speedHistory.append(speed);
                        // 限制历史记录长度，防止内存无限增长
                        if (m_speedHistory.size() > kMaxSpeedHistory) {
                            m_speedHistory.removeFirst();
                        }
                    }
                }

                emit transferComplete();
            });
    connect(transfer, &BaseTransfer::transferError,
            this, [this](const QString& reason) {
                setOtaState(OtaState::Error);
                ++m_transferCount;
                ++m_failedTransfers;
                m_lastTransferSuccess = false;
                // 增强错误消息: 追加协议名称上下文
                QString enriched = reason;
                if (!m_currentProtocol.isEmpty()) {
                    enriched = tr("[%1] %2").arg(protocolDisplayName(m_currentProtocol), enriched);
                }
                if (!m_currentFileName.isEmpty() && !reason.contains(m_currentFileName)) {
                    enriched = tr("[%1] %2").arg(m_currentFileName, enriched);
                }
                emit transferError(enriched);
            });
}

/** @brief 绑定XModemTransfer的transferStats信号到OtaManager转发 */
void OtaManager::connectXModemStats()
{
    connect(m_xmodem, &XModemTransfer::transferStats,
            this, &OtaManager::transferStats);
}

/** @brief 绑定YModemTransfer的transferStats信号到OtaManager转发
 *
 * YModem的transferStats有3个参数(fileName)，只取前2个(rate, eta)转发
 */
void OtaManager::connectYModemStats()
{
    connect(m_ymodem, &YModemTransfer::transferStats,
            this, [this](double rate, double eta, const QString& /*fileName*/) {
                emit transferStats(rate, eta);
            });
}

/** @brief 设置数据连接(同步到三个协议实例，活跃传输期间自动取消) @param conn 新的数据连接 */
void OtaManager::setConnection(IConnection* conn)
{
    // 活跃传输期间切换连接: 先取消当前传输，避免协议实例持有失效的连接
    if (m_otaState != OtaState::Idle) {
        qWarning() << tr("OtaManager: 活跃传输期间切换连接，当前状态: %1，自动取消传输")
                      .arg(static_cast<int>(m_otaState));
        cancelTransfer();
    }

    m_conn = conn;
    m_xmodem->setConnection(conn);
    m_ymodem->setConnection(conn);
    m_zmodem->setConnection(conn);
}

/** @brief 开始OTA传输(验证文件→检测类型→HEX转BIN→启动计时→选择协议→启动) @param filePath 固件文件路径 @param protocol 传输协议名称 @return true=成功启动，false=验证失败 */
bool OtaManager::startTransfer(const QString& filePath, const QString& protocol)
{
    // ---- 步骤1: 连接检查 ----
    if (!m_conn) {
        setOtaState(OtaState::Error);
        emit transferError(tr("无可用连接"));
        return false;
    }

    // ---- 步骤2: 递增传输尝试计数器 ----
    ++m_totalTransfers;

    // ---- 步骤3: 进入文件选择验证阶段 ----
    setOtaState(OtaState::Selecting);

    // ---- 步骤4: 验证文件路径 ----
    QString errorMsg;
    if (!validateFilePath(filePath, errorMsg)) {
        setOtaState(OtaState::Error);
        emit transferError(errorMsg);
        return false;
    }

    // ---- 步骤5: 检测文件类型并处理 ----
    FirmwareType type = detectFirmwareType(filePath);
    QString effectivePath = filePath;

    if (type == FirmwareType::IntelHex) {
        // HEX文件需要转换为BIN
        QString binPath;
        if (!convertHexToBin(filePath, binPath)) {
            setOtaState(OtaState::Error);
            emit transferError(tr("HEX文件转换失败: %1").arg(filePath));
            return false;
        }
        effectivePath = binPath;
    } else if (type == FirmwareType::Unknown) {
        // 未知类型按BIN处理，给出警告但不阻止
        qDebug() << "OtaManager: Unknown firmware type, treating as binary:" << filePath;
    }

    // ---- 步骤6: 记录当前文件名和协议（用于错误消息上下文） ----
    m_currentFileName = QFileInfo(filePath).fileName();
    m_currentProtocol = protocol;
    m_currentFileSize = QFileInfo(effectivePath).size();

    // ---- 步骤7: 启动传输计时器(用于计算本次传输速率) ----
    m_transferTimer.start();

    // ---- 步骤8: 切换到传输状态 ----
    setOtaState(OtaState::Transferring);

    // ---- 步骤9: 根据协议选择传输实例 ----
    if (protocol == "ymodem") {
        m_ymodem->setFilePath(effectivePath);
        return m_ymodem->start();
    }

    if (protocol == "zmodem") {
        m_zmodem->setFilePath(effectivePath);
        return m_zmodem->start();
    }

    // XMODEM模式选择
    if (protocol == "xmodem-checksum") {
        m_xmodem->setMode(XModemTransfer::Checksum);
    } else if (protocol == "xmodem-1k") {
        m_xmodem->setMode(XModemTransfer::OneK);
    } else {
        m_xmodem->setMode(XModemTransfer::CRC);
    }

    m_xmodem->setFilePath(effectivePath);
    return m_xmodem->start();
}

/** @brief 取消正在进行的传输，委托给三个协议实例 */
void OtaManager::cancelTransfer()
{
    if (m_xmodem->isRunning()) {
        m_xmodem->cancel();
    }
    if (m_ymodem->isRunning()) {
        m_ymodem->cancel();
    }
    if (m_zmodem->isRunning()) {
        m_zmodem->cancel();
    }
    setOtaState(OtaState::Idle);
}

/** @brief 检查是否有任何协议实例正在传输 */
bool OtaManager::isTransferring() const
{
    return m_xmodem->isRunning() || m_ymodem->isRunning() || m_zmodem->isRunning();
}
