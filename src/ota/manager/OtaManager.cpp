/**
 * @file OtaManager.cpp
 * @brief OTA升级管理器实现 - 文件验证、HEX转换、状态管理、校验和验证、信号转发
 *
 * 核心流程:
 *   startTransfer()
 *     1. 设置状态为 Selecting
 *     2. 验证文件路径(存在/可读/大小)
 *     3. 检测文件类型(BIN/HEX)
 *     4. HEX文件自动转BIN(IntelHexParser)
 *     5. 设置状态为 Transferring
 *     6. 启动传输计时器
 *     7. 调用对应协议的start()
 *
 *   传输完成时记录速率到历史记录，用于计算平均速率
 */

#include "ota/manager/OtaManager.h"
#include "protocol/hex/IntelHexParser.h"
#include "utils/crypto/CRC.h"

#include <QFileInfo>
#include <QFile>
#include <QTemporaryFile>
#include <QDir>
#include <QDebug>

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造OTA管理器，创建三个传输协议实例并连接信号 */
OtaManager::OtaManager(QObject* parent)
    : QObject(parent)
    , m_xmodem(new XModemTransfer(this))
    , m_ymodem(new YModemTransfer(this))
    , m_zmodem(new ZModemTransfer(this))
{
    // 连接三个协议的基础信号(progress/complete/error)
    connectTransferSignals(m_xmodem);
    connectTransferSignals(m_ymodem);
    connectTransferSignals(m_zmodem);

    // 连接各协议的传输速率信号
    connectXModemStats();
    connectYModemStats();

    // 连接XModem模式降级信号
    connect(m_xmodem, &XModemTransfer::modeDegraded,
            this, [this](const QString& from, const QString& to) {
                QString msg = tr("接收方仅支持Checksum模式，已自动从 %1 降级为 %2")
                                  .arg(from, to);
                qWarning() << "OtaManager:" << msg;
                emit modeDegraded(msg);
            });
}

/** @brief 析构函数 — 清理HEX转换产生的临时BIN文件 */
OtaManager::~OtaManager()
{
    if (!m_tempBinPath.isEmpty()) {
        QFile::remove(m_tempBinPath);
    }
}

// ============================================================================
// 信号连接
// ============================================================================

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

// ============================================================================
// 状态管理
// ============================================================================

/** @brief 设置OTA状态并发射otaStateChanged信号 */
void OtaManager::setOtaState(OtaState state)
{
    if (m_otaState != state) {
        m_otaState = state;
        emit otaStateChanged(state);
    }
}

/** @brief 获取当前OTA状态(Idle/Selecting/Transferring/Complete/Error) @return OtaState枚举 */
OtaManager::OtaState OtaManager::otaState() const
{
    return m_otaState;
}

/** @brief 获取历史传输总次数 */
int OtaManager::transferCount() const
{
    return m_transferCount;
}

/** @brief 获取上次传输是否成功 */
bool OtaManager::lastTransferSuccess() const
{
    return m_lastTransferSuccess;
}

/** @brief 获取上次传输的文件名 */
QString OtaManager::lastFileName() const
{
    return m_currentFileName;
}

/** @brief 获取当前协议名称 */
QString OtaManager::currentProtocolName() const
{
    return m_currentProtocol;
}

// ============================================================================
// 连接和传输控制
// ============================================================================

/** @brief 设置数据连接(同步到三个协议实例，活跃传输期间自动取消) @param conn 新的数据连接 */
void OtaManager::setConnection(IConnection* conn)
{
    // 活跃传输期间切换连接: 先取消当前传输，避免协议实例持有失效的连接
    if (m_otaState != OtaState::Idle) {
        qWarning() << "OtaManager: 活跃传输期间切换连接，当前状态:"
                   << static_cast<int>(m_otaState) << "，自动取消传输";
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
    // 注意: 始终使用用户选择的原始文件名，而非HEX转换后的临时BIN文件名
    // 避免在错误消息和Toast通知中显示类似 "EmbedDebug_XXXXXX.bin" 的临时文件名
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


// ---- 文件验证/类型检测/HEX转换 已拆分至 OtaManagerFileOps.cpp ----


// ============================================================================
// 校验和验证
// ============================================================================

/** @brief 计算文件的CRC32校验和 @param filePath 文件路径 @return CRC32十六进制字符串(8位大写)，失败返回空字符串 */
QString OtaManager::computeFileCrc32(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    // 使用CRC工具计算CRC32
    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        return {};
    }

    quint32 crc = CRC::crc32(data);
    return QString("%1").arg(crc, 8, 16, QChar('0')).toUpper();
}

/** @brief 传输后校验和验证，对比固件文件CRC32与预期值 @param filePath 固件文件路径 @param expectedChecksum 预期CRC32(十六进制字符串) @param outError 错误描述输出 @return VerifyResult校验结果 */
OtaManager::VerifyResult OtaManager::verifyChecksum(const QString& filePath,
                                                     const QString& expectedChecksum,
                                                     QString& outError)
{
    // 检查文件是否存在
    QFileInfo info(filePath);
    if (!info.exists()) {
        outError = tr("校验文件不存在: %1").arg(filePath);
        return VerifyResult::FileNotFound;
    }

    // 检查预期校验和是否为空
    if (expectedChecksum.trimmed().isEmpty()) {
        outError = tr("预期校验和为空");
        return VerifyResult::ChecksumEmpty;
    }

    // 计算文件CRC32
    QString actual = computeFileCrc32(filePath);
    if (actual.isEmpty()) {
        outError = tr("无法读取文件计算校验和: %1").arg(filePath);
        return VerifyResult::ReadError;
    }

    // 比对校验和(不区分大小写)
    if (actual.compare(expectedChecksum.trimmed(), Qt::CaseInsensitive) != 0) {
        outError = tr("校验和不匹配: 预期 %1, 实际 %2")
                       .arg(expectedChecksum.trimmed().toUpper(), actual);
        return VerifyResult::Mismatch;
    }

    return VerifyResult::Ok;
}

// ---- 协议显示名称 / 传输统计 / 平均速率 → 已拆分至 OtaManagerProgress.cpp ----
