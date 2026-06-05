/**
 * @file SerialProtocolFuzzer.cpp
 * @brief 串口协议模糊测试器核心实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 包含六种变异策略的具体实现、确定性 PRNG、CSV 导出和迭代器逻辑。
 * 统计查询与重置方法见 @see SerialProtocolFuzzerStats.cpp
 */

#include "utils/fuzzer/SerialProtocolFuzzer.h"

#include <QDateTime>
#include <QStringList>

// ──────────────────────────── 构造 ────────────────────────────

/** @brief 构造串口协议模糊测试器，设置 objectName 供 QSS 使用 @param parent 父对象 */
SerialProtocolFuzzer::SerialProtocolFuzzer(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SerialProtocolFuzzer"));
    /* 使用配置中的种子初始化 PRNG */
    m_rngState = m_config.randomSeed;
    if (m_rngState == 0) {
        m_rngState = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch()) & 0xFFFFFFFF;
    }
}

// ──────────────────────────── PRNG ────────────────────────────

/**
 * @brief Xorshift32 伪随机数生成器
 *
 * 经典 32 位 Xorshift 算法，周期 2^32 - 1，
 * 速度快且跨平台确定性一致。
 *
 * @param max 返回值上界(不含)，max=0 时不限制
 * @return [0, max) 范围内的伪随机数
 */
quint32 SerialProtocolFuzzer::randomUInt(quint32 max)
{
    /* Xorshift32 核心 */
    m_rngState ^= m_rngState << 13;
    m_rngState ^= m_rngState >> 17;
    m_rngState ^= m_rngState << 5;

    if (max == 0) {
        return m_rngState;
    }
    return m_rngState % max;
}

/** @brief 生成一个 [0, 255] 范围内的随机字节 @return 随机字节值 */
quint8 SerialProtocolFuzzer::randomByte()
{
    return static_cast<quint8>(randomUInt(256));
}

/** @brief 从常见边界测试值池中随机选取一个 */
quint8 SerialProtocolFuzzer::boundaryValue()
{
    /* 边界值池: 0/MAX/MIN/哨兵值/off-by-one */
    static const quint8 kBoundaryPool[] = {
        0x00,       ///< 零值
        0x01,       ///< 最小正整数
        0x7F,       ///< INT8_MAX
        0x80,       ///< INT8_MIN(补码)
        0xFE,       ///< MAX-1
        0xFF,       ///< MAX
        0x55,       ///< 交替位 01010101
        0xAA,       ///< 交替位 10101010
        0x0F,       ///< 低四位全1
        0xF0,       ///< 高四位全1
    };
    constexpr int poolSize = sizeof(kBoundaryPool) / sizeof(kBoundaryPool[0]);
    return kBoundaryPool[randomUInt(poolSize)];
}

// ──────────────────────────── 配置 ────────────────────────────

/** @brief 设置模糊测试配置
 *
 * 配置变更后自动重置迭代器和 PRNG 种子，
 * 确保相同配置产生完全一致的输出序列。
 *
 * @param config 新的配置结构体
 */
void SerialProtocolFuzzer::setConfig(const FuzzConfig &config)
{
    m_config = config;
    m_currentIndex = 0;

    /* 重新初始化 PRNG */
    m_rngState = m_config.randomSeed;
    if (m_rngState == 0) {
        m_rngState = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch()) & 0xFFFFFFFF;
    }
}

/** @brief 获取当前配置 @return 配置结构体只读引用 */
const SerialProtocolFuzzer::FuzzConfig &SerialProtocolFuzzer::config() const
{
    return m_config;
}

// ──────────────────────────── 迭代器 ────────────────────────────

/** @brief 获取当前迭代位置(从0开始) @return 当前迭代索引 */
int SerialProtocolFuzzer::currentIteration() const
{
    return m_currentIndex;
}

/** @brief 是否还有未生成的迭代 @return true=仍有剩余 */
bool SerialProtocolFuzzer::hasMore() const
{
    return m_currentIndex < m_config.iterationCount;
}

// ──────────────────────────── 策略: RandomBytes ────────────────────────────

