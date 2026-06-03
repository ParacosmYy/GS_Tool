/**
 * @file ProtocolEngine.cpp
 * @brief 自定义协议解析引擎实现
 *
 * 接收原始串口字节流，根据 ProtocolSchema 定义的帧格式
 * 自动完成帧同步、长度解析、字段提取。
 *
 * 解析流程：
 * 1. 数据追加到内部缓冲区
 * 2. 在缓冲区中搜索帧头
 * 3. 读取长度字段，判断帧是否完整
 * 4. 提取完整帧，按字段定义解析各字段值
 * 5. 发射 frameParsed 信号
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

#include <QDataStream>
#include <QDateTime>
#include <QtMath>

/** @brief 缓冲区最大容量 64KB，防止内存膨胀 */
static constexpr int MAX_BUFFER_SIZE = 65536;

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
ProtocolEngine::ProtocolEngine(QObject *parent)
    : QObject(parent)
    , m_schema(nullptr)
    , m_parseErrors(0)
    , m_framesParsed(0)
    , m_framesRejected(0)
    , m_totalBytesProcessed(0)
    , m_lastParseTimestamp(0)
{
}

/**
 * @brief 析构函数
 */
ProtocolEngine::~ProtocolEngine() = default;

/**
 * @brief 设置协议帧结构定义
 *
 * 将引擎绑定到指定的 ProtocolSchema，后续 feedData() 调用
 * 将根据该定义进行帧解析。传入 nullptr 可清除当前定义。
 * 设置后会自动调用 reset() 清空缓冲区和计数器。
 *
 * @param schema 协议定义对象指针
 */
void ProtocolEngine::setSchema(ProtocolSchema *schema)
{
    m_schema = schema;
    reset();
}

/**
 * @brief 向引擎喂入新的串口数据
 *
 * 将数据追加到内部缓冲区，然后尝试按当前 schema
 * 定义的帧格式进行帧同步与解析。解析成功后发射
 * frameParsed() 信号，失败时发射 parseError()。
 *
 * 支持连续解析：一次喂入的数据中可能包含多个完整帧，
 * 本方法会循环提取直到缓冲区中不再有完整帧。
 *
 * @param data 新接收到的原始字节流
 */
void ProtocolEngine::feedData(const QByteArray &data)
{
    /* 追加数据到缓冲区 */
    m_buffer.append(data);
    m_totalBytesProcessed += static_cast<quint64>(data.size());

    /* 无 schema 或 schema 无效时直接返回 */
    if (!m_schema || !m_schema->isValid()) {
        return;
    }

    /* 缓冲区溢出保护：超过上限时丢弃前半部分 */
    if (m_buffer.size() > MAX_BUFFER_SIZE) {
        int discardBytes = m_buffer.size() / 2;
        m_buffer.remove(0, discardBytes);
        emit parseError(tr("缓冲区溢出，已丢弃前 %1 字节").arg(discardBytes));
        ++m_parseErrors;
    }

    /* 循环提取完整帧 */
    while (!m_buffer.isEmpty()) {
        if (!tryParseOneFrame()) {
            break;
        }
    }
}

/**
 * @brief 重置解析状态
 *
 * 清空内部接收缓冲区，重置解析统计计数器。
 * 不会清除当前 schema 设置。
 */
void ProtocolEngine::reset()
{
    m_buffer.clear();
    m_framesParsed = 0;
    m_parseErrors = 0;
    m_framesRejected = 0;
    m_totalBytesProcessed = 0;
    m_lastParseTimestamp = 0;
}

/**
 * @brief 获取当前使用的协议定义
 * @return 协议定义指针，未设置时为 nullptr
 */
ProtocolSchema *ProtocolEngine::currentSchema() const
{
    return m_schema;
}

/**
 * @brief 获取已成功解析的帧数
 * @return 成功解析帧计数
 */
int ProtocolEngine::framesParsed() const
{
    return static_cast<int>(m_framesParsed);
}

