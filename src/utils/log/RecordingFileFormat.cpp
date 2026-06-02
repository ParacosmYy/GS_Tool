/**
 * @file RecordingFileFormat.cpp
 * @brief 录制文件格式读写器实现
 *
 * 实现EDB (EmbedDebug Binary) 文件格式的序列化与反序列化。
 * EDB格式在原始EDL录制数据基础上封装了JSON元数据层，
 * 支持通道名称、协议信息、采样率、标记点等富上下文信息。
 *
 * 文件布局:
 *   [3B] 魔数 "EDB"
 *   [1B] 版本号
 *   [4B] 元数据JSON长度 (quint32 LE)
 *   [4B] 保留字段 (全零)
 *   [NB] 元数据JSON (UTF-8紧凑格式)
 *   [..] 原始EDL录制数据
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
        qWarning() << "[RecordingFileFormat] setData: 原始数据为空，忽略";
        return false;
    }
    m_rawData  = rawData;
    m_metadata = meta;
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

// ============================================================================
// 公开方法 - 文件IO
// ============================================================================

/**
 * @brief 将录制数据保存到文件
 *
 * 文件写入流程:
 *   1. 将 m_metadata 序列化为紧凑JSON
 *   2. 构造12字节文件头 (魔数+版本+JSON长度+保留)
 *   3. 顺序写入: 文件头 → JSON元数据 → 原始EDL数据
 *
 * @param filePath 目标文件路径
 * @return true 写入成功; false 文件打开失败或数据为空
 */
bool RecordingFileFormat::saveToFile(const QString& filePath)
{
    if (m_rawData.isEmpty()) {
        qWarning() << "[RecordingFileFormat] saveToFile: 无数据可保存";
        return false;
    }

    // --- 序列化元数据为紧凑JSON ---
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(m_metadata);
    QByteArray jsonBytes = jsonDoc.toJson(QJsonDocument::Compact);

    // 检查元数据大小上限 (quint32 最大 ~4GB，实际远小于此)
    if (static_cast<quint32>(jsonBytes.size()) > 0xFFFFFFFF) {
        qWarning() << "[RecordingFileFormat] saveToFile: 元数据过大，无法写入";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[RecordingFileFormat] saveToFile: 无法打开文件:"
                   << filePath << file.errorString();
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
        qWarning() << "[RecordingFileFormat] saveToFile: 写入元数据失败:"
                   << file.errorString();
        file.close();
        return false;
    }

    // --- 写入原始录制数据 ---
    if (file.write(m_rawData) != m_rawData.size()) {
        qWarning() << "[RecordingFileFormat] saveToFile: 写入录制数据失败:"
                   << file.errorString();
        file.close();
        return false;
    }

    file.close();
    m_filePath = filePath;
    return true;
}

/**
 * @brief 从文件加载录制数据
 *
 * 文件读取流程:
 *   1. 读取并校验12字节文件头 (魔数、版本)
 *   2. 根据头部记录的长度读取JSON元数据
 *   3. 读取剩余全部字节作为原始EDL数据
 *   4. 大文件加载时周期性发射 loadProgress 信号
 *   5. 完成后发射 loadCompleted 信号
 *
 * @param filePath 源文件路径
 * @return true 加载成功; false 文件不存在、头损坏或版本不匹配
 */
bool RecordingFileFormat::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[RecordingFileFormat] loadFromFile: 无法打开文件:"
                   << filePath << file.errorString();
        return false;
    }

    const qint64 fileSize = file.size();

    // --- 最小文件大小校验 ---
    if (fileSize < kHeaderSize) {
        qWarning() << "[RecordingFileFormat] loadFromFile: 文件过小 ("
                   << fileSize << "字节)，至少需要" << kHeaderSize << "字节";
        file.close();
        return false;
    }

    // --- 读取文件头 ---
    QByteArray header = file.read(kHeaderSize);
    if (header.size() != kHeaderSize) {
        qWarning() << "[RecordingFileFormat] loadFromFile: 读取文件头失败";
        file.close();
        return false;
    }

    // --- 校验魔数 ---
    if (!header.startsWith(kMagic)) {
        qWarning() << "[RecordingFileFormat] loadFromFile: 魔数不匹配，期望"
                   << kMagic;
        file.close();
        return false;
    }

    // --- 校验版本号 ---
    const quint8 fileVersion = static_cast<quint8>(header[3]);
    if (fileVersion != kVersion) {
        qWarning() << "[RecordingFileFormat] loadFromFile: 版本不匹配，文件版本"
                   << fileVersion << "，当前支持版本" << kVersion;
        file.close();
        return false;
    }

    // --- 解析元数据长度 (字节4-7, quint32 LE) ---
    // 手动解析以避免QDataStream对QByteArray的长度前缀处理
    quint32 metaSize = static_cast<quint8>(header[4])
                     | (static_cast<quint32>(static_cast<quint8>(header[5])) << 8)
                     | (static_cast<quint32>(static_cast<quint8>(header[6])) << 16)
                     | (static_cast<quint32>(static_cast<quint8>(header[7])) << 24);

    // --- 校验元数据区域不超出文件范围 ---
    const qint64 dataStart = kHeaderSize + static_cast<qint64>(metaSize);
    if (dataStart > fileSize) {
        qWarning() << "[RecordingFileFormat] loadFromFile: 元数据区域 (偏移"
                   << dataStart << ") 超出文件大小 (" << fileSize << ")";
        file.close();
        return false;
    }

    // --- 读取元数据JSON ---
    QByteArray jsonBytes;
    if (metaSize > 0) {
        jsonBytes = file.read(static_cast<qint64>(metaSize));
        if (static_cast<quint32>(jsonBytes.size()) != metaSize) {
            qWarning() << "[RecordingFileFormat] loadFromFile: 读取元数据不完整";
            file.close();
            return false;
        }

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonBytes);
        if (jsonDoc.isNull()) {
            qWarning() << "[RecordingFileFormat] loadFromFile: 元数据JSON解析失败";
            file.close();
            return false;
        }
        m_metadata = jsonDoc.toVariant().toMap();
    } else {
        m_metadata.clear();
    }

    // 发射进度: 元数据已解析
    emit loadProgress(0.1);

    // --- 读取剩余原始录制数据 ---
    const qint64 rawBytesRemaining = fileSize - dataStart;
    if (rawBytesRemaining < 0) {
        qWarning() << "[RecordingFileFormat] loadFromFile: 文件数据区域大小异常";
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
                qWarning() << "[RecordingFileFormat] loadFromFile: 读取录制数据不完整"
                           << "已读" << totalRead + chunk.size()
                           << "期望" << rawBytesRemaining;
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

    file.close();
    m_filePath = filePath;

    // --- 发射完成信号 ---
    emit loadProgress(1.0);
    emit loadCompleted(m_metadata);

    return true;
}