/**
 * @brief 生成完全随机的字节序列
 *
 * 使用模板数据长度作为目标长度(无模板时默认 16 字节)，
 * 每个字节独立随机生成。
 */
SerialProtocolFuzzer::FuzzResult SerialProtocolFuzzer::doRandomBytes()
{
    FuzzResult result;
    int len = m_config.templateData.isEmpty()
                  ? 16
                  : m_config.templateData.size();

    /* 限制最大长度 */
    len = qMin(len, kMaxDataSize);
    if (len <= 0) {
        len = 16;
    }

    result.fuzzedData.resize(len);
    for (int i = 0; i < len; ++i) {
        result.fuzzedData[i] = static_cast<char>(randomByte());
        result.mutatedOffsets.append(i);
    }

    result.description = tr("RandomBytes: %1 字节完全随机数据").arg(len);
    return result;
}

// ──────────────────────────── 策略: BitFlip ────────────────────────────

/**
 * @brief 对模板数据进行随机位翻转
 *
 * 在模板数据副本上随机选取最多 maxMutationRate 个位置，
 * 每个位置翻转一个随机位(0->1 或 1->0)。
 */
SerialProtocolFuzzer::FuzzResult SerialProtocolFuzzer::doBitFlip()
{
    FuzzResult result;
    result.fuzzedData = m_config.templateData;

    if (result.fuzzedData.isEmpty()) {
        result.description = tr("BitFlip: 模板数据为空，跳过");
        return result;
    }

    int mutations = qMin(m_config.maxMutationRate, result.fuzzedData.size());

    for (int i = 0; i < mutations; ++i) {
        int offset = static_cast<int>(randomUInt(static_cast<quint32>(result.fuzzedData.size())));
        int bit = static_cast<int>(randomUInt(8));

        /* 翻转指定位 */
        char &byte = result.fuzzedData[offset];
        byte = static_cast<char>(static_cast<quint8>(byte) ^ (1 << bit));

        if (!result.mutatedOffsets.contains(offset)) {
            result.mutatedOffsets.append(offset);
        }
    }

    result.description = tr("BitFlip: 翻转 %1 个位，涉及 %2 个字节")
                             .arg(mutations)
                             .arg(result.mutatedOffsets.size());
    return result;
}

// ──────────────────────────── 策略: ByteFlip ────────────────────────────

/**
 * @brief 将随机字节替换为边界值
 *
 * 在模板数据副本上随机选取最多 maxMutationRate 个位置，
 * 将每个位置的值替换为从边界值池中随机选取的值。
 */
SerialProtocolFuzzer::FuzzResult SerialProtocolFuzzer::doByteFlip()
{
    FuzzResult result;
    result.fuzzedData = m_config.templateData;

    if (result.fuzzedData.isEmpty()) {
        result.description = tr("ByteFlip: 模板数据为空，跳过");
        return result;
    }

    int mutations = qMin(m_config.maxMutationRate, result.fuzzedData.size());

    for (int i = 0; i < mutations; ++i) {
        int offset = static_cast<int>(randomUInt(static_cast<quint32>(result.fuzzedData.size())));
        quint8 newVal = boundaryValue();

        result.fuzzedData[offset] = static_cast<char>(newVal);

        if (!result.mutatedOffsets.contains(offset)) {
            result.mutatedOffsets.append(offset);
        }
    }

    result.description = tr("ByteFlip: 替换 %1 个字节为边界值")
                             .arg(result.mutatedOffsets.size());
    return result;
}

// ──────────────────────────── 策略: BoundaryValues ────────────────────────────

/**
 * @brief 用常见边界测试值填充整个数据
 *
 * 遍历模板数据每个字节位置，用从边界值池中选取的值替换，
 * 适用于测试协议解析器对极端值的处理能力。
 */