/**
 * @brief 获取解析错误次数
 * @return 解析错误计数
 */
int ProtocolEngine::parseErrors() const
{
    return m_parseErrors;
}

/**
 * @brief 获取已成功解析的帧数（64位）
 * @return 成功解析帧计数
 */
quint64 ProtocolEngine::framesParsedCount() const
{
    return m_framesParsed;
}

/**
 * @brief 获取因验证失败而被拒绝的帧数
 * @return 被拒绝帧计数
 */
quint64 ProtocolEngine::framesRejected() const
{
    return m_framesRejected;
}

/**
 * @brief 获取引擎处理的总字节数
 * @return 累计处理的字节总数
 */
quint64 ProtocolEngine::totalBytesProcessed() const
{
    return m_totalBytesProcessed;
}

/**
 * @brief 获取最后一次成功解析的时间戳
 * @return 毫秒级时间戳（自Unix纪元起），尚未解析过时返回0
 */
qint64 ProtocolEngine::lastParseTimestamp() const
{
    return m_lastParseTimestamp;
}

/**
 * @brief 重置所有解析统计计数器
 *
 * 将帧计数、拒绝计数、字节总数和时间戳全部归零。
 * 不影响当前 schema 设置和缓冲区内容。
 */
void ProtocolEngine::resetParseStatistics()
{
    m_framesParsed = 0;
    m_parseErrors = 0;
    m_framesRejected = 0;
    m_totalBytesProcessed = 0;
    m_lastParseTimestamp = 0;
}

/* ============================================================================
 * 私有方法实现
 * ============================================================================ */

/**
 * @brief 尝试从缓冲区中解析一帧
 *
 * 按以下步骤处理：
 * 1. 在缓冲区中查找帧头
 * 2. 检查长度字段是否已接收完整
 * 3. 读取帧长度，检查整帧数据是否完整
 * 4. 提取帧数据，解析各字段
 * 5. 发射信号，从缓冲区移除已处理字节
 *
 * @return true 成功提取一帧，false 缓冲区数据不足以构成完整帧
 */
bool ProtocolEngine::tryParseOneFrame()
{
    const auto framing = m_schema->framing();
    const auto &headerBytes = framing.header;

    /* ---- 步骤1：在缓冲区中查找帧头 ---- */
    int headerPos = findHeader(m_buffer, headerBytes);
    if (headerPos < 0) {
        /* 未找到帧头，但保留最后几个字节（可能是部分帧头） */
        trimBufferBeforePartialHeader(headerBytes);
        return false;
    }

    /* 丢弃帧头之前的垃圾数据 */
    if (headerPos > 0) {
        m_buffer.remove(0, headerPos);
    }

    /* ---- 步骤2：检查长度字段是否已接收 ---- */
    int lengthFieldEnd = framing.lengthFieldOffset + framing.lengthFieldSize;
    if (m_buffer.size() < lengthFieldEnd) {
        /* 长度字段尚未完整接收，等待更多数据 */
        return false;
    }

    /* ---- 步骤3：读取帧长度（小端序） ---- */
    int frameLength = readLengthField(m_buffer, framing.lengthFieldOffset,
                                      framing.lengthFieldSize);

    /* 长度值合理性检查 */
    if (frameLength <= 0) {
        emit parseError(tr("帧长度无效: %1").arg(frameLength));
        ++m_parseErrors;
        ++m_framesRejected;
        /* 跳过当前帧头的第一个字节，重新搜索 */
        m_buffer.remove(0, 1);
        return true; /* 继续尝试解析下一帧 */
    }

    /* ---- 步骤4：检查整帧数据是否完整 ---- */
    if (m_buffer.size() < frameLength) {
        /* 帧数据尚未完整接收 */
        return false;
    }

    /* ---- 步骤5：提取完整帧 ---- */
    QByteArray rawFrame = m_buffer.left(frameLength);
    m_buffer.remove(0, frameLength);

    /* ---- 步骤6：校验和/CRC验证 ---- */
    if (framing.checksumType != ProtocolSchema::ChecksumType::None) {
        bool checksumValid = validateChecksum(rawFrame, framing);
        if (!checksumValid) {
            emit parseError(tr("帧校验失败"));
            ++m_parseErrors;
            ++m_framesRejected;
            return true;
        }
    }

    /* ---- 步骤7：解析字段 ---- */
    QVariantMap fields;
    const auto fieldDefs = m_schema->fields();
    for (const auto &field : fieldDefs) {
        fields[field.name] = extractField(rawFrame, field);
    }

    /* ---- 步骤8：发射信号 ---- */
    ++m_framesParsed;
    m_lastParseTimestamp = QDateTime::currentMSecsSinceEpoch();
    emit frameParsed(fields, rawFrame);

    return true;
}

