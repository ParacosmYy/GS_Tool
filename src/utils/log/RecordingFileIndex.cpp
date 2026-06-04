/**
 * @file RecordingFileIndex.cpp
 * @brief 录制文件读取/加载路径实现
 *
 * 实现EDB (EmbedDebug Binary) 文件格式的反序列化部分，
 * 包括文件头校验、元数据解析、分块数据读取、EOF完整性校验
 * 和加载进度上报。支持大文件流式读取和周期性进度信号发射。
 *
 * 读取流程:
 *   1. 读取并校验12字节文件头 (魔数、版本、保留字节)
 *   2. 根据头部记录的长度读取JSON元数据
 *   3. 分块读取剩余字节 (去除EOF标记) 作为原始EDL数据
 *   4. 校验EOF标记完整性
 *   5. 大文件加载时周期性发射 loadProgress 信号
 *   6. 完成后发射 loadCompleted 信号
 *
 * @note 写入/保存路径 (saveToFile) 保留在 RecordingFileFormat.cpp
 */

#include "utils/log/RecordingFileFormat.h"

#include <QFile>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

// ============================================================================
// 公开方法 - 文件加载/读取路径
// ============================================================================

/**
 * @brief 从文件加载录制数据
 *
 * 完整的文件读取流程:
 *   1. 读取并校验12字节文件头 (魔数、版本、保留字节)
 *   2. 根据头部记录的长度读取JSON元数据
 *   3. 读取剩余字节 (去除EOF标记) 作为原始EDL数据
 *   4. 校验EOF标记完整性
 *   5. 大文件加载时周期性发射 loadProgress 信号
 *   6. 完成后发射 loadCompleted 信号
 *
 * @param filePath 源文件路径
 * @return true 加载成功; false 文件不存在、头损坏或版本不匹配
 */
