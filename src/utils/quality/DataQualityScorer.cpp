/**
 * @file DataQualityScorer.cpp
 * @brief 数据质量评分引擎实现 — 5维度评估+等级划分
 */

#include "utils/quality/DataQualityScorer.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
DataQualityScorer::DataQualityScorer(QObject* parent)
    : QObject(parent)
    , m_lastScore(100.0)
    , m_scoreSum(0.0)
{
    /* 默认权重: 完整性30% / 一致性25% / 时效性20% / 准确性15% / 有效性10% */
    m_weights[Dimension::Completeness] = 0.30;
    m_weights[Dimension::Consistency]  = 0.25;
    m_weights[Dimension::Timeliness]   = 0.20;
    m_weights[Dimension::Accuracy]     = 0.15;
    m_weights[Dimension::Validity]     = 0.10;
}

/** @brief 设置维度权重 @param dimension 维度 @param weight 权重 */
void DataQualityScorer::setWeight(Dimension dimension, double weight)
{
    m_weights[dimension] = qBound(0.0, weight, 1.0);
}

/** @brief 评估数据质量 @param data 数据 @param expectedCount 预期点数 @return 综合分 */
double DataQualityScorer::evaluate(
    const QVector<double>& data, int expectedCount)
{
    if (data.isEmpty()) {
        return 0.0;
    }

    m_lastScores.clear();

    double totalScore = 0.0;
    double totalWeight = 0.0;

    struct DimEval { Dimension dim; double score; };
    QVector<DimEval> evals = {
        {Dimension::Completeness, scoreCompleteness(data, expectedCount)},
        {Dimension::Consistency,  scoreConsistency(data)},
        {Dimension::Timeliness,   scoreTimeliness(data)},
        {Dimension::Accuracy,     scoreAccuracy(data)},
        {Dimension::Validity,     scoreValidity(data)}
    };

    for (const auto& e : evals) {
        double w = m_weights.value(e.dim, 0.0);
        totalScore += e.score * w;
        totalWeight += w;

        DimensionScore ds;
        ds.dimension = e.dim;
        ds.score = e.score;
        ds.weight = w;
        m_lastScores.append(ds);
    }

    double finalScore = (totalWeight > 0) ? totalScore / totalWeight : 0.0;
    finalScore = qBound(0.0, finalScore, 100.0);

    /* 更新统计 */
    ++m_stats.totalEvaluations;
    m_scoreSum += finalScore;
    m_stats.averageScore = m_scoreSum
        / static_cast<double>(m_stats.totalEvaluations);
    if (finalScore > m_stats.peakScore) {
        m_stats.peakScore = finalScore;
    }
    if (finalScore < m_stats.lowestScore) {
        m_stats.lowestScore = finalScore;
    }

    Grade grade = toGrade(finalScore);
    int gradeKey = static_cast<int>(grade);
    ++m_stats.evaluationsByGrade[gradeKey];

    if (!qFuzzyCompare(m_lastScore, finalScore)) {
        emit qualityChanged(m_lastScore, finalScore);
    }
    m_lastScore = finalScore;

    return finalScore;
}

/** @brief 获取维度分数 @return 分数列表 */
QVector<DataQualityScorer::DimensionScore> DataQualityScorer::dimensionScores() const
{
    return m_lastScores;
}

/** @brief 分数转等级 @param score 分数 @return 等级 */
DataQualityScorer::Grade DataQualityScorer::toGrade(double score)
{
    if (score >= 90.0) return Grade::A;
    if (score >= 80.0) return Grade::B;
    if (score >= 60.0) return Grade::C;
    if (score >= 40.0) return Grade::D;
    return Grade::F;
}

/** @brief 重置统计 */
void DataQualityScorer::resetStatistics()
{
    m_stats = Stats{};
    m_scoreSum = 0.0;
    m_lastScore = 100.0;
}