/**
 * @brief 在缓冲区中查找帧头字节序列
 *
 * 逐字节扫描缓冲区，匹配帧头的完整字节序列。
 *
 * @param buffer 待搜索的缓冲区
 * @param header 帧头字节序列
 * @return 帧头起始位置，未找到返回 -1
 */
int ProtocolEngine::findHeader(const QByteArray &buffer,
                                const QVector<int> &header) const
{
    if (header.isEmpty() || buffer.size() < header.size()) {
        return -1;
    }

    int searchLimit = buffer.size() - header.size() + 1;
    for (int i = 0; i < searchLimit; ++i) {
        bool match = true;
        for (int j = 0; j < header.size(); ++j) {
            if (static_cast<quint8>(buffer.at(i + j)) !=
                static_cast<quint8>(header.at(j))) {
                match = false;
                break;
            }
        }
        if (match) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 修剪缓冲区，丢弃不可能是帧头前缀的垃圾数据
 *
 * 当未找到完整帧头时，保留缓冲区末尾可能构成部分帧头的字节。
 * 其余字节全部丢弃。
 *
 * @param header 帧头字节序列
 */
void ProtocolEngine::trimBufferBeforePartialHeader(const QVector<int> &header)
{
    if (header.isEmpty() || m_buffer.isEmpty()) {
        return;
    }

    int maxKeep = header.size() - 1;
    if (maxKeep <= 0) {
        /* 帧头只有1字节或空，无法有部分匹配 */
        m_buffer.clear();
        return;
    }

    /* 从缓冲区末尾向前查找，看是否有部分帧头 */
    int bufferSize = m_buffer.size();
    int keepBytes = 0;

    for (int keep = 1; keep <= qMin(maxKeep, bufferSize); ++keep) {
        int startIdx = bufferSize - keep;
        bool partialMatch = true;
        for (int j = 0; j < keep; ++j) {
            if (static_cast<quint8>(m_buffer.at(startIdx + j)) !=
                static_cast<quint8>(header.at(j))) {
                partialMatch = false;
                break;
            }
        }
        if (partialMatch) {
            keepBytes = keep;
        }
    }

    /* 只保留可能是部分帧头的末尾字节 */
    int discardCount = bufferSize - keepBytes;
    if (discardCount > 0) {
        m_buffer.remove(0, discardCount);
    }
}

/**
 * @brief 从帧数据中读取长度字段（小端序）
 *
 * 按指定偏移和字节数从缓冲区中提取小端序整数。
 * 支持 1/2/4 字节长度字段。
 *
 * @param buffer 帧数据缓冲区
 * @param offset 长度字段偏移
 * @param size 长度字段字节数
 * @return 解析得到的长度值
 */
int ProtocolEngine::readLengthField(const QByteArray &buffer,
                                     int offset, int size) const
{
    if (size <= 0 || offset < 0 || (offset + size) > buffer.size()) {
        return -1;
    }

    quint32 value = 0;
    for (int i = 0; i < size; ++i) {
        value |= (static_cast<quint8>(buffer.at(offset + i)) << (8 * i));
    }
    return static_cast<int>(value);
}

/**
 * @brief 从帧数据中提取单个字段值
 *
 * 根据字段定义的偏移、大小和类型，从原始帧数据中提取
 * 对应的字节并转换为 QVariant。支持多种整数、浮点类型。
 *
 * @param frame 完整的原始帧数据
 * @param field 字段定义
 * @return 提取到的字段值，越界或类型未知时返回空 QByteArray
 */
QVariant ProtocolEngine::extractField(const QByteArray &frame,
                                       const ProtocolSchema::FieldDefinition &field) const
{
    /* 边界检查：字段是否在帧范围内 */
    if (field.offset < 0 || field.size <= 0 ||
        (field.offset + field.size) > frame.size()) {
        return QVariant(QByteArray());
    }

    QByteArray fieldBytes = frame.mid(field.offset, field.size);

    /* 按类型解析 */
    if (field.type == QLatin1String("uint8")) {
        return QVariant(static_cast<quint8>(fieldBytes.at(0)));
    }
    if (field.type == QLatin1String("uint16_le")) {
        return QVariant(static_cast<quint16>(
            static_cast<quint8>(fieldBytes.at(0)) |
            (static_cast<quint8>(fieldBytes.at(1)) << 8)));
    }
    if (field.type == QLatin1String("int16_le")) {
        quint16 raw = static_cast<quint16>(
            static_cast<quint8>(fieldBytes.at(0)) |
            (static_cast<quint8>(fieldBytes.at(1)) << 8));
        return QVariant(static_cast<qint16>(raw));
    }
    if (field.type == QLatin1String("uint32_le")) {
        quint32 val = 0;
        val |= static_cast<quint8>(fieldBytes.at(0));
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(1))) << 8;
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(2))) << 16;
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(3))) << 24;
        return QVariant(val);
    }
    if (field.type == QLatin1String("int32_le")) {
        quint32 val = 0;
        val |= static_cast<quint8>(fieldBytes.at(0));
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(1))) << 8;
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(2))) << 16;
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(3))) << 24;
        return QVariant(static_cast<qint32>(val));
    }
    if (field.type == QLatin1String("float_le")) {
        QDataStream ds(fieldBytes);
        ds.setByteOrder(QDataStream::LittleEndian);
        float val = 0.0f;
        ds >> val;
        return QVariant(val);
    }
    if (field.type == QLatin1String("double_le")) {
        QDataStream ds(fieldBytes);
        ds.setByteOrder(QDataStream::LittleEndian);
        double val = 0.0;
        ds >> val;
        return QVariant(val);
    }
    /* bytes 类型或未知类型：返回原始字节 */
    return QVariant(fieldBytes);
}

