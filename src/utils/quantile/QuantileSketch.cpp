/**
 * @file QuantileSketch.cpp
 * @brief 流式近似分位数草图实现 — 基于GK算法
 */

#include "QuantileSketch.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

QuantileSketch::QuantileSketch(QObject* parent)
    : QObject(parent)
{
}

QuantileSketch::~QuantileSketch() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void QuantileSketch::setEpsilon(double epsilon)
{
    m_epsilon = qBound(0.001, epsilon, 0.5);
}

double QuantileSketch::epsilon() const
{
    return m_epsilon;
}

// ═══════════════════════════════════════════════════════════
// 操作
// ═══════════════════════════════════════════════════════════

void QuantileSketch::add(double value)
{
    QElapsedTimer timer;
    timer.start();

    // 查找插入位置(保持有序)
    int pos = 0;
    while (pos < m_summary.size() && m_summary[pos].value < value) {
        ++pos;
    }

    // 计算新元组的g和delta
    const int twoEpsN = static_cast<int>(2.0 * m_epsilon *
                            static_cast<double>(m_count + 1));

    Tuple t;
    t.value = value;

    if (pos == 0 || pos == m_summary.size()) {
        t.g = 1;
        t.delta = 0;
    } else {
        t.g = 1;
        t.delta = twoEpsN - 1;
    }

    m_summary.insert(pos, t);
    m_count += 1;

    // 定期压缩
    const int threshold = static_cast<int>(
        1.0 / (2.0 * m_epsilon));
    if (static_cast<int>(m_summary.size()) > threshold) {
        compress(twoEpsN);
    }

    m_stats.totalAdds += 1;
    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);

    emit valueAdded(value);
}

double QuantileSketch::query(double percentile)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalQueries += 1;

    if (m_summary.isEmpty()) {
        const double elapsedMs = static_cast<double>(timer.elapsed());
        updateAvgTime(elapsedMs);
        emit queryCompleted(percentile, 0.0);
        return 0.0;
    }

    percentile = qBound(0.0, percentile, 1.0);
    const double rank = percentile * static_cast<double>(m_count);
    const double eps = static_cast<double>(m_count) * m_epsilon;

    int cumG = 0;
    double result = m_summary[0].value;

    for (int i = 0; i < m_summary.size(); ++i) {
        cumG += m_summary[i].g;
        const double target = rank - eps;
        if (static_cast<double>(cumG) + static_cast<double>(m_summary[i].delta) / 2.0 > target) {
            break;
        }
        result = m_summary[i].value;
    }

    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);
    emit queryCompleted(percentile, result);
    return result;
}

void QuantileSketch::merge(const QuantileSketch& other)
{
    QElapsedTimer timer;
    timer.start();

    if (other.m_summary.isEmpty()) {
        return;
    }

    // 合并两个有序摘要
    QVector<Tuple> merged;
    merged.reserve(m_summary.size() + other.m_summary.size());

    int i = 0, j = 0;
    while (i < m_summary.size() && j < other.m_summary.size()) {
        if (m_summary[i].value <= other.m_summary[j].value) {
            merged.append(m_summary[i]);
            ++i;
        } else {
            merged.append(other.m_summary[j]);
            ++j;
        }
    }
    while (i < m_summary.size()) {
        merged.append(m_summary[i++]);
    }
    while (j < other.m_summary.size()) {
        merged.append(other.m_summary[j++]);
    }

    m_summary = std::move(merged);
    m_count += other.m_count;

    // 合并后压缩
    const int twoEpsN = static_cast<int>(2.0 * m_epsilon *
                            static_cast<double>(m_count));
    compress(twoEpsN);

    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);
    emit mergeCompleted(m_count);
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

quint64 QuantileSketch::count() const
{
    return m_count;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

QuantileSketch::Stats QuantileSketch::stats() const
{
    return m_stats;
}

void QuantileSketch::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void QuantileSketch::compress(int band)
{
    if (m_summary.size() <= 2) {
        return;
    }

    QVector<Tuple> compressed;
    compressed.reserve(m_summary.size());

    int i = 0;
    while (i < m_summary.size()) {
        if (i == 0 || i == m_summary.size() - 1) {
            // 保留首尾元组
            compressed.append(m_summary[i]);
            ++i;
        } else {
            // 合并相邻且band相同的元组
            Tuple merged = m_summary[i];
            int j = i + 1;
            while (j < m_summary.size() - 1) {
                if (this->band(merged.delta) == this->band(m_summary[j].delta)) {
                    merged.g += m_summary[j].g;
                    ++j;
                } else {
                    break;
                }
            }
            compressed.append(merged);
            i = j;
        }
    }

    m_summary = std::move(compressed);
    Q_UNUSED(band)
}

int QuantileSketch::band(int delta) const
{
    if (delta == 0) return 0;
    int b = 0;
    int power = 1;
    while (power <= delta) {
        power *= 2;
        ++b;
    }
    return b;
}

void QuantileSketch::updateAvgTime(double elapsedMs) const
{
    const auto total = m_stats.totalAdds + m_stats.totalQueries;
    if (total <= 1) {
        m_stats.avgProcessingTimeMs = elapsedMs;
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs *
                static_cast<double>(total - 1) / static_cast<double>(total) +
            elapsedMs / static_cast<double>(total);
    }
}