/** @brief 完整性评分 @param data 数据 @param expected 预期点数 @return 0-100 */
double DataQualityScorer::scoreCompleteness(
    const QVector<double>& data, int expected)
{
    if (expected <= 0) {
        return 100.0;
    }
    double ratio = static_cast<double>(data.size())
        / static_cast<double>(expected);
    return qBound(0.0, ratio * 100.0, 100.0);
}

/** @brief 一致性评分 @param data 数据 @return 0-100 */
double DataQualityScorer::scoreConsistency(const QVector<double>& data)
{
    if (data.size() < 2) {
        return 100.0;
    }

    double sum = 0.0;
    for (double v : data) {
        sum += v;
    }
    double mean = sum / data.size();

    double sqSum = 0.0;
    for (double v : data) {
        double d = v - mean;
        sqSum += d * d;
    }
    double stddev = qSqrt(sqSum / data.size());

    /* 变异系数越低，一致性越高 */
    if (qAbs(mean) < 1e-10) {
        return (stddev < 1e-10) ? 100.0 : 0.0;
    }
    double cv = stddev / qAbs(mean);
    /* CV=0 → 100分, CV=1 → 0分 */
    return qBound(0.0, (1.0 - qMin(cv, 1.0)) * 100.0, 100.0);
}

/** @brief 时效性评分 @param data 数据 @return 0-100 */
double DataQualityScorer::scoreTimeliness(const QVector<double>& data)
{
    if (data.size() < 3) {
        return 100.0;
    }

    /* 检测间隔均匀性(用差分的标准差衡量) */
    QVector<double> diffs;
    diffs.reserve(data.size() - 1);
    for (int i = 1; i < data.size(); ++i) {
        diffs.append(data[i] - data[i - 1]);
    }

    double sum = 0.0;
    for (double d : diffs) {
        sum += d;
    }
    double meanDiff = sum / diffs.size();

    double sqSum = 0.0;
    for (double d : diffs) {
        double dev = d - meanDiff;
        sqSum += dev * dev;
    }
    double diffStddev = qSqrt(sqSum / diffs.size());

    if (qAbs(meanDiff) < 1e-10) {
        return 100.0;
    }
    double cv = diffStddev / qAbs(meanDiff);
    return qBound(0.0, (1.0 - qMin(cv, 1.0)) * 100.0, 100.0);
}

/** @brief 准确性评分 @param data 数据 @return 0-100 */
double DataQualityScorer::scoreAccuracy(const QVector<double>& data)
{
    if (data.size() < 2) {
        return 100.0;
    }

    /* 用中位数偏差比评估: 偏差大=准确性低 */
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    int n = sorted.size();
    double median = (n % 2 == 0)
        ? (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0
        : sorted[n / 2];

    double sum = 0.0;
    for (double v : data) {
        sum += v;
    }
    double mean = sum / data.size();

    double meanDev = qAbs(mean - median);
    double range = sorted.last() - sorted.first();
    if (range < 1e-10) {
        return 100.0;
    }

    double ratio = meanDev / range;
    return qBound(0.0, (1.0 - qMin(ratio * 10.0, 1.0)) * 100.0, 100.0);
}

/** @brief 有效性评分 @param data 数据 @return 0-100 */
double DataQualityScorer::scoreValidity(const QVector<double>& data)
{
    if (data.isEmpty()) {
        return 0.0;
    }

    /* 检查NaN/Inf/重复值比例 */
    int invalid = 0;
    int duplicates = 0;
    double prev = data[0];

    for (int i = 0; i < data.size(); ++i) {
        if (qIsNaN(data[i]) || qIsInf(data[i])) {
            ++invalid;
        }
        if (i > 0 && qFuzzyCompare(data[i], prev)) {
            ++duplicates;
        }
        if (i > 0) {
            prev = data[i];
        }
    }

    double invalidRate = static_cast<double>(invalid) / data.size();
    double dupRate = static_cast<double>(duplicates) / data.size();

    /* 无效值直接扣分，重复值轻微扣分 */
    double score = 100.0 - invalidRate * 100.0 - dupRate * 20.0;
    return qBound(0.0, score, 100.0);
}
