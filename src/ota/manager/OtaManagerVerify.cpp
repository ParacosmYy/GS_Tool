/**
 * @file OtaManagerVerify.cpp
 * @brief OTA升级管理器 — 状态查询与校验和验证(拆分自OtaManager.cpp)
 *
 * 本文件从 OtaManager.cpp 拆分而来，包含:
 *   - 状态设置与查询 (setOtaState / otaState)
 *   - 传输状态查询 (transferCount / lastTransferSuccess / lastFileName / currentProtocolName)
 *   - CRC32校验和计算 (computeFileCrc32)
 *   - 校验和比对验证 (verifyChecksum)
 *
 * 拆分原因: OtaManager.cpp 超过 340 行目标，将状态查询和校验和验证
 * 独立出来以保持主文件聚焦于 OTA 生命周期管理和信号连接。
 *
 * @see OtaManager.cpp — 构造/析构/信号连接/连接管理/传输控制
 * @see OtaManagerFileOps.cpp — 文件验证/类型检测/HEX转换
 * @see OtaManagerProgress.cpp — 协议显示名称/传输统计/平均速率
 */

#include "ota/manager/OtaManager.h"
#include "utils/crypto/CRC.h"

#include <QFileInfo>

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
