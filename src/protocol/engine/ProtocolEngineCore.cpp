/**
 * @file ProtocolEngineCore.cpp
 * @brief 自定义协议解析引擎 - 帧解析核心循环实现
 *
 * 从 ProtocolEngine.cpp 拆分而来，包含:
 *   - feedData():          数据喂入、缓冲区溢出保护、循环帧提取入口
 *   - tryParseOneFrame():  单帧提取主循环(帧头搜索→长度读取→校验→字段提取)
 *
 * 构造/析构/schema设置/reset等生命周期方法见 ProtocolEngine.cpp。
 * 解析辅助方法(findHeader/trimBufferBeforePartialHeader/readLengthField/extractField)
 * 见 ProtocolEngineParsing.cpp。
 * 校验和计算见 ProtocolEngineChecksum.cpp。
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

#include <QDataStream>
#include <QDateTime>

/** @brief 缓冲区最大容量 64KB，防止内存膨胀(与ProtocolEngine.cpp中的定义保持一致) */
static constexpr int MAX_BUFFER_SIZE = 65536;

/** @brief 单帧最大长度限制 16KB，超过此值的帧被视为畸形帧 */
static constexpr int MAX_FRAME_LENGTH = 16384;

/** @brief 向引擎喂入新的串口数据(追加缓冲→溢出保护→循环解析) @param data 新接收到的原始字节流 */
void ProtocolEngine::feedData(const QByteArray &data)
{
    /* 追加数据到缓冲区 */
    m_buffer.append(data);
    m_totalBytesProcessed += static_cast<quint64>(data.size());
    ++m_totalBuilds;  // 累计数据注入计数

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

/** @brief 尝试从缓冲区中解析一帧(搜索帧头→读取长度→校验→字段提取) @return true成功提取一帧，false缓冲区数据不足 */
bool ProtocolEngine::tryParseOneFrame()
{
    ++m_totalParses;  // 累计帧解析尝试计数

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
    ++m_totalMatches;  // 累计帧头匹配成功计数

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

    /* 帧长度上限检查: 防止畸形帧消耗过多内存 */
    if (frameLength > MAX_FRAME_LENGTH) {
        emit parseError(tr("帧长度超出限制: %1 > %2").arg(frameLength).arg(MAX_FRAME_LENGTH));
        ++m_parseErrors;
        ++m_totalParseErrors;
        ++m_framesRejected;
        /* 丢弃整个帧头后重新搜索 */
        m_buffer.remove(0, headerBytes.size());
        return true;
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
            /* CRC校验失败: 记录帧长度和前8字节十六进制用于诊断 */
            QString frameHex = rawFrame.left(8).toHex(' ').toUpper();
            emit parseError(tr("CRC校验失败(%1): 期望=0x%2, 实际=0x%3, 帧长=%4, 头部=[%5]")
                                .arg(algoName)
                                .arg(expectedVal, 0, 16)
                                .arg(actualVal, 0, 16)
                                .arg(rawFrame.size())
                                .arg(frameHex));
            emit checksumFailed(expectedVal, actualVal, algoName);
            ++m_parseErrors;
            ++m_totalParseErrors;
            ++m_framesRejected;
            return true; /* 跳过畸形帧，继续解析后续数据 */
        }
        ++m_crcPassCount;
        ++m_totalValidationPasses;
        ++m_totalCrcChecks;
    }

    /* ---- 步骤7：解析字段(带越界保护) ---- */
    QVariantMap fields;
    const auto fieldDefs = m_schema->fields();
    for (const auto &field : fieldDefs) {
        /* 跳过偏移超出帧范围的字段定义，避免越界访问 */
        if (field.offset < 0 || field.offset >= rawFrame.size()) {
            continue;
        }
        fields[field.name] = extractField(rawFrame, field);
    }

    /* ---- 步骤8：发射信号 ---- */
    ++m_framesParsed;
    m_totalBytesParsed += static_cast<quint64>(rawFrame.size());
    m_lastParseTimestamp = QDateTime::currentMSecsSinceEpoch();
    emit frameParsed(fields, rawFrame);

    return true;
}
