/**
 * @file YModemTransferStats.cpp
 * @brief YMODEM传输器 - 统计查询、速率计算与文件管理实现
 *
 * 从 YModemTransfer.cpp 拆分而来，包含统计getter、传输速率/ETA计算、
 * 统计更新信号和文件加载方法。
 */

#include "ota/protocols/ymodem/YModemTransfer.h"
#include <QFile>
#include <QFileInfo>

// ── 统计getter/reset已在.h中内联实现(stats()/resetStats()/便捷getter) ──

/** @brief 设置单个文件路径用于传输 @param path 文件绝对路径 */
void YModemTransfer::setFilePath(const QString& path) { m_filePaths = QStringList{path}; }
/** @brief 设置多个文件路径用于批量传输 @param paths 文件路径列表 */
void YModemTransfer::setFilePaths(const QStringList& paths) { m_filePaths = paths; }
/** @brief 获取当前传输速率
 *  @return 传输速率，单位: 字节/秒 */
double YModemTransfer::transferRate() const { return m_currentRate; }

/** @brief 计算剩余传输时间
 *  @return 预计剩余秒数，无法计算时返回-1 */
double YModemTransfer::etaSeconds() const
{
    if (m_currentRate <= 0.0 || m_totalBytes <= 0) return -1.0;
    qint64 remaining = m_totalBytes - m_totalBytesSent;
    if (remaining <= 0) return 0.0;
    return static_cast<double>(remaining) / m_currentRate;
}

/** @brief 更新传输速率统计并发射transferStats信号 */
void YModemTransfer::updateTransferStats()
{
    qint64 elapsedMs = m_transferTimer.elapsed();
    if (elapsedMs <= 0) return;
    double elapsedSec = static_cast<double>(elapsedMs) / 1000.0;
    if (elapsedSec > 0.0) {
        m_currentRate = static_cast<double>(m_totalBytesSent) / elapsedSec;
    }
    double eta = etaSeconds();
    emit transferStats(m_currentRate, eta, m_currentFileName);
}

/** @brief 加载下一个待传输文件到内存
 *  @return 文件加载成功返回true，无更多文件或读取失败返回false */
bool YModemTransfer::loadNextFile()
{
    if (m_fileIndex >= m_filePaths.size()) {
        emit transferError(tr("无更多文件可传输"));
        return false;
    }
    QFile file(m_filePaths[m_fileIndex]);
    if (!file.open(QIODevice::ReadOnly)) {
        emit transferError(
            tr("无法打开文件: %1").arg(m_filePaths[m_fileIndex]));
        return false;
    }
    m_currentData = file.readAll();
    if (m_currentData.size() != file.size()) {
        emit transferError(tr("读取文件失败"));
        return false;
    }
    file.close();
    m_currentFileName = QFileInfo(m_filePaths[m_fileIndex]).fileName();
    m_bytesSent = 0;
    return true;
}