/**
 * @brief 验证帧的校验和/CRC
 *
 * 根据framing中指定的校验类型，从帧数据中提取校验字段
 * 并与计算值比较。校验字段位于帧末尾，长度取决于校验类型。
 *
 * @param frame 完整帧数据(含校验字段)
 * @param framing 帧格式定义(含校验类型和校验字段偏移)
 * @return true=校验通过, false=校验失败
 */
bool ProtocolEngine::validateChecksum(const QByteArray &frame,
                                       const ProtocolSchema::FramingRule &framing) const
{
    if (frame.isEmpty()) { return false; }

    /* 校验字段大小: CRC8/XOR=1, CRC16=2, CRC32=4 */
    int checksumSize = 0;
    switch (framing.checksumType) {
    case ProtocolSchema::ChecksumType::None:       return true;
    case ProtocolSchema::ChecksumType::Crc8:       checksumSize = 1; break;
    case ProtocolSchema::ChecksumType::Xor:        checksumSize = 1; break;
    case ProtocolSchema::ChecksumType::Crc16Ccitt: checksumSize = 2; break;
    case ProtocolSchema::ChecksumType::Crc16Modbus: checksumSize = 2; break;
    case ProtocolSchema::ChecksumType::Crc32:      checksumSize = 4; break;
    default: return true;
    }

    /* 帧必须至少包含校验字段 */
    if (frame.size() < checksumSize) { return false; }

    /* 分离: 数据部分(不含校验) 和 校验字段 */
    QByteArray payload = frame.left(frame.size() - checksumSize);
    QByteArray checksumBytes = frame.right(checksumSize);

    switch (framing.checksumType) {
    case ProtocolSchema::ChecksumType::Crc8: {
        quint8 expected = static_cast<quint8>(checksumBytes.at(0));
        return computeCrc8(payload) == expected;
    }
    case ProtocolSchema::ChecksumType::Xor: {
        quint8 expected = static_cast<quint8>(checksumBytes.at(0));
        return computeXor(payload) == expected;
    }
    case ProtocolSchema::ChecksumType::Crc16Ccitt: {
        quint16 expected = static_cast<quint16>(
            (static_cast<quint8>(checksumBytes.at(0))) |
            (static_cast<quint16>(static_cast<quint8>(checksumBytes.at(1))) << 8));
        return computeCrc16Ccitt(payload) == expected;
    }
    case ProtocolSchema::ChecksumType::Crc16Modbus: {
        quint16 expected = static_cast<quint16>(
            (static_cast<quint8>(checksumBytes.at(0))) |
            (static_cast<quint16>(static_cast<quint8>(checksumBytes.at(1))) << 8));
        return computeCrc16Modbus(payload) == expected;
    }
    case ProtocolSchema::ChecksumType::Crc32: {
        quint32 expected = 0;
        for (int i = 0; i < 4; ++i) {
            expected |= static_cast<quint32>(static_cast<quint8>(checksumBytes.at(i))) << (8 * i);
        }
        return computeCrc32(payload) == expected;
    }
    default: return true;
    }
}

