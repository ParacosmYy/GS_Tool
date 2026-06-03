/**
 * @file RecordingFileFormat.cpp
 * @brief 录制文件格式读写器实现
 *
 * 实现EDB (EmbedDebug Binary) 文件格式的序列化与反序列化。
 * EDB格式在原始EDL录制数据基础上封装了JSON元数据层，
 * 支持通道名称、协议信息、采样率、标记点等富上下文信息。
 * 所有错误均通过 m_lastError 提供可读描述。
 *
 * 文件布局:
 *   [3B]  魔数 "EDB"
 *   [1B]  版本号
 *   [4B]  元数据JSON长度 (quint32 LE)
 *   [4B]  保留字段 (全零)
 *   [NB]  元数据JSON (UTF-8紧凑格式)
 *   [..]  原始EDL录制数据
 *   [4B]  EOF标记 "EDBE"
 */

#include "utils/log/RecordingFileFormat.h"

#include <QFile>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

// ============================================================================
// 构造函数
// ============================================================================

/**
 * @brief 构造函数，初始化基类
 * @param parent 父对象指针
 */
RecordingFileFormat::RecordingFileFormat(QObject* parent)
    : QObject(parent)
{
}

// ============================================================================
// 公开方法 - 数据存取
// ============================================================================

/**
 * @brief 设置待保存的原始录制数据和元数据
 *
 * 将调用方提供的原始EDL字节流和元数据Map缓存到内部成员，
 * 供后续 saveToFile() 序列化写出。
 *
 * @param rawData  原始录制字节流 (EDL记录格式)
 * @param meta     附加元数据键值对
 * @return true 数据非空且成功缓存
 */
bool RecordingFileFormat::setData(const QByteArray& rawData,
                                   const QVariantMap& meta)
{
    if (rawData.isEmpty()) {
        m_lastError = tr("原始数据为空，无法设置");
        qWarning() << "[RecordingFileFormat] setData:" << m_lastError;
        return false;
    }
    m_rawData  = rawData;
    m_metadata = meta;
    m_lastError.clear();
    return true;
}

/**
 * @brief 获取已加载的原始录制数据
 * @return 原始EDL录制字节流
 */
QByteArray RecordingFileFormat::rawData() const
{
    return m_rawData;
}

/**
 * @brief 检查是否持有有效数据
 * @return true m_rawData非空
 */
bool RecordingFileFormat::hasData() const
{
    return !m_rawData.isEmpty();
}

/**
 * @brief 获取文件元数据
 * @return 元数据键值对 (可能为空)
 */
QVariantMap RecordingFileFormat::metadata() const
{
    return m_metadata;
}

/**
 * @brief 获取最近一次操作的错误描述
 * @return 错误信息字符串，无错误时返回空串
 */
QString RecordingFileFormat::lastError() const
{
    return m_lastError;
}

// ============================================================================
// 公开方法 - 文件IO
// ============================================================================

/**
 * @brief 将录制数据保存到文件
 *
 * 文件写入流程:
 *   1. 将 m_metadata 序列化为紧凑JSON
 *   2. 构造12字节文件头 (魔数+版本+JSON长度+保留)
 *   3. 顺序写入: 文件头 → JSON元数据 → 原始EDL数据 → EOF标记
 *
 * @param filePath 目标文件路径
 * @return true 写入成功; false 文件打开失败或数据为空
 */
