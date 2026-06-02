/**
 * @file DataLoggerEdl.cpp
 * @brief 数据录制/回放管理器 — EDL二进制格式的底层读写原语
 *
 * 将EDL(EmbedDebug Log)格式的文件头写入、记录写入、记录读取等
 * 纯I/O操作从DataLogger.cpp拆分出来，保持主文件行数在500行以内。
 */
#include "utils/log/DataLogger.h"
#include <QDataStream>

// ---- EDL文件格式底层读写 ----

/** @brief 写入日志文件头(magic+version+recordCount占位) */
void DataLogger::writeHeader()
{
    if (!m_recordFile) return;

    QDataStream stream(m_recordFile);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_6_8);

    // Magic
    stream.writeRawData(kMagic, 3);
    // Version
    stream << kVersion;
    // Record count (placeholder, updated on close)
    stream << static_cast<quint32>(0);
}

/**
 * @brief 写入一条数据记录(时间戳+方向+数据长度+数据)
 * @param timestamp 相对起始时间的毫秒偏移
 * @param dir 数据方向
 * @param data 原始字节
 */
void DataLogger::writeRecord(quint64 timestamp, Direction dir, const QByteArray& data)
{
    if (!m_recordFile) return;

    QDataStream stream(m_recordFile);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_6_8);

    // Timestamp (8 bytes)
    stream << timestamp;
    // Direction (1 byte)
    stream << static_cast<quint8>(dir);
    // Length (4 bytes)
    stream << static_cast<quint32>(data.size());
    // Data
    stream.writeRawData(data.constData(), data.size());
}

/** @brief 从回放文件中读取下一条记录 @param header 输出记录头 @param data 输出记录数据 @return true=读取成功, false=EOF或错误 */
bool DataLogger::readNextRecord(RecordHeader& header, QByteArray& data)
{
    if (!m_playbackFile || m_playbackFile->atEnd()) return false;

    QDataStream stream(m_playbackFile);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_6_8);

    // Timestamp
    stream >> header.timestamp;
    if (stream.status() != QDataStream::Ok) return false;

    // Direction
    stream >> header.direction;
    if (stream.status() != QDataStream::Ok) return false;

    // Length
    quint32 length = 0;
    stream >> length;
    if (stream.status() != QDataStream::Ok) return false;

    // Data
    // 防御性校验: 拒绝异常大的记录长度（上限1MB），防止恶意/损坏的 .edl 文件导致崩溃
    if (length > 1024 * 1024) {
        qWarning() << "DataLogger: record too large:" << length;
        return false;
    }
    data.resize(static_cast<int>(length));
    if (stream.readRawData(data.data(), static_cast<int>(length)) != static_cast<int>(length)) {
        return false;
    }

    return true;
}
