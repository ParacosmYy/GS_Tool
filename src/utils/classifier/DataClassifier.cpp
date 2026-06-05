/**
 * @file DataClassifier.cpp
 * @brief 数据分类引擎实现 — 阈值/KNN/均值漂移/规则分类
 */

#include "utils/classifier/DataClassifier.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DataClassifier::DataClassifier(QObject* parent)
    : QObject(parent)
    , m_method(ClassifyMethod::Threshold)
    , m_bandwidth(1.0)
{
}

void DataClassifier::setMethod(ClassifyMethod method) { m_method = method; }

/** @brief 添加分类规则 @param rule 规则 */
void DataClassifier::addRule(const ClassRule& rule)
{
    m_rules.append(rule);
}

/** @brief 清除所有规则 */
void DataClassifier::clearRules()
{
    m_rules.clear();
}

/** @brief 设置训练数据 @param data 训练数据(值,类别) */
void DataClassifier::setTrainingData(const QVector<QPair<double, int>>& data)
{
    m_trainingData = data;
}

/** @brief 分类单个值 @param value 输入值 @return 分类结果 */
DataClassifier::ClassResult DataClassifier::classify(double value)
{
    ClassResult result;
    switch (m_method) {
    case ClassifyMethod::Threshold:
        result = classifyThreshold(value);
        break;
    case ClassifyMethod::KNearest:
        result = classifyKNearest(value);
        break;
    case ClassifyMethod::MeanShift:
        result = classifyMeanShift(value);
        break;
    case ClassifyMethod::RuleBased:
        result = classifyRuleBased(value);
        break;
    }

    ++m_stats.totalClassifications;
    ++m_stats.classCounts[result.classId];
    if (result.confidence > m_stats.peakConfidence) {
        m_stats.peakConfidence = result.confidence;
    }

    emit classified(result.classId, result.confidence);
    return result;
}

/** @brief 批量分类 @param data 数据 @return 结果列表 */
QList<DataClassifier::ClassResult> DataClassifier::classifyBatch(
    const QVector<double>& data)
{
    QList<ClassResult> results;
    for (double v : data) {
        results.append(classify(v));
    }
    return results;
}

void DataClassifier::resetStatistics() { m_stats = Stats{}; }

/** @brief 阈值分类 @param value 值 @return 结果 */
DataClassifier::ClassResult DataClassifier::classifyThreshold(double value)
{
    ClassResult result;
    for (const auto& rule : m_rules) {
        if (value >= rule.minValue && value <= rule.maxValue) {
            result.classId = rule.classId;
            result.label = rule.label;
            result.distance = 0.0;
            double range = rule.maxValue - rule.minValue;
            if (range > 0) {
                double mid = (rule.minValue + rule.maxValue) / 2.0;
                result.confidence = 1.0 - qAbs(value - mid) / (range / 2.0);
            } else {
                result.confidence = 1.0;
            }
            return result;
        }
    }
    result.classId = -1;
    result.confidence = 0.0;
    return result;
}

/** @brief K近邻分类 @param value 值 @return 结果 */
DataClassifier::ClassResult DataClassifier::classifyKNearest(double value)
{
    ClassResult result;
    if (m_trainingData.isEmpty()) {
        result.classId = -1;
        return result;
    }

    /* 计算所有距离 */
    QVector<QPair<double, int>> distances;
    distances.reserve(m_trainingData.size());
    for (const auto& td : m_trainingData) {
        double d = qAbs(value - td.first);
        distances.append({d, td.second});
    }

    /* 按距离排序取前K个 */
    const int K = qMin(5, distances.size());
    std::partial_sort(distances.begin(), distances.begin() + K,
                      distances.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });

    /* 多数投票 */
    QMap<int, int> votes;
    for (int i = 0; i < K; ++i) {
        ++votes[distances[i].second];
    }

    int bestClass = -1, bestVotes = 0;
    for (auto it = votes.constBegin(); it != votes.constEnd(); ++it) {
        if (it.value() > bestVotes) {
            bestVotes = it.value();
            bestClass = it.key();
        }
    }

    result.classId = bestClass;
    result.confidence = static_cast<double>(bestVotes) / K;
    result.distance = distances[0].first;
    return result;
}

/** @brief 均值漂移分类 @param value 值 @return 结果 */
DataClassifier::ClassResult DataClassifier::classifyMeanShift(double value)
{
    ClassResult result;
    if (m_trainingData.isEmpty()) {
        result.classId = -1;
        return result;
    }

    /* 简单均值漂移: 迭代收敛到局部密度最大 */
    double current = value;
    for (int iter = 0; iter < 20; ++iter) {
        double weightedSum = 0.0;
        double weightSum = 0.0;
        for (const auto& td : m_trainingData) {
            double dist = qAbs(current - td.first);
            if (dist < m_bandwidth) {
                double w = 1.0 - dist / m_bandwidth;
                weightedSum += td.first * w;
                weightSum += w;
            }
        }
        if (weightSum < 1e-10) break;
        double next = weightedSum / weightSum;
        if (qAbs(next - current) < 1e-6) break;
        current = next;
    }

    /* 找最近的训练点确定类别 */
    double minDist = 1e300;
    int bestClass = -1;
    for (const auto& td : m_trainingData) {
        double d = qAbs(current - td.first);
        if (d < minDist) {
            minDist = d;
            bestClass = td.second;
        }
    }

    result.classId = bestClass;
    result.distance = minDist;
    result.confidence = qMax(0.0, 1.0 - minDist / m_bandwidth);
    return result;
}

/** @brief 规则分类 @param value 值 @return 结果 */
DataClassifier::ClassResult DataClassifier::classifyRuleBased(double value)
{
    ClassResult result;
    for (const auto& rule : m_rules) {
        if (value >= rule.minValue && value <= rule.maxValue) {
            result.classId = rule.classId;
            result.label = rule.label;
            double range = rule.maxValue - rule.minValue;
            if (range > 0) {
                double distToEdge = qMin(value - rule.minValue, rule.maxValue - value);
                result.distance = range / 2.0 - distToEdge;
                result.confidence = distToEdge / (range / 2.0);
            } else {
                result.confidence = 1.0;
            }
            return result;
        }
    }
    result.classId = -1;
    result.confidence = 0.0;
    return result;
}