bool RecordingFileFormat::saveToFile(const QString& filePath)
{
    m_lastError.clear();

    if (m_rawData.isEmpty()) {
        m_lastError = tr("无录制数据可保存，请先设置数据");
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        return false;
    }

    // --- 序列化元数据为紧凑JSON ---
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(m_metadata);
    QByteArray jsonBytes = jsonDoc.toJson(QJsonDocument::Compact);

    // 检查元数据大小上限 (quint32 最大 ~4GB，实际远小于此)
    if (static_cast<quint32>(jsonBytes.size()) > 0xFFFFFFFF) {
        m_lastError = tr("元数据过大(%1字节)，超过格式上限")
                          .arg(jsonBytes.size());
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = tr("无法打开文件写入: %1 (%2)")
                          .arg(filePath, file.errorString());
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        return false;
    }

    // --- 构造并写入文件头 ---
    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    // 魔数 "EDB" (3字节)
    out.writeRawData(kMagic, 3);

    // 版本号 (1字节)
    out << kVersion;

    // 元数据JSON长度 (4字节 quint32)
    out << static_cast<quint32>(jsonBytes.size());

    // 保留字段 (4字节全零)
    out << static_cast<quint32>(0);

    // --- 写入元数据JSON ---
    if (file.write(jsonBytes) != jsonBytes.size()) {
        m_lastError = tr("写入元数据失败: %1").arg(file.errorString());
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        file.close();
        return false;
    }

    // --- 写入原始录制数据 ---
    if (file.write(m_rawData) != m_rawData.size()) {
        m_lastError = tr("写入录制数据失败: %1").arg(file.errorString());
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        file.close();
        return false;
    }

    // --- 写入EOF标记 ---
    if (file.write(kEofMarker, kEofMarkerSize) != kEofMarkerSize) {
        m_lastError = tr("写入EOF标记失败: %1").arg(file.errorString());
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        file.close();
        return false;
    }

    file.close();
    m_filePath = filePath;
    ++m_totalSaves;
    return true;
}

/**
 * @brief 从文件加载录制数据
 *
 * 文件读取流程:
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
        return false;
    }

    // --- 读取文件头 ---
    QByteArray header = file.read(kHeaderSize);
    if (header.size() != kHeaderSize) {
        m_lastError = tr("读取文件头失败: 期望%1字节，实际%2字节")
                          .arg(kHeaderSize).arg(header.size());
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        return false;
    }

    // --- 校验魔数 ---
    if (!header.startsWith(kMagic)) {
        m_lastError = tr("文件魔数不匹配: 期望\"%1\"").arg(QString::fromLatin1(kMagic));
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
        return false;
    }

    // --- 校验版本号 ---
    const quint8 fileVersion = static_cast<quint8>(header[3]);
    if (fileVersion != kVersion) {
        m_lastError = tr("文件版本不匹配: 文件版本%1，当前支持版本%2")
                          .arg(fileVersion).arg(kVersion);
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
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
        return false;
    }

    // --- 解析元数据长度 (字节4-7, quint32 LE) ---
    quint32 metaSize = static_cast<quint8>(header[4])
                     | (static_cast<quint32>(static_cast<quint8>(header[5])) << 8)
                     | (static_cast<quint32>(static_cast<quint8>(header[6])) << 16)
                     | (static_cast<quint32>(static_cast<quint8>(header[7])) << 24);

    // --- 校验元数据区域不超出文件范围 ---
    const qint64 dataStart = kHeaderSize + static_cast<qint64>(metaSize);
    if (dataStart > fileSize - kEofMarkerSize) {
        m_lastError = tr("元数据区域偏移(%1)超出文件有效范围(%2)")
                          .arg(dataStart).arg(fileSize - kEofMarkerSize);
        qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
        file.close();
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
            return false;
        }

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonBytes);
        if (jsonDoc.isNull()) {
            m_lastError = tr("元数据JSON解析失败，文件可能已损坏");
            qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
            file.close();
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
        return false;
    }

    // 大文件分块读取，周期性上报进度
    m_rawData.clear();
    if (rawBytesRemaining > 0) {
        constexpr qint64 chunkSize = 1024 * 1024;  ///< 每次读取1MB
        qint64 totalRead = 0;

        while (totalRead < rawBytesRemaining) {
            const qint64 toRead = qMin(chunkSize, rawBytesRemaining - totalRead);
            QByteArray chunk = file.read(toRead);

            if (chunk.size() != toRead) {
                m_lastError = tr("读取录制数据不完整: 已读%1字节，期望%2字节")
                                  .arg(totalRead + chunk.size()).arg(rawBytesRemaining);
                qWarning() << "[RecordingFileFormat] loadFromFile:" << m_lastError;
                file.close();
                return false;
            }

            m_rawData.append(chunk);
            totalRead += chunk.size();

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
        return false;
    }

    file.close();
    m_filePath = filePath;

    // --- 发射完成信号 ---
    emit loadProgress(1.0);
    emit loadCompleted(m_metadata);

    ++m_totalLoads;
    return true;
}