/**
 * @brief 计算CRC-8校验值
 *
 * 使用多项式0x07 (CRC-8/ITU标准)。
 * 初始值0x00，无输入/输出反转。
 *
 * @param data 待校验数据
 * @param polynomial CRC多项式(默认0x07)
 * @return CRC-8校验值
 */
quint8 ProtocolEngine::computeCrc8(const QByteArray &data, quint8 polynomial)
{
    quint8 crc = 0x00;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data.at(i));
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief 计算CRC-16 CCITT校验值
 *
 * 多项式0x1021，初始值0xFFFF，无输入/输出反转。
 * 常用于XMODEM/CRC-CCITT协议。
 *
 * @param data 待校验数据
 * @return CRC-16 CCITT校验值
 */
quint16 ProtocolEngine::computeCrc16Ccitt(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= (static_cast<quint16>(static_cast<quint8>(data.at(i))) << 8);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief 计算CRC-16 Modbus校验值
 *
 * 多项式0x8005，初始值0xFFFF，输入反转+输出反转。
 * Modbus RTU协议标准校验算法。
 *
 * @param data 待校验数据
 * @return CRC-16 Modbus校验值(小端序存储: 低字节在前)
 */
quint16 ProtocolEngine::computeCrc16Modbus(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data.at(i));
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief 计算CRC-32校验值
 *
 * 使用标准CRC-32多项式0xEDB88320(反转形式)。
 * 初始值0xFFFFFFFF，输出异或0xFFFFFFFF。
 * 兼容ZIP/PNG等标准的CRC-32。
 *
 * @param data 待校验数据
 * @return CRC-32校验值
 */
quint32 ProtocolEngine::computeCrc32(const QByteArray &data)
{
    quint32 crc = 0xFFFFFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data.at(i));
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x00000001) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

/**
 * @brief 计算异或校验值
 *
 * 对所有字节逐个异或，结果为单字节校验值。
 *
 * @param data 待校验数据
 * @return 异或校验结果
 */
quint8 ProtocolEngine::computeXor(const QByteArray &data)
{
    quint8 result = 0x00;
    for (int i = 0; i < data.size(); ++i) {
        result ^= static_cast<quint8>(data.at(i));
    }
    return result;
}
