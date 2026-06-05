/**
 * @file ByteFrequencyAnalyzer.cpp
 * @brief 字节频率分析器核心实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 包含 256 级字节计数、香农熵计算、累积频率分布、
 * Top-N 频率查询、重复模式检测和 CSV 直方图导出。
 */

#include "utils/frequency2/ByteFrequencyAnalyzer.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

// ──────────────────────────── 构造 ────────────────────────────

/** @brief 构造字节频率分析器，初始化 256 级计数数组，设置 objectName 供 QSS 使用
 *  @param parent 父对象
 */
ByteFrequencyAnalyzer::ByteFrequencyAnalyzer(QObject *parent)
    : QObject(parent)
    , m_byteCounts(256, 0)
{
    setObjectName(QStringLiteral("ByteFrequencyAnalyzer"));
}

// ──────────────────────────── 核心分析 ────────────────────────────

/** @brief 对数据进行完整的字节频率分析
 *
 * 遍历数据中每个字节，统计 256 个值的出现次数，然后计算:
 *   - 各字节的频率 = count / totalBytes
 *   - 累积频率(按字节值 0→255 排列)
 *   - 香农信息熵 H = -sum(p_i * log2(p_i))，其中 p_i > 0
 *   - 归一化熵 = H / log2(uniqueBytes)
 *   - 最高/最低频字节值
 *
 * 分析完成后发射 analysisComplete 信号。
 *
 * @param data 待分析的数据(可为空，此时返回零值结果)
 * @return 分析结果快照
 */
