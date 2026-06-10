/**
 * @file RecordingFileFormat.cpp
 * @brief 录制文件格式读写器实现 — 写入路径
 *
 * 实现EDB (EmbedDebug Binary) 文件格式的序列化部分，
 * 包括文件头构造、元数据JSON写入、原始数据落盘和EOF标记。
 * 写入路径负责将内存中的录制数据和元数据持久化为EDB二进制文件。
 *
 * 文件布局:
 *   [3B]  魔数 "EDB"
 *   [1B]  版本号
 *   [4B]  元数据JSON长度 (quint32 LE)
 *   [4B]  保留字段 (全零)
 *   [NB]  元数据JSON (UTF-8紧凑格式)
 *   [..]  原始EDL录制数据
 *   [4B]  EOF标记 "EDBE"
 *
 * @note 读取/加载路径 (loadFromFile) 已拆分至 RecordingFileIndex.cpp
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
    const QString normalizedPath = filePath.trimmed();

    if (normalizedPath.isEmpty()) {
        m_lastError = tr("文件路径不能为空");
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        ++m_totalErrors; ++m_serializationErrors;
        return false;
    }

    if (m_rawData.isEmpty()) {
        m_lastError = tr("无录制数据可保存，请先设置数据");
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        ++m_totalErrors; ++m_serializationErrors;
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
        ++m_totalErrors; ++m_serializationErrors;
        return false;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = tr("无法打开文件写入: %1 (%2)")
                          .arg(normalizedPath, file.errorString());
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        ++m_totalErrors; ++m_serializationErrors;
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
        ++m_totalErrors; ++m_serializationErrors;
        return false;
    }

    // --- 写入原始录制数据 ---
    if (file.write(m_rawData) != m_rawData.size()) {
        m_lastError = tr("写入录制数据失败: %1").arg(file.errorString());
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_serializationErrors;
        return false;
    }

    // --- 写入EOF标记 ---
    if (file.write(kEofMarker, kEofMarkerSize) != kEofMarkerSize) {
        m_lastError = tr("写入EOF标记失败: %1").arg(file.errorString());
        qWarning() << "[RecordingFileFormat] saveToFile:" << m_lastError;
        file.close();
        ++m_totalErrors; ++m_serializationErrors;
        return false;
    }

    file.close();
    m_filePath = normalizedPath;
    ++m_totalSaves;
    ++m_totalIndexBuilds;  ///< 统计: 保存时构建段索引
    m_totalBytesWritten += static_cast<quint64>(jsonBytes.size() + m_rawData.size());
    return true;
}

// ============================================================================
// 文件加载/索引/统计方法已拆分至 RecordingFileIndex.cpp
// ============================================================================