SerialProtocolFuzzer::FuzzResult SerialProtocolFuzzer::doBoundaryValues()
{
    FuzzResult result;
    result.fuzzedData = m_config.templateData;

    if (result.fuzzedData.isEmpty()) {
        /* 无模板时生成 8 字节边界值序列 */
        result.fuzzedData.resize(8);
        for (int i = 0; i < 8; ++i) {
            result.fuzzedData[i] = static_cast<char>(boundaryValue());
        }
        result.description = tr("BoundaryValues: 生成 %1 字节边界值(无模板)")
                                 .arg(result.fuzzedData.size());
        for (int i = 0; i < result.fuzzedData.size(); ++i) {
            result.mutatedOffsets.append(i);
        }
        return result;
    }

    for (int i = 0; i < result.fuzzedData.size(); ++i) {
        result.fuzzedData[i] = static_cast<char>(boundaryValue());
        result.mutatedOffsets.append(i);
    }

    result.description = tr("BoundaryValues: 全部 %1 字节替换为边界值")
                             .arg(result.fuzzedData.size());
    return result;
}

// ──────────────────────────── 策略: LengthFuzz ────────────────────────────

/**
 * @brief 变异数据长度
 *
 * 生成五种不同长度的数据变体之一:
 *   - 空数据(0字节)
 *   - 单字节
 *   - 原长度-1
 *   - 原长度
 *   - 原长度+1
 * 填充内容来自模板或随机字节。
 */
SerialProtocolFuzzer::FuzzResult SerialProtocolFuzzer::doLengthFuzz()
{
    FuzzResult result;

    int baseLen = m_config.templateData.isEmpty()
                      ? 32
                      : m_config.templateData.size();

    /* 五种长度变体 */
    int lengths[] = {0, 1, qMax(baseLen - 1, 0), baseLen, baseLen + 1};
    int choice = static_cast<int>(randomUInt(5));
    int targetLen = lengths[choice];

    /* 限制最大长度 */
    targetLen = qMin(targetLen, kMaxDataSize);

    if (targetLen == 0) {
        result.description = tr("LengthFuzz: 空数据(0字节)");
        return result;
    }

    result.fuzzedData.resize(targetLen);

    /* 优先从模板复制，超出部分用随机字节填充 */
    int copyLen = qMin(targetLen, m_config.templateData.size());
    if (copyLen > 0) {
        memcpy(result.fuzzedData.data(), m_config.templateData.constData(),
               static_cast<size_t>(copyLen));
    }
    for (int i = copyLen; i < targetLen; ++i) {
        result.fuzzedData[i] = static_cast<char>(randomByte());
        result.mutatedOffsets.append(i);
    }

    /* 记录截断/扩展的边界偏移 */
    if (targetLen < baseLen) {
        result.mutatedOffsets.append(targetLen - 1);
    } else if (targetLen > baseLen) {
        result.mutatedOffsets.append(baseLen);
    }

    result.description = tr("LengthFuzz: 长度 %1 -> %2 (变体 %3)")
                             .arg(baseLen)
                             .arg(targetLen)
                             .arg(choice);
    return result;
}

// ──────────────────────────── 策略: FieldFuzz ────────────────────────────

/**
 * @brief 仅对指定偏移范围的字段进行变异
 *
 * 使用 fuzzFields 列表中指定的字节偏移进行定向变异，
 * 其他字节保持模板原值。适用于已知协议字段布局的定向测试。
 */
SerialProtocolFuzzer::FuzzResult SerialProtocolFuzzer::doFieldFuzz()
{
    FuzzResult result;
    result.fuzzedData = m_config.templateData;

    if (result.fuzzedData.isEmpty() || m_config.fuzzFields.isEmpty()) {
        result.description = tr("FieldFuzz: 模板或字段列表为空，跳过");
        return result;
    }

    int dataSize = result.fuzzedData.size();

    for (int offset : m_config.fuzzFields) {
        if (offset < 0 || offset >= dataSize) {
            continue;
        }

        /* 对每个指定偏移应用随机变异(边界值或随机字节) */
        if (randomUInt(2) == 0) {
            result.fuzzedData[offset] = static_cast<char>(boundaryValue());
        } else {
            result.fuzzedData[offset] = static_cast<char>(randomByte());
        }
        result.mutatedOffsets.append(offset);
    }

    result.description = tr("FieldFuzz: 变异 %1/%2 个指定偏移")
                             .arg(result.mutatedOffsets.size())
                             .arg(m_config.fuzzFields.size());
    return result;
}

