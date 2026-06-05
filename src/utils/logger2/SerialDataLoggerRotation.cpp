/**
 * @file SerialDataLoggerRotation.cpp
 * @brief 高级串口数据日志记录器 -- 日志轮转/压缩/定时回调
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 包含轮转执行、gzip 压缩、文件数量限制和定时器回调。
 * 核心逻辑见 @see SerialDataLogger.cpp
 */

#include "utils/logger2/SerialDataLogger.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

// ──────────────────────────────────────────────
// 定时 flush 回调
// ──────────────────────────────────────────────

/**
 * @brief 定时 flush 回调(1秒间隔)
 *
 * 将缓冲区数据写入磁盘，保持数据持久性。
 */
void SerialDataLogger::onFlushTimer()
{
    if (m_logging && !m_paused) {
        flush();
    }
}

// ──────────────────────────────────────────────
// 定时轮转检查回调
// ──────────────────────────────────────────────

/**
 * @brief 定时轮转检查(30秒间隔)
 *
 * 检查是否满足时间轮转条件(每小时/每天)。
 * 文件大小轮转在 flush() 中即时触发，此处不重复检查。
 */
void SerialDataLogger::onRotationCheck()
{
    if (!m_logging) {
        return;
    }

    if (m_rotationPeriod == RotationPeriod::None) {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    bool shouldRotate = false;

    switch (m_rotationPeriod) {
    case RotationPeriod::Hourly: {
        // 上次轮转距今超过 1 小时
        const qint64 hourMs = 3600LL * 1000LL;
        if (now - m_rotationBaseEpoch >= hourMs) {
            shouldRotate = true;
        }
        break;
    }
    case RotationPeriod::Daily: {
        // 上次轮转距今超过 24 小时
        const qint64 dayMs = 86400LL * 1000LL;
        if (now - m_rotationBaseEpoch >= dayMs) {
            shouldRotate = true;
        }
        break;
    }
    case RotationPeriod::None:
        break;
    }

    if (shouldRotate) {
        performRotation();
    }
}

// ──────────────────────────────────────────────
// 执行轮转
// ──────────────────────────────────────────────

/**
 * @brief 执行日志轮转
 *
 * 步骤：
 * 1. 关闭当前文件
 * 2. 压缩旧日志(异步)
 * 3. 清理超出数量限制的文件
 * 4. 打开新文件
 *
 * 轮转失败时发出 error 信号，但不终止日志记录状态。
 */
void SerialDataLogger::performRotation()
{
    if (!m_file) {
        return;
    }

    // 保存旧文件路径
    const QString oldPath = m_currentFilePath;

    // 关闭当前文件
    closeLogFile();

    // 压缩旧日志(异步)
    compressOldFiles();

    // 更新轮转基准时间
    m_rotationBaseEpoch = QDateTime::currentMSecsSinceEpoch();
    ++m_totalRotations;

    // 打开新文件
    const QString newPath = resolvePattern(m_filePattern);
    if (!openLogFile(newPath)) {
        emit error(tr("日志轮转后无法创建新文件"));
        return;
    }

    // 强制执行最大文件数限制
    enforceMaxFiles();

    emit logRotated(newPath);
}

// ──────────────────────────────────────────────
// gzip 压缩旧日志
// ──────────────────────────────────────────────

/**
 * @brief 使用外部 gzip 压缩非当前日志文件
 *
 * 遍历日志目录中的未压缩日志文件，调用 gzip 压缩。
 * 压缩成功后删除原始文件。当前正在写入的文件不会被压缩。
 *
 * 在 Windows 上使用 gzip.exe(需在 PATH 中)或 Qt 自带的压缩。
 */
void SerialDataLogger::compressOldFiles()
{
    if (!m_logDir.exists()) {
        return;
    }

    // 收集可压缩的文件扩展名
    const QStringList compressExts = {
        QStringLiteral("log"), QStringLiteral("csv"),
        QStringLiteral("hex"), QStringLiteral("txt")
    };

    const QFileInfoList entries = m_logDir.entryInfoList(QDir::Files, QDir::Time);
    for (const QFileInfo& fi : entries) {
        // 跳过已压缩文件
        if (fi.suffix() == QStringLiteral("gz")) {
            continue;
        }
        // 跳过当前正在写入的文件
        if (fi.absoluteFilePath() == m_currentFilePath) {
            continue;
        }
        // 只压缩指定扩展名的文件
        if (!compressExts.contains(fi.suffix())) {
            continue;
        }

        const QString filePath = fi.absoluteFilePath();
        const QString gzPath = filePath + QStringLiteral(".gz");

        // 使用 QProcess 调用 gzip
        QProcess gzip;
        gzip.start(QStringLiteral("gzip"), { QStringLiteral("-f"), filePath });
        if (gzip.waitForFinished(5000) && gzip.exitCode() == 0) {
            ++m_totalCompressions;
        }
        // gzip 失败时静默跳过(不阻塞主流程)
    }
}

// ──────────────────────────────────────────────
// 文件数量限制
// ──────────────────────────────────────────────

/**
 * @brief 删除超出最大文件数量限制的最旧日志文件
 *
 * 按 m_maxFiles 配置，超出时删除最早创建的文件(含 .gz 压缩文件)。
 * m_maxFiles 为 0 时不执行任何删除操作。
 */
void SerialDataLogger::enforceMaxFiles()
{
    if (m_maxFiles <= 0) {
        return;
    }

    const QStringList allFiles = getLogFiles();

    // 需要删除的文件数 = 总数 - 限制(当前文件不计入删除)
    const int excess = allFiles.size() - m_maxFiles;
    if (excess <= 0) {
        return;
    }

    // getLogFiles 按时间升序，删除最旧的 excess 个
    for (int i = 0; i < excess && i < allFiles.size(); ++i) {
        // 不删除当前正在写入的文件
        if (allFiles[i] == m_currentFilePath) {
            continue;
        }
        QFile::remove(allFiles[i]);
    }
}
