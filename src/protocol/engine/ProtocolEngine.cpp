/**
 * @file ProtocolEngine.cpp
 * @brief 自定义协议解析引擎核心实现
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
 *
 * 统计接口见 ProtocolEngineStats.cpp
 * 校验和计算见 ProtocolEngineChecksum.cpp
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

#include <QDataStream>
#include <QDateTime>
#include <QtMath>

/** @brief 缓冲区最大容量 64KB，防止内存膨胀 */
static constexpr int MAX_BUFFER_SIZE = 65536;

/** @brief 构造函数 @param parent 父对象指针 */
ProtocolEngine::ProtocolEngine(QObject *parent)
    : QObject(parent)
    , m_schema(nullptr)
    , m_parseErrors(0)
    , m_checksumAlgorithm(ChecksumAlgorithm::Auto)
    , m_framesParsed(0)
    , m_framesRejected(0)
    , m_totalBytesProcessed(0)
    , m_totalValidations(0)
    , m_totalParseErrors(0)
    , m_lastParseTimestamp(0)
    , m_totalCrcErrors(0)
    , m_totalBytesParsed(0)
    , m_crcPassCount(0)
    , m_crcFailCount(0)
    , m_totalValidationPasses(0)
    , m_totalValidationFailures(0)
    , m_totalCrcChecks(0)
{
}

/** @brief 析构函数 */
ProtocolEngine::~ProtocolEngine() = default;

/** @brief 设置协议帧结构定义(绑定schema→自动reset) @param schema 协议定义对象指针 */
void ProtocolEngine::setSchema(ProtocolSchema *schema)
{
    m_schema = schema;
    reset();
}

/** @brief 向引擎喂入新的串口数据(追加缓冲→溢出保护→循环解析) @param data 新接收到的原始字节流 */
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
        ++m_totalParseErrors;
    }

    /* 循环提取完整帧 */
    while (!m_buffer.isEmpty()) {
        if (!tryParseOneFrame()) {
            break;
        }
    }
}

/** @brief 重置解析状态(清空缓冲区+重置计数器，不清除schema和算法配置) */
void ProtocolEngine::reset()
{
    m_buffer.clear();
    m_framesParsed = 0;
    m_parseErrors = 0;
    m_framesRejected = 0;
    m_totalBytesProcessed = 0;
    m_totalValidations = 0;
    m_totalParseErrors = 0;
    m_lastParseTimestamp = 0;
    m_totalCrcErrors = 0;
    m_totalBytesParsed = 0;
    m_crcPassCount = 0;
    m_crcFailCount = 0;
    m_totalValidationPasses = 0;
    m_totalValidationFailures = 0;
    m_totalCrcChecks = 0;
}

/** @brief 获取当前使用的协议定义 @return 协议定义指针，未设置时为 nullptr */
ProtocolSchema *ProtocolEngine::currentSchema() const
{
    return m_schema;
}

/* ============================================================================
 * 帧解析核心逻辑
 * ============================================================================ */

/** @brief 尝试从缓冲区中解析一帧(搜索帧头→读取长度→校验→字段提取) @return true成功提取一帧，false缓冲区数据不足 */
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
        ++m_totalParseErrors;
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
    ChecksumAlgorithm effectiveAlgo = resolveEffectiveAlgorithm(framing);
    if (effectiveAlgo != ChecksumAlgorithm::None) {
        quint64 expectedVal = 0;
        quint64 actualVal = 0;
        bool checksumValid = validateChecksum(rawFrame, framing, &expectedVal, &actualVal);
        ++m_totalValidations;
        if (!checksumValid) {
            ++m_crcFailCount;
            ++m_totalCrcErrors;
            ++m_totalValidationFailures;
            ++m_totalCrcChecks;
            QString algoName = checksumAlgorithmToString(effectiveAlgo);
            emit parseError(tr("帧校验失败(%1): 期望=0x%2, 实际=0x%3")
                                .arg(algoName)
                                .arg(expectedVal, 0, 16)
                                .arg(actualVal, 0, 16));
            emit checksumFailed(expectedVal, actualVal, algoName);
            ++m_parseErrors;
            ++m_totalParseErrors;
            ++m_framesRejected;
            return true;
        }
        ++m_crcPassCount;
        ++m_totalValidationPasses;
        ++m_totalCrcChecks;
    }

    /* ---- 步骤7：解析字段 ---- */
    QVariantMap fields;
    const auto fieldDefs = m_schema->fields();
    for (const auto &field : fieldDefs) {
        fields[field.name] = extractField(rawFrame, field);
    }

    /* ---- 步骤8：发射信号 ---- */
    ++m_framesParsed;
    m_totalBytesParsed += static_cast<quint64>(rawFrame.size());
    m_lastParseTimestamp = QDateTime::currentMSecsSinceEpoch();
    emit frameParsed(fields, rawFrame);

    return true;
}

/** @brief 在缓冲区中查找帧头字节序列 @param buffer 待搜索的缓冲区 @param header 帧头字节序列 @return 帧头起始位置，未找到返回-1 */
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

/** @brief 修剪缓冲区，保留末尾可能构成部分帧头的字节 @param header 帧头字节序列 */
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

/** @brief 从帧数据中读取长度字段(小端序，支持1/2/4字节) @param buffer 帧数据缓冲区 @param offset 长度字段偏移 @param size 长度字段字节数 @return 解析得到的长度值 */
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

/** @brief 从帧数据中提取单个字段值(支持uint8/uint16/int16/uint32/int32/float/double) @param frame 完整的原始帧数据 @param field 字段定义 @return 提取到的字段值，越界时返回空QByteArray */
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