bool RecordingFileFormat::loadFromFile(const QString& filePath)
{
    m_lastError.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = tr("无法打开文件读取: %1 (%2)")
                          .arg(filePath, file.errorString());
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        ++m_totalErrors; ++m_deserializationErrors;
        return false;
    }

    const qint64 fileSize = file.size();

    // --- 最小文件大小校验 (头部 + EOF标记) ---
    const qint64 minFileSize = kHeaderSize + kEofMarkerSize;
    if (fileSize < minFileSize) {
        m_lastError = tr("文件过小(%1字节)，至少需要%2字节")
                          .arg(fileSize).arg(minFileSize);
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_deserializationErrors;
        return false;
    }

    // --- 读取文件头 ---
    QByteArray header = file.read(kHeaderSize);
    if (header.size() != kHeaderSize) {
        m_lastError = tr("读取文件头失败: 期望%1字节，实际%2字节")
                          .arg(kHeaderSize).arg(header.size());
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_deserializationErrors;
        return false;
    }

    // --- 校验魔数 ---
    if (!header.startsWith(kMagic)) {
        m_lastError = tr("文件魔数不匹配: 期望\"%1\"").arg(QString::fromLatin1(kMagic));
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_deserializationErrors; ++m_totalValidationFailures;
        return false;
    }

    // --- 校验版本号 ---
    const quint8 fileVersion = static_cast<quint8>(header[3]);
    if (fileVersion != kVersion) {
        m_lastError = tr("文件版本不匹配: 文件版本%1，当前支持版本%2")
                          .arg(fileVersion).arg(kVersion);
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_deserializationErrors; ++m_totalValidationFailures;
        return false;
    }

    // --- 校验保留字节 (字节8-11应为全零) ---
    bool reservedValid = true;
    for (int i = 8; i < kHeaderSize; ++i) {
        if (static_cast<quint8>(header[i]) != 0) {
            reservedValid = false;
            break;
        }
    }
    if (!reservedValid) {
        m_lastError = tr("文件头保留字节非零，文件可能已损坏");
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_deserializationErrors; ++m_totalValidationFailures;
        return false;
    }

    // --- 解析元数据长度 (字节4-7, quint32 LE) ---
    quint32 metaSize = static_cast<quint8>(header[4])
                     | (static_cast<quint32>(static_cast<quint8>(header[5])) << 8)
                     | (static_cast<quint32>(static_cast<quint8>(header[6])) << 16)
                     | (static_cast<quint32>(static_cast<quint8>(header[7])) << 24);

    // --- 统计：元数据查询视为一次索引查找 ---
    ++m_lookupsPerformed;

    // --- 校验元数据区域不超出文件范围 ---
    const qint64 dataStart = kHeaderSize + static_cast<qint64>(metaSize);
    if (dataStart > fileSize - kEofMarkerSize) {
        m_lastError = tr("元数据区域偏移(%1)超出文件有效范围(%2)")
                          .arg(dataStart).arg(fileSize - kEofMarkerSize);
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_deserializationErrors;
        return false;
    }

    // --- 读取元数据JSON ---
    QByteArray jsonBytes;
    if (metaSize > 0) {
        jsonBytes = file.read(static_cast<qint64>(metaSize));
        if (static_cast<quint32>(jsonBytes.size()) != metaSize) {
            m_lastError = tr("读取元数据不完整: 期望%1字节，实际%2字节")
                              .arg(metaSize).arg(jsonBytes.size());
            qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
            file.close();
            ++m_totalErrors; ++m_deserializationErrors;
            return false;
        }

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonBytes);
        if (jsonDoc.isNull()) {
            m_lastError = tr("元数据JSON解析失败，文件可能已损坏");
            qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
            file.close();
            ++m_totalErrors; ++m_deserializationErrors;
            return false;
        }
        m_metadata = jsonDoc.toVariant().toMap();
    } else {
        m_metadata.clear();
    }

    // 发射进度: 元数据已解析
    emit loadProgress(0.1);

    // --- 读取剩余原始录制数据 (排除EOF标记) ---
    const qint64 rawBytesRemaining = fileSize - dataStart - kEofMarkerSize;
    if (rawBytesRemaining < 0) {
        m_lastError = tr("文件数据区域大小异常: 无效的数据长度");
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_deserializationErrors;
        return false;
    }

    // 大文件分块读取，周期性上报进度
    m_rawData.clear();
    if (rawBytesRemaining > 0) {
        constexpr qint64 chunkSize = 1024 * 1024;  ///< 每次读取1MB
        qint64 totalRead = 0;

        // --- 统计：文件定位操作(数据区起始位置) ---
        ++m_totalSeekOperations;

        while (totalRead < rawBytesRemaining) {
            const qint64 toRead = qMin(chunkSize, rawBytesRemaining - totalRead);
            QByteArray chunk = file.read(toRead);

            if (chunk.size() != toRead) {
                m_lastError = tr("读取录制数据不完整: 已读%1字节，期望%2字节")
                                  .arg(totalRead + chunk.size()).arg(rawBytesRemaining);
                qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
                file.close();
                ++m_totalErrors; ++m_deserializationErrors;
                return false;
            }

            m_rawData.append(chunk);
            totalRead += chunk.size();
            ++m_totalSegmentsLoaded;  ///< 统计: 每个分块读取计为一段

            // 计算并上报进度 (0.1 ~ 1.0 区间映射)
            const qreal progress = 0.1 + 0.9 * (static_cast<qreal>(totalRead)
                                                / static_cast<qreal>(rawBytesRemaining));
            emit loadProgress(progress);
        }
    }

    // --- 校验EOF标记 ---
    QByteArray eofMarker = file.read(kEofMarkerSize);
    if (eofMarker.size() != kEofMarkerSize || eofMarker != QByteArray(kEofMarker, kEofMarkerSize)) {
        m_lastError = tr("EOF标记校验失败: 文件可能被截断或损坏 (期望\"%1\")")
                          .arg(QString::fromLatin1(kEofMarker));
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_deserializationErrors; ++m_totalValidationFailures;
        return false;
    }

    file.close();

    // --- 统计：缓存命中检测（同一文件路径重复加载） ---
    if (m_filePath == filePath) {
        ++m_cacheHits;
    } else {
        ++m_totalCacheMisses;
    }
    m_filePath = filePath;

    // --- 统计：累计读取字节数(不含头部和EOF) ---
    m_totalBytesRead += static_cast<quint64>(jsonBytes.size() + rawBytesRemaining);

    // --- 发射完成信号 ---
    emit loadProgress(1.0);
    emit loadCompleted(m_metadata);

    ++m_totalLoads;
    return true;
}

// 统计重置见 RecordingFileIndexStats.cpp