ByteFrequencyAnalyzer::AnalysisResult ByteFrequencyAnalyzer::analyze(const QByteArray &data)
{
    AnalysisResult result;

    /* 空数据直接返回零值结果 */
    if (data.isEmpty()) {
        m_lastResult = result;
        emit analysisComplete(result);
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    /* ---- 步骤1: 统计各字节出现次数 ---- */
    std::fill(m_byteCounts.begin(), m_byteCounts.end(), quint64(0));

    const char *ptr = data.constData();
    const int len = data.size();
    for (int i = 0; i < len; ++i) {
        m_byteCounts[static_cast<quint8>(ptr[i])]++;
    }

    result.totalBytes = static_cast<quint64>(len);

    /* ---- 步骤2: 统计不同字节值数量 ---- */
    int uniqueCount = 0;
    for (int i = 0; i < 256; ++i) {
        if (m_byteCounts[i] > 0) {
            uniqueCount++;
        }
    }
    result.uniqueBytes = uniqueCount;

    /* ---- 步骤3: 查找最高/最低频字节 ---- */
    quint64 maxCount = 0;
    quint64 minCount = std::numeric_limits<quint64>::max();
    int maxByte = 0;
    int minByte = 0;

    for (int i = 0; i < 256; ++i) {
        if (m_byteCounts[i] > 0) {
            if (m_byteCounts[i] > maxCount) {
                maxCount = m_byteCounts[i];
                maxByte = i;
            }
            if (m_byteCounts[i] < minCount) {
                minCount = m_byteCounts[i];
                minByte = i;
            }
        }
    }
    result.mostFrequentByte  = static_cast<quint64>(maxByte);
    result.leastFrequentByte = static_cast<quint64>(minByte);

    /* ---- 步骤4: 计算香农熵 ---- */
    /* 计算实际出现的不同字节数，用于正确的归一化 */
    int uniqueBytes = 0;
    for (int i = 0; i < 256; ++i) {
        if (m_byteCounts[i] > 0) ++uniqueBytes;
    }
    result.maxEntropy        = (uniqueBytes > 1) ? std::log2(static_cast<double>(uniqueBytes)) : 0.0;
    result.shannonEntropy    = computeShannonEntropy();
    result.normalizedEntropy = (result.maxEntropy > 0.0)
        ? result.shannonEntropy / result.maxEntropy
        : 0.0;

    /* ---- 步骤5: 保存原始数据用于后续模式检测 ---- */
    m_lastData = data;

    /* ---- 步骤6: 更新累计统计 ---- */
    m_stats.totalAnalyses++;
    m_stats.totalBytesAnalyzed += static_cast<quint64>(len);

    /* 更新平均熵: 使用增量均值公式 avg = avg + (new - avg) / n */
    double delta = result.shannonEntropy - m_stats.avgEntropy;
    m_stats.avgEntropy += delta / static_cast<double>(m_stats.totalAnalyses);

    if (static_cast<quint64>(len) > m_stats.maxBytesInSingleAnalysis) {
        m_stats.maxBytesInSingleAnalysis = static_cast<quint64>(len);
    }

    /* 记录结果 */
    m_lastResult = result;
    Q_UNUSED(timer);

    emit analysisComplete(result);
    return result;
}

/** @brief 查询指定字节值的频率统计
 *
 * 根据 256 级计数数组计算该字节的频率和累积频率。
 * 累积频率为字节值 0 到 byteValue 的频率之和。
 *
 * @param byteValue 字节值(0~255)
 * @return 该字节的统计信息，未分析过或字节值越界时返回零值
 */
ByteFrequencyAnalyzer::ByteStats ByteFrequencyAnalyzer::byteFrequency(int byteValue) const
{
    ByteStats stats;

    if (byteValue < 0 || byteValue >= 256) {
        return stats;
    }

    /* 尚未进行过分析 */
    if (m_lastResult.totalBytes == 0) {
        return stats;
    }

    stats.count = m_byteCounts[byteValue];
    stats.frequency = static_cast<double>(stats.count)
                    / static_cast<double>(m_lastResult.totalBytes);

    /* 计算累积频率: 字节值 0 到 byteValue 的频率之和 */
    double cumulative = 0.0;
    for (int i = 0; i <= byteValue; ++i) {
        cumulative += static_cast<double>(m_byteCounts[i])
                    / static_cast<double>(m_lastResult.totalBytes);
    }
    stats.cumulativeFreq = cumulative;

    /* 计算该字节的局部信息量 -log2(p) */
    if (stats.frequency > 0.0) {
        stats.entropy = -std::log2(stats.frequency);
    }

    return stats;
}

/** @brief 获取出现频率最高的 N 个字节值(按计数降序)
 *
 * 将 256 个字节值按出现次数排序，返回前 count 个。
 * 每个元素为 (字节值, ByteStats) 对。
 *
 * @param count 返回数量，超过 256 时截断为 256
 * @return (字节值, ByteStats) 对的列表，按计数降序
 */
QList<QPair<int, ByteFrequencyAnalyzer::ByteStats>>
ByteFrequencyAnalyzer::topFrequencies(int count) const
{
    QList<QPair<int, ByteStats>> result;

    if (m_lastResult.totalBytes == 0) {
        return result;
    }

    /* 构建索引数组并按计数降序排序 */
    QList<int> indices;
    indices.reserve(256);
    for (int i = 0; i < 256; ++i) {
        indices.append(i);
    }

    std::sort(indices.begin(), indices.end(), [this](int a, int b) {
        return m_byteCounts[a] > m_byteCounts[b];
    });

    int n = qBound(0, count, 256);
    result.reserve(n);

    for (int i = 0; i < n; ++i) {
        int byteVal = indices[i];
        /* 跳过计数为 0 的字节值 */
        if (m_byteCounts[byteVal] == 0) {
            break;
        }
        result.append(qMakePair(byteVal, byteFrequency(byteVal)));
    }

    return result;
}

/** @brief 获取最近一次分析结果 @return AnalysisResult 快照 */
ByteFrequencyAnalyzer::AnalysisResult ByteFrequencyAnalyzer::analysisResult() const
{
    return m_lastResult;
}

// ──────────────────────────── 导出 ────────────────────────────

/** @brief 将字节频率直方图导出为 CSV 格式
 *
 * 输出 256 行数据，列:
 *   ByteValue(0~255), Count, Frequency, CumulativeFrequency
 * 第一行为表头，使用逗号分隔。
 *
 * @return CSV 格式字符串
 */
QString ByteFrequencyAnalyzer::exportHistogram() const
{
    QString csv = QStringLiteral("ByteValue,Count,Frequency,CumulativeFrequency\n");

    if (m_lastResult.totalBytes == 0) {
        /* 尚未分析过: 输出 256 行零值 */
        for (int i = 0; i < 256; ++i) {
            csv += QString::number(i) + QStringLiteral(",0,0.000000,0.000000\n");
        }
        return csv;
    }

    double cumulative = 0.0;
    double total = static_cast<double>(m_lastResult.totalBytes);

    for (int i = 0; i < 256; ++i) {
        double freq = static_cast<double>(m_byteCounts[i]) / total;
        cumulative += freq;

        csv += QString::number(i) + QStringLiteral(",")
             + QString::number(m_byteCounts[i]) + QStringLiteral(",")
             + QString::number(freq, 'f', 6) + QStringLiteral(",")
             + QString::number(cumulative, 'f', 6) + QStringLiteral("\n");
    }

    return csv;
}

// ──────────────────────────── 模式检测 ────────────────────────────

/** @brief 检测数据中重复出现的字节序列(主导模式)
 *
 * 使用自相关思想: 对每个候选长度 L (2 ~ min(dataSize/2, 64))，
 * 统计连续重复次数最多的长度为 L 的子序列。
 *
 * 时间复杂度: O(N * maxLen)，其中 maxLen = min(dataSize/2, 64)。
 *
 * @param minLength 最短模式长度(字节数)，小于 2 时强制为 2
 * @return 检测到的重复模式，未找到返回空 QByteArray
 */
QByteArray ByteFrequencyAnalyzer::detectPattern(int minLength) const
{
    QByteArray pattern = findDominantPattern(qMax(2, minLength));

    if (!pattern.isEmpty()) {
        /* 注意: 不在此处修改 m_stats，因为该方法为 const */
        /* 模式计数在 analyze 中通过间接调用更新 */
    }

    return pattern;
}

/** @brief 在最近分析的数据中查找重复子序列
 *
 * 对每个候选长度 L，遍历数据寻找连续重复:
 *   比较 data[i..i+L-1] 与 data[i+L..i+2L-1]，若相等则计数++。
 * 返回 (重复次数 * 长度) 最大的子序列。
 *
 * @param minLength 最短模式长度
 * @return 重复次数最多的子序列，未找到返回空
 */
QByteArray ByteFrequencyAnalyzer::findDominantPattern(int minLength) const
{
    const int dataSize = m_lastData.size();

    if (dataSize < minLength * 2) {
        return QByteArray();
    }

    /* 限制最大搜索长度，避免在大数据集上 O(N^2) 开销 */
    const int maxLen = qMin(dataSize / 2, 64);

    QByteArray bestPattern;
    int bestScore = 0; /* score = repeatCount * length */

    const char *data = m_lastData.constData();

    /* 遍历候选模式长度 */
    for (int L = minLength; L <= maxLen; ++L) {
        int limit = dataSize - L;

        for (int start = 0; start < limit; ++start) {
            /* 检查从 start 开始的长度为 L 的子序列的连续重复次数 */
            int repeats = 1;
            int pos = start + L;

            while (pos + L <= dataSize) {
                bool match = true;
                for (int j = 0; j < L; ++j) {
                    if (data[start + j] != data[pos + j]) {
                        match = false;
                        break;
                    }
                }
                if (!match) {
                    break;
                }
                repeats++;
                pos += L;
            }

            /* 至少重复 2 次才视为模式 */
            if (repeats >= 2) {
                int score = repeats * L;
                if (score > bestScore) {
                    bestScore = score;
                    bestPattern = m_lastData.mid(start, L);
                }
            }
        }
    }

    return bestPattern;
}

// ──────────────────────────── 内部方法 ────────────────────────────

/** @brief 计算当前字节计数的香农信息熵
 *
 * H = -sum(p_i * log2(p_i))，对所有 p_i > 0 的字节值求和。
 * 当所有 256 个值均匀出现时，H = 8.0 bit(最大熵)。
 * 当数据仅由单一字节值组成时，H = 0.0 bit。
 *
 * @return 香农熵(bit)，数据为空或 totalBytes 为 0 时返回 0
 */
double ByteFrequencyAnalyzer::computeShannonEntropy() const
{
    if (m_lastResult.totalBytes == 0) {
        return 0.0;
    }

    double entropy = 0.0;
    double total = static_cast<double>(m_lastResult.totalBytes);

    for (int i = 0; i < 256; ++i) {
        if (m_byteCounts[i] > 0) {
            double p = static_cast<double>(m_byteCounts[i]) / total;
            entropy -= p * std::log2(p);
        }
    }

    return entropy;
}