// ──────────────────────────── 核心生成 ────────────────────────────

/**
 * @brief 生成单条模糊测试数据
 *
 * 根据当前配置的策略分发到对应的变异方法，
 * 更新迭代索引和统计计数器，发射 fuzzGenerated 信号。
 *
 * @return 变异结果，配置无效时返回空结果
 */
SerialProtocolFuzzer::FuzzResult SerialProtocolFuzzer::generate()
{
    FuzzResult result;

    switch (m_config.strategy) {
    case FuzzStrategy::RandomBytes:
        result = doRandomBytes();
        break;
    case FuzzStrategy::BitFlip:
        result = doBitFlip();
        break;
    case FuzzStrategy::ByteFlip:
        result = doByteFlip();
        break;
    case FuzzStrategy::BoundaryValues:
        result = doBoundaryValues();
        break;
    case FuzzStrategy::LengthFuzz:
        result = doLengthFuzz();
        break;
    case FuzzStrategy::FieldFuzz:
        result = doFieldFuzz();
        break;
    }

    /* 更新迭代序号和统计 */
    m_currentIndex++;
    result.iteration = m_currentIndex;

    m_totalIterations++;
    m_totalBytesGenerated += static_cast<quint64>(result.fuzzedData.size());
    m_totalMutations += static_cast<quint64>(result.mutatedOffsets.size());
    m_totalFuzzFields += static_cast<quint64>(m_config.fuzzFields.size());

    emit fuzzGenerated(result);
    return result;
}

/**
 * @brief 批量生成指定数量的模糊测试数据
 *
 * @param count 生成条数(0=使用配置中的 iterationCount)
 * @return 结果列表
 */
QList<SerialProtocolFuzzer::FuzzResult> SerialProtocolFuzzer::generateBatch(int count)
{
    if (count <= 0) {
        count = m_config.iterationCount;
    }

    QList<FuzzResult> results;
    results.reserve(count);

    for (int i = 0; i < count; ++i) {
        results.append(generate());
    }

    emit batchComplete(count);
    return results;
}

/**
 * @brief 迭代器式获取下一条结果
 *
 * 基于 m_currentIndex 递进，到达 iterationCount 后 hasMore() 返回 false。
 *
 * @return 变异结果
 */
SerialProtocolFuzzer::FuzzResult SerialProtocolFuzzer::next()
{
    return generate();
}

// ──────────────────────────── CSV 导出 ────────────────────────────

/**
 * @brief 将结果列表导出为CSV格式
 *
 * 输出五列:
 *   Iteration  - 迭代序号
 *   Mutations  - 变异点数
 *   Offsets    - 变异偏移列表(分号分隔)
 *   HexData    - 变异后数据十六进制
 *   Description- 变异描述
 *
 * @param results 结果列表
 * @return CSV格式字符串
 */
QString SerialProtocolFuzzer::exportResults(const QList<FuzzResult> &results) const
{
    QString csv = QStringLiteral("Iteration,Mutations,Offsets,HexData,Description\n");

    for (const auto &r : results) {
        /* 变异偏移列表(分号分隔) */
        QStringList offsetStrs;
        for (int off : r.mutatedOffsets) {
            offsetStrs << QString::number(off);
        }

        /* 十六进制数据 */
        QStringList hexBytes;
        for (char b : r.fuzzedData) {
            hexBytes << QStringLiteral("%1")
                          .arg(static_cast<quint8>(b), 2, 16, QChar('0'))
                          .toUpper();
        }

        csv += QStringLiteral("%1,%2,\"%3\",\"%4\",%5\n")
                   .arg(r.iteration)
                   .arg(r.mutatedOffsets.size())
                   .arg(offsetStrs.join(QStringLiteral(";")))
                   .arg(hexBytes.join(QStringLiteral(" ")))
                   .arg(r.description);
    }

    return csv;
}

// ──────────────────────────── 统计 — 见 SerialProtocolFuzzerStats.cpp ────────────────────────────
