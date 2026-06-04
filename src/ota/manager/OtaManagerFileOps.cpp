/**
 * @file OtaManagerFileOps.cpp
 * @brief OTA升级管理器 — 文件验证/类型检测/HEX转换(拆分自OtaManager.cpp)
 *
 * 实现:
 *   - validateFilePath: 验证固件文件路径(非空/存在/可读/大小>0/<=64MB)
 *   - detectFirmwareType: 检测固件文件类型(.bin→Binary/.hex→IntelHex)
 *   - convertHexToBin: HEX文件转换为BIN临时文件(析构时自动清理)
 */

#include "ota/manager/OtaManager.h"
#include "protocol/hex/IntelHexParser.h"

#include <QFileInfo>
#include <QFile>
#include <QTemporaryFile>
#include <QDir>
#include <QDebug>

// ============================================================================
// 文件验证
// ============================================================================

/** @brief 验证固件文件路径(非空/存在/可读/大小>0/<=64MB) @param filePath 文件路径 @param errorMsg 错误信息输出 @return true=文件有效 */
bool OtaManager::validateFilePath(const QString& filePath, QString& errorMsg) const
{
    if (filePath.isEmpty()) {
        errorMsg = tr("文件路径为空");
        return false;
    }

    QFileInfo info(filePath);
    if (!info.exists()) {
        errorMsg = tr("文件不存在: %1").arg(filePath);
        return false;
    }

    if (!info.isReadable()) {
        errorMsg = tr("文件不可读: %1").arg(filePath);
        return false;
    }

    if (info.size() == 0) {
        errorMsg = tr("文件为空: %1").arg(filePath);
        return false;
    }

    if (info.size() > kMaxFirmwareSize) {
        errorMsg = tr("文件过大(%1 MB，上限 %2 MB)")
                       .arg(info.size() / (1024.0 * 1024.0), 0, 'f', 1)
                       .arg(kMaxFirmwareSize / (1024.0 * 1024.0), 0, 'f', 0);
        return false;
    }

    return true;
}

/** @brief 检测固件文件类型(.bin→Binary/.hex→IntelHex/其他→Unknown) @param filePath 文件路径 @return 文件类型枚举 */
OtaManager::FirmwareType OtaManager::detectFirmwareType(const QString& filePath) const
{
    QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == "bin") {
        return FirmwareType::Binary;
    }
    if (suffix == "hex" || suffix == "ihex") {
        return FirmwareType::IntelHex;
    }
    return FirmwareType::Unknown;
}

/** @brief 将HEX文件转换为BIN临时文件(析构时自动清理) @param hexPath HEX文件路径 @param outBinPath 输出的BIN文件路径 @return true=转换成功 */
bool OtaManager::convertHexToBin(const QString& hexPath, QString& outBinPath)
{
    QByteArray binary;
    quint32 startAddr = 0;

    if (!IntelHex::parse(hexPath, binary, startAddr)) {
        return false;
    }

    if (binary.isEmpty()) {
        return false;
    }

    // 删除上一次的临时文件对象(释放文件句柄和堆内存)
    if (m_tempBinFile) {
        m_tempBinFile->close();
        delete m_tempBinFile;
    }

    // 创建新的临时BIN文件
    m_tempBinFile = new QTemporaryFile(QDir::tempPath() + "/EmbedDebug_XXXXXX.bin", this);
    if (!m_tempBinFile->open()) {
        delete m_tempBinFile;
        m_tempBinFile = nullptr;
        return false;
    }

    if (m_tempBinFile->write(binary) != binary.size()) {
        delete m_tempBinFile;
        m_tempBinFile = nullptr;
        return false;
    }
    m_tempBinFile->close();

    // 清理旧的临时文件(磁盘上)
    if (!m_tempBinPath.isEmpty()) {
        QFile::remove(m_tempBinPath);
    }

    m_tempBinPath = m_tempBinFile->fileName();
    outBinPath = m_tempBinPath;

    // 不自动删除，因为传输过程需要读取
    m_tempBinFile->setAutoRemove(false);

    qDebug() << "OtaManager: HEX converted to BIN:" << m_tempBinPath
             << "size:" << binary.size() << "startAddr: 0x" << Qt::hex << startAddr;

    return true;
}
