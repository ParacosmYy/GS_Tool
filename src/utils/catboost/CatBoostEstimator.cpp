/**
 * @file CatBoostEstimator.cpp
 * @brief 梯度提升决策树回归估计器实现 — CatBoost风格Stub
 */

#include "CatBoostEstimator.h"

#include <QElapsedTimer>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

CatBoostEstimator::CatBoostEstimator(QObject* parent)
    : QObject(parent)
{
}

CatBoostEstimator::~CatBoostEstimator() = default;

// ═══════════════════════════════════════════════════════════
// 模型操作
// ═══════════════════════════════════════════════════════════

bool CatBoostEstimator::train(const QVector<QVector<double>>& X,
                              const QVector<double>& y)
{
    QElapsedTimer timer;
    timer.start();

    // 参数校验
    if (X.isEmpty() || y.isEmpty()) {
        m_stats.totalTrains += 1;
        emit trainingCompleted(false);
        return false;
    }

    const int nSamples = qMin(X.size(), y.size());
    if (nSamples == 0) {
        m_stats.totalTrains += 1;
        emit trainingCompleted(false);
        return false;
    }

    const int nFeatures = X[0].size();

    // Stub实现: 计算每列特征均值
    m_featureMeans.resize(nFeatures);
    for (int j = 0; j < nFeatures; ++j) {
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < nSamples; ++i) {
            if (j < X[i].size()) {
                sum += X[i][j];
                ++count;
            }
        }
        m_featureMeans[j] = (count > 0) ? (sum / static_cast<double>(count)) : 0.0;
    }

    // 计算目标均值
    double targetSum = 0.0;
    for (int i = 0; i < nSamples; ++i) {
        targetSum += y[i];
    }
    m_targetMean = targetSum / static_cast<double>(nSamples);

    // 计算简单线性权重(Stub)
    m_trained = true;

    const double elapsedMs = static_cast<double>(timer.elapsed());
    m_stats.totalTrains += 1;
    updateAvgTime(elapsedMs);

    emit trainingCompleted(true);
    return true;
}

double CatBoostEstimator::predict(const QVector<double>& features)
{
    QElapsedTimer timer;
    timer.start();

    double result = m_targetMean;

    if (m_trained && !features.isEmpty() && !m_featureMeans.isEmpty()) {
        // Stub预测: 基于目标均值和特征偏移的简单估计
        const int n = qMin(features.size(), m_featureMeans.size());
        double deviation = 0.0;
        for (int i = 0; i < n; ++i) {
            deviation += (features[i] - m_featureMeans[i]);
        }
        // 简单线性响应: 目标均值 + 平均偏差的衰减因子
        if (n > 0) {
            result += deviation / static_cast<double>(n) * 0.1;
        }
    }

    const double elapsedMs = static_cast<double>(timer.elapsed());
    m_stats.totalPredictions += 1;
    updateAvgTime(elapsedMs);

    emit predictionReady(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

bool CatBoostEstimator::isTrained() const
{
    return m_trained;
}

int CatBoostEstimator::featureCount() const
{
    return m_featureMeans.size();
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

CatBoostEstimator::Stats CatBoostEstimator::stats() const
{
    return m_stats;
}

void CatBoostEstimator::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void CatBoostEstimator::updateAvgTime(double elapsedMs) const
{
    const auto total = m_stats.totalTrains + m_stats.totalPredictions;
    if (total <= 1) {
        m_stats.avgProcessingTimeMs = elapsedMs;
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs *
                static_cast<double>(total - 1) / static_cast<double>(total) +
            elapsedMs / static_cast<double>(total);
    }
}
