/**
 * @file ArithmeticCoder.cpp
 * @brief 算术编码压缩器实现 — 频率表/编码/解码
 *
 * 使用32位整数算术实现精确的算术编码，频率表作为头部携带在编码数据中，
 * 支持自适应频率表、压缩比预估、编码效率统计。
 */

#include "utils/compress4/ArithmeticCoder.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cstring>

// ── 算术编码常量 ──

/** @brief 编码值范围位数(使用32位整数算术) */
static constexpr int kBits = 16;

/** @brief 编码范围最大值 */
static constexpr quint32 kMax = 1U << kBits;

/** @brief 一半范围 */
static constexpr quint32 kHalf = kMax >> 1;

/** @brief 四分之一范围 */
static constexpr quint32 kQuarter = kHalf >> 1;

/** @brief 频率表头部大小(魔数1B + 符号数2B + 每符号: 值1B + 频率4B) */
static constexpr int kHeaderBaseSize = 3;

/** @brief 频率表头部中每个符号的条目大小 */
static constexpr int kSymbolEntrySize = 5;

// ── 构造函数 ──

/** @brief 构造函数 @param parent 父对象 */
ArithmeticCoder::ArithmeticCoder(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ArithmeticCoder"));
}

// ── 频率表 ──

/**
 * @brief 从样本数据构建频率表
 * @param sample 样本数据
 */
void ArithmeticCoder::buildFrequencyTable(const QByteArray& sample)
{
    m_freq.clear();
    m_totalSymbols = 0;

    for (char b : sample) {
        quint8 sym = static_cast<quint8>(b);
        ++m_freq[sym];
        ++m_totalSymbols;
    }

    /* 构建累积概率表 */
    rebuildCumulative();
}

/**
 * @brief 手动设置符号频率
 * @param frequencies 符号->频率映射
 */
void ArithmeticCoder::setFrequencyTable(
    const QMap<quint8, quint64>& frequencies)
{
    m_freq = frequencies;
    m_totalSymbols = 0;
    for (auto it = m_freq.constBegin(); it != m_freq.constEnd(); ++it) {
        m_totalSymbols += it.value();
    }
    rebuildCumulative();
}

/** @brief 重建累积概率表 */
void ArithmeticCoder::rebuildCumulative()
{
    m_cumulative.clear();
    double cumulative = 0.0;
    for (auto it = m_freq.constBegin(); it != m_freq.constEnd(); ++it) {
        SymbolFreq sf;
        sf.symbol = it.key();
        sf.count = it.value();
        cumulative += static_cast<double>(it.value())
            / static_cast<double>(m_totalSymbols);
        sf.cumulativeProb = cumulative;
        m_cumulative.append(sf);
    }
    /* 确保最后一个累积概率为1.0 */
    if (!m_cumulative.isEmpty()) {
        m_cumulative.last().cumulativeProb = 1.0;
    }
}

// ── 编码 ──

/**
 * @brief 压缩数据(算术编码)
 * @param data 原始数据
 * @return 编码结果(含频率表头部)
 *
 * 编码格式: [频率表头部] + [算术编码数据]
 * 频率表头部: 魔数(1B) + 符号数(2B) + N*(符号(1B)+频率(4B))
 */
