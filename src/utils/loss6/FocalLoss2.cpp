/**
 * @file FocalLoss2.cpp
 * @brief Focal Loss损失函数实现
 */

#include "utils/loss6/FocalLoss2.h"

#include <QtMath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
FocalLoss2::FocalLoss2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算二分类Focal Loss
 * FL(p_t) = -alpha_t * (1-p_t)^gamma * log(p_t)
 */
FocalLoss2::LossResult FocalLoss2::binaryLoss(
    const QVector<double>& predictions,
    const QVector<int>& targets,
    double gamma,
    double alpha)
{
    m_timer.start();
    LossResult result;
    int n = qMin(predictions.size(), targets.size());
    if (n == 0) return result;

    double totalLoss = 0.0;
    double totalGradSq = 0.0;
    int posCount = 0, negCount = 0;
    int easyCount = 0, hardCount = 0;

    for (int i = 0; i < n; ++i) {
        double p = qBound(m_epsilon, predictions[i], 1.0 - m_epsilon);
        int y = targets[i];

        /* p_t = p if y=1, else (1-p) */
        double pt = (y == 1) ? p : (1.0 - p);
        double alphaT = (y == 1) ? alpha : (1.0 - alpha);

        /* Focal loss: -alpha_t * (1-p_t)^gamma * log(p_t) */
        double modulator = focalModulator(pt, gamma);
        double lossVal = -alphaT * modulator * safeLog(pt);

        totalLoss += lossVal;

        /* 梯度 */
        double grad = gradient(p, y, gamma, alpha);
        totalGradSq += grad * grad;

        /* 统计样本类型 */
        if (y == 1) ++posCount; else ++negCount;
        if (pt > 0.9) ++easyCount;
        if (pt < 0.3) ++hardCount;
    }

    result.loss = totalLoss;
    result.perSampleLoss = totalLoss / static_cast<double>(n);
    result.gradNorm = std::sqrt(totalGradSq / static_cast<double>(n));
    result.posRatio = static_cast<double>(posCount) / static_cast<double>(n);
    result.negRatio = static_cast<double>(negCount) / static_cast<double>(n);
    result.easyCount = easyCount;
    result.hardCount = hardCount;

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalComputations;
    m_stats.totalSamples += n;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(result.loss, result.gradNorm);
    return result;
}

/**
 * @brief 计算多分类Focal Loss
 * FL(p_t) = -alpha_t * (1-p_t)^gamma * log(p_t)
 */
FocalLoss2::LossResult FocalLoss2::multiClassLoss(
    const QVector<QVector<double>>& predictions,
    const QVector<int>& targets,
    double gamma,
    const QVector<double>& alpha)
{
    m_timer.start();
    LossResult result;
    int n = predictions.size();
    if (n == 0) return result;

    int K = predictions[0].size();
    /* 默认alpha: 均匀分布 */
    QVector<double> effectiveAlpha = alpha.isEmpty()
        ? QVector<double>(K, 1.0 / K) : alpha;

    double totalLoss = 0.0;
    double totalGradSq = 0.0;
    int easyCount = 0, hardCount = 0;

    for (int i = 0; i < n; ++i) {
        int y = targets[i];
        if (y < 0 || y >= K || y >= predictions[i].size()) continue;

        /* 对预测概率做softmax归一化保护 */
        double pt = qBound(m_epsilon, predictions[i][y], 1.0 - m_epsilon);
        double aT = (y < effectiveAlpha.size()) ? effectiveAlpha[y] : 1.0 / K;

        double modulator = focalModulator(pt, gamma);
        double lossVal = -aT * modulator * safeLog(pt);
        totalLoss += lossVal;

        /* 梯度近似 */
        double grad = -aT * (modulator * (1.0 / qMax(m_epsilon, pt))
                     - gamma * std::pow(1.0 - pt, gamma - 1.0) * safeLog(pt));
        totalGradSq += grad * grad;

        if (pt > 0.9) ++easyCount;
        if (pt < 0.3) ++hardCount;
    }

    result.loss = totalLoss;
    result.perSampleLoss = totalLoss / static_cast<double>(n);
    result.gradNorm = std::sqrt(totalGradSq / static_cast<double>(qMax(1, n)));
    result.easyCount = easyCount;
    result.hardCount = hardCount;

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalComputations;
    m_stats.totalSamples += n;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(result.loss, result.gradNorm);
    return result;
}

