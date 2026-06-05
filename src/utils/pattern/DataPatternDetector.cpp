/**
 * @file DataPatternDetector.cpp
 * @brief 数据模式检测器实现 — 滑动窗口模式发现
 */

#include "utils/pattern/DataPatternDetector.h"

/** @brief 构造函数 @param parent 父对象 */
DataPatternDetector::DataPatternDetector(QObject* parent)
    : QObject(parent)
    , m_minPatternLen(2)
    , m_maxPatternLen(16)
    , m_minOccurrences(2)
{
}

/** @brief 设置最小模式长度 @param minLen 最小字节长度 */
void DataPatternDetector::setMinPatternLength(int minLen)
{
    m_minPatternLen = qMax(1, minLen);
}

/** @brief 设置最大模式长度 @param maxLen 最大字节长度 */
void DataPatternDetector::setMaxPatternLength(int maxLen)
{
    m_maxPatternLen = qMax(m_minPatternLen, maxLen);
}

/** @brief 设置最小出现次数过滤 @param minCount 最小次数 */
void DataPatternDetector::setMinOccurrences(int minCount)
{
    m_minOccurrences = qMax(2, minCount);
}

/** @brief 扫描数据中的重复模式 — 滑动窗口+哈希表统计 @param data 待扫描数据 */
void DataPatternDetector::scan(const QByteArray& data)
{
    if (data.size() < m_minPatternLen) return;

    ++m_stats.totalScans;
    m_stats.totalBytesAnalyzed += static_cast<quint64>(data.size());

    findPatterns(data);

    /* 更新统计极值 */
    int patternCount = m_patterns.size();
    if (patternCount > m_stats.peakPatterns) {
        m_stats.peakPatterns = patternCount;
    }

    emit scanCompleted(patternCount);
}

/** @brief 使用滑动窗口发现重复模式 @param data 数据 */
void DataPatternDetector::findPatterns(const QByteArray& data)
{
    int dataSize = data.size();

    /* 对每种模式长度进行滑动窗口扫描 */
    for (int patLen = m_minPatternLen; patLen <= qMin(m_maxPatternLen, dataSize); ++patLen) {
        QMap<QByteArray, QList<int>> candidates;

        /* 提取所有长度为patLen的子串及其位置 */
        for (int offset = 0; offset <= dataSize - patLen; ++offset) {
            QByteArray sub = data.mid(offset, patLen);
            candidates[sub].append(offset);
        }

        /* 筛选出现次数>=阈值的模式 */
        for (auto it = candidates.constBegin(); it != candidates.constEnd(); ++it) {
            const QByteArray& pattern = it.key();
            const QList<int>& positions = it.value();

            if (positions.size() < m_minOccurrences) continue;

            /* 检查是否已有此模式(可能被更短长度的扫描发现) */
            if (m_patterns.contains(pattern)) {
                /* 更新已有模式的次数 */
                auto& existing = m_patterns[pattern];
                int newOccs = positions.size();
                if (newOccs > existing.occurrences) {
                    existing.occurrences = newOccs;
                    existing.lastOffset = positions.last();
                }
                continue;
            }

            /* 新模式 */
            DetectedPattern dp;
            dp.pattern = pattern;
            dp.occurrences = positions.size();
            dp.firstOffset = positions.first();
            dp.lastOffset = positions.last();

            /* 计算频率: 出现次数/数据大小(KB) */
            double kb = qMax(1.0, data.size() / 1024.0);
            dp.frequency = dp.occurrences / kb;

            m_patterns.insert(pattern, dp);

            ++m_stats.totalPatternsFound;
            ++m_stats.totalDetections;
            if (pattern.size() > m_stats.longestPattern) {
                m_stats.longestPattern = pattern.size();
            }

            emit patternDetected(pattern, dp.occurrences);
        }
    }
}

/** @brief 获取检测到的模式(按频率降序) @return 模式列表 */
QList<DataPatternDetector::DetectedPattern> DataPatternDetector::detectedPatterns() const
{
    QList<DetectedPattern> result = m_patterns.values();

    /* 按出现次数降序排序 */
    std::sort(result.begin(), result.end(),
        [](const DetectedPattern& a, const DetectedPattern& b) {
            if (a.occurrences != b.occurrences)
                return a.occurrences > b.occurrences;
            return a.pattern.size() > b.pattern.size();
        });

    return result;
}

/** @brief 清除所有检测数据 */
void DataPatternDetector::clear()
{
    m_patterns.clear();
}

/** @brief 重置所有统计计数器 */
void DataPatternDetector::resetStatistics()
{
    m_stats = Stats{};
}