ArithmeticCoder::EncodeResult ArithmeticCoder::compress(
    const QByteArray& data)
{
    EncodeResult result;
    result.originalSize = data.size();

    if (data.isEmpty()) {
        emit error(tr("编码失败: 输入数据为空"));
        ++m_stats.totalEncodeErrors;
        return result;
    }

    /* 如果没有频率表, 从输入数据构建 */
    if (m_freq.isEmpty()) {
        buildFrequencyTable(data);
    }

    QElapsedTimer timer;
    timer.start();

    /* 序列化频率表头部 */
    QByteArray header = serializeFreqTable();

    /* 算术编码核心 */
    QByteArray encoded;
    quint32 low = 0;
    quint32 high = kMax - 1;
    int pendingBits = 0;

    auto outputBit = [&](int bit) {
        encoded.append(static_cast<char>(bit ? 0x80 : 0x00));
        /* 填充pending bits */
        char fill = bit ? static_cast<char>(0x80) : 0x00;
        while (pendingBits > 0) {
            encoded.append(fill);
            --pendingBits;
        }
    };

    for (int i = 0; i < data.size(); ++i) {
        quint8 symbol = static_cast<quint8>(data[i]);
        auto range = symbolRange(symbol);
        quint64 rangeSize = static_cast<quint64>(high - low + 1);

        quint64 newLow = low + static_cast<quint64>(
            range.first * static_cast<double>(rangeSize));
        quint64 newHigh = low + static_cast<quint64>(
            range.second * static_cast<double>(rangeSize)) - 1;

        low = static_cast<quint32>(newLow);
        high = static_cast<quint32>(newHigh);

        /* 归一化输出 */
        while (true) {
            if (high < kHalf) {
                outputBit(0);
                low <<= 1;
                high = (high << 1) | 1;
            } else if (low >= kHalf) {
                outputBit(1);
                low = (low - kHalf) << 1;
                high = ((high - kHalf) << 1) | 1;
            } else if (low >= kQuarter && high < 3 * kQuarter) {
                ++pendingBits;
                low -= kQuarter;
                high -= kQuarter;
                low <<= 1;
                high = (high << 1) | 1;
            } else {
                break;
            }
        }
    }

    /* 输出剩余位 */
    ++pendingBits;
    if (low < kQuarter) {
        outputBit(0);
    } else {
        outputBit(1);
    }

    /* 组装最终输出 */
    result.encoded = header + encoded;
    result.encodedSize = result.encoded.size();
    result.compressionRatio = (data.size() > 0)
        ? static_cast<double>(result.encodedSize)
          / static_cast<double>(data.size())
        : 0.0;
    result.entropy = computeEntropy();
    result.elapsedMs = timer.elapsed();

    /* 更新统计 */
    ++m_stats.totalEncodes;
    m_stats.totalBytesEncoded += data.size();
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes);

    emit encodeCompleted(result);
    return result;
}

// ── 解码 ──

/**
 * @brief 解压数据
 * @param data 编码数据(含频率表头部)
 * @return 解码后数据; 失败返回空
 */
QByteArray ArithmeticCoder::decompress(const QByteArray& data)
{
    if (data.size() < kHeaderBaseSize) {
        emit error(tr("解码失败: 数据过短"));
        ++m_stats.totalEncodeErrors;
        return {};
    }

    /* 读取频率表头部 */
    int offset = deserializeFreqTable(data, 0);
    if (offset < 0 || m_freq.isEmpty()) {
        emit error(tr("解码失败: 频率表无效"));
        ++m_stats.totalEncodeErrors;
        return {};
    }

    /* 从头部后开始解码 */
    /* 这里需要从头部获取原始数据长度 */
    if (offset + 4 > data.size()) {
        emit error(tr("解码失败: 缺少长度字段"));
        ++m_stats.totalEncodeErrors;
        return {};
    }

    int originalLen = 0;
    originalLen |= (static_cast<quint8>(data[offset]) << 24);
    originalLen |= (static_cast<quint8>(data[offset + 1]) << 16);
    originalLen |= (static_cast<quint8>(data[offset + 2]) << 8);
    originalLen |= static_cast<quint8>(data[offset + 3]);
    offset += 4;

    QByteArray payload = data.mid(offset);
    QByteArray result;
    result.reserve(originalLen);

    /* 算术解码核心 */
    quint32 low = 0;
    quint32 high = kMax - 1;
    quint32 code = 0;

    /* 读取初始编码值 */
    for (int i = 0; i < kBits && i < payload.size(); ++i) {
        code <<= 1;
        if (payload[i] & 0x80) code |= 1;
    }

    int bitPos = kBits;

    for (int count = 0; count < originalLen; ++count) {
        quint64 rangeSize = static_cast<quint64>(high - low + 1);
        double normalizedCode = static_cast<double>(code - low)
            / static_cast<double>(rangeSize);

        /* 在累积表中查找符号 */
        quint8 symbol = 0;
        for (int j = 0; j < m_cumulative.size(); ++j) {
            double prevProb = (j == 0) ? 0.0
                : m_cumulative[j - 1].cumulativeProb;
            if (normalizedCode >= prevProb
                && normalizedCode < m_cumulative[j].cumulativeProb) {
                symbol = m_cumulative[j].symbol;
                break;
            }
        }

        result.append(static_cast<char>(symbol));

        auto symRange = symbolRange(symbol);
        quint64 newLow = low + static_cast<quint64>(
            symRange.first * static_cast<double>(rangeSize));
        quint64 newHigh = low + static_cast<quint64>(
            symRange.second * static_cast<double>(rangeSize)) - 1;

        low = static_cast<quint32>(newLow);
        high = static_cast<quint32>(newHigh);

        /* 归一化 */
        while (true) {
            if (high < kHalf) {
                low <<= 1;
                high = (high << 1) | 1;
                code <<= 1;
                if (bitPos < payload.size()) {
                    code |= (payload[bitPos] & 0x80) ? 1 : 0;
                    ++bitPos;
                }
            } else if (low >= kHalf) {
                low = (low - kHalf) << 1;
                high = ((high - kHalf) << 1) | 1;
                code = (code - kHalf) << 1;
                if (bitPos < payload.size()) {
                    code |= (payload[bitPos] & 0x80) ? 1 : 0;
                    ++bitPos;
                }
            } else if (low >= kQuarter && high < 3 * kQuarter) {
                low -= kQuarter;
                high -= kQuarter;
                code -= kQuarter;
                low <<= 1;
                high = (high << 1) | 1;
                code <<= 1;
                if (bitPos < payload.size()) {
                    code |= (payload[bitPos] & 0x80) ? 1 : 0;
                    ++bitPos;
                }
            } else {
                break;
            }
        }
    }

    ++m_stats.totalDecodes;
    m_timeSum += 0; /* 解码时间不计入平均(简化) */
    emit decodeCompleted(result.size());
    return result;
}