/**
 * @brief 计算二分类梯度
 * d(FL)/d(p) 的解析形式
 */
double FocalLoss2::gradient(double pred, int target, double gamma,
                             double alpha) const
{
    double p = qBound(m_epsilon, pred, 1.0 - m_epsilon);
    double pt = (target == 1) ? p : (1.0 - p);
    double alphaT = (target == 1) ? alpha : (1.0 - alpha);
    double sign = (target == 1) ? 1.0 : -1.0;

    /* 导数: alpha_t * [(1-p_t)^(gamma-1) * (-gamma*log(p_t) + (1-p_t))/p_t] * sign */
    double oneMinusPt = 1.0 - pt;
    double modulatorPart = std::pow(qMax(m_epsilon, oneMinusPt), gamma - 1.0);
    double logPart = safeLog(pt);

    double grad = alphaT * modulatorPart
        * (oneMinusPt - gamma * logPart * pt)
        / qMax(m_epsilon, pt);

    return sign * grad;
}

/**
 * @brief 推荐最优阈值
 * 通过网格搜索在[0.1, 0.9]上搜索最大化F1的阈值
 */
QPair<double, double> FocalLoss2::recommendThreshold(
    const QVector<double>& predictions,
    const QVector<int>& targets,
    double gamma)
{
    m_timer.start();

    double bestThresh = 0.5;
    double bestF1 = 0.0;

    /* 网格搜索: 81个候选阈值 */
    for (int step = 1; step <= 80; ++step) {
        double thresh = step / 100.0;
        double f1 = computeF1(predictions, targets, thresh);
        if (f1 > bestF1) {
            bestF1 = f1;
            bestThresh = thresh;
        }
    }

    /* 精细搜索: 在最优阈值附近细分 */
    double lo = qMax(0.01, bestThresh - 0.02);
    double hi = qMin(0.99, bestThresh + 0.02);
    for (int step = 0; step <= 40; ++step) {
        double thresh = lo + (hi - lo) * step / 40.0;
        double f1 = computeF1(predictions, targets, thresh);
        if (f1 > bestF1) {
            bestF1 = f1;
            bestThresh = thresh;
        }
    }

    m_timeSum += m_timer.elapsed();
    ++m_stats.totalComputations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return {bestThresh, bestF1};
}

/** @brief 重置统计 */
void FocalLoss2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 安全log计算
 * 裁剪到[m_epsilon, 1]范围避免数值错误
 */
double FocalLoss2::safeLog(double x) const
{
    return std::log(qMax(m_epsilon, x));
}

/**
 * @brief 计算Focal调制因子 (1-p_t)^gamma
 */
double FocalLoss2::focalModulator(double pt, double gamma) const
{
    return std::pow(qMax(m_epsilon, 1.0 - pt), gamma);
}

/**
 * @brief 计算F1分数
 */
double FocalLoss2::computeF1(const QVector<double>& preds,
                              const QVector<int>& targets,
                              double threshold) const
{
    int tp = 0, fp = 0, fn = 0;
    int n = qMin(preds.size(), targets.size());

    for (int i = 0; i < n; ++i) {
        int predicted = (preds[i] >= threshold) ? 1 : 0;
        int actual = targets[i];
        if (predicted == 1 && actual == 1) ++tp;
        else if (predicted == 1 && actual == 0) ++fp;
        else if (predicted == 0 && actual == 1) ++fn;
    }

    double precision = (tp + fp > 0) ? tp / static_cast<double>(tp + fp) : 0.0;
    double recall = (tp + fn > 0) ? tp / static_cast<double>(tp + fn) : 0.0;
    return (precision + recall > 0) ? 2.0 * precision * recall
        / (precision + recall) : 0.0;
}