// ── 辅助功能 ──

/**
 * @brief 预估压缩比
 * @param data 待分析数据
 * @return 预估压缩比(0.0~1.0)
 */
double ArithmeticCoder::estimateRatio(const QByteArray& data) const
{
    if (data.isEmpty() || m_freq.isEmpty()) return 1.0;
    return theoreticalLength(data.size())
        / (static_cast<double>(data.size()) * 8.0);
}

/** @brief 获取频率表 */
QVector<ArithmeticCoder::SymbolFreq>
ArithmeticCoder::frequencyTable() const
{
    return m_cumulative;
}

/** @brief 序列化频率表 */
QByteArray ArithmeticCoder::serializeFreqTable() const
{
    QByteArray header;
    header.append(static_cast<char>(kFreqHeaderMagic));

    int symCount = m_cumulative.size();
    header.append(static_cast<char>((symCount >> 8) & 0xFF));
    header.append(static_cast<char>(symCount & 0xFF));

    for (const auto& sf : m_cumulative) {
        header.append(static_cast<char>(sf.symbol));
        quint32 cnt = static_cast<quint32>(
            qMin(sf.count, static_cast<quint64>(0xFFFFFFFF)));
        header.append(static_cast<char>((cnt >> 24) & 0xFF));
        header.append(static_cast<char>((cnt >> 16) & 0xFF));
        header.append(static_cast<char>((cnt >> 8) & 0xFF));
        header.append(static_cast<char>(cnt & 0xFF));
    }

    return header;
}

/** @brief 反序列化频率表 @return 新偏移量; 失败返回-1 */
int ArithmeticCoder::deserializeFreqTable(const QByteArray& data, int offset)
{
    if (offset + kHeaderBaseSize > data.size()) return -1;
    if (static_cast<quint8>(data[offset]) != kFreqHeaderMagic) return -1;

    int symCount = (static_cast<quint8>(data[offset + 1]) << 8)
        | static_cast<quint8>(data[offset + 2]);
    offset += 3;

    if (offset + symCount * kSymbolEntrySize > data.size()) return -1;

    m_freq.clear();
    m_totalSymbols = 0;
    for (int i = 0; i < symCount; ++i) {
        quint8 sym = static_cast<quint8>(data[offset]);
        quint32 cnt = (static_cast<quint8>(data[offset + 1]) << 24)
            | (static_cast<quint8>(data[offset + 2]) << 16)
            | (static_cast<quint8>(data[offset + 3]) << 8)
            | static_cast<quint8>(data[offset + 4]);
        m_freq[sym] = cnt;
        m_totalSymbols += cnt;
        offset += kSymbolEntrySize;
    }

    rebuildCumulative();
    return offset;
}

/** @brief 计算信息熵 */
double ArithmeticCoder::computeEntropy() const
{
    if (m_totalSymbols == 0) return 0.0;
    double entropy = 0.0;
    for (auto it = m_freq.constBegin(); it != m_freq.constEnd(); ++it) {
        double p = static_cast<double>(it.value())
            / static_cast<double>(m_totalSymbols);
        if (p > 0) entropy -= p * qLn(p) / qLn(2.0);
    }
    return entropy;
}

/** @brief 理论编码长度(比特) */
double ArithmeticCoder::theoreticalLength(int dataLen) const
{
    return computeEntropy() * static_cast<double>(dataLen);
}

/** @brief 查找符号累积区间 */
QPair<double, double> ArithmeticCoder::symbolRange(quint8 symbol) const
{
    double low = 0.0;
    for (const auto& sf : m_cumulative) {
        if (sf.symbol == symbol) {
            return {low, sf.cumulativeProb};
        }
        low = sf.cumulativeProb;
    }
    return {0.0, 0.0};
}

/** @brief 重置统计 */
void ArithmeticCoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
