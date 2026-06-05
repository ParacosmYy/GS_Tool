/**
 * @file FocalLoss.cpp
 * @brief Focal Loss损失函数实现
 */

#include "FocalLoss.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

FocalLoss::FocalLoss(double gamma, double alpha, QObject* parent)
    : QObject(parent)
    , m_gamma(gamma)
    , m_alpha(alpha)
    , m_timeSum(0.0)
{
}

double FocalLoss::binaryLoss(double prediction, int target) const
{
    double p = qBound(1e-7, prediction, 1.0 - 1e-7);
    double pt = (target == 1) ? p : (1.0 - p);
    double alphaT = (target == 1) ? m_alpha : (1.0 - m_alpha);
    double focalWeight = std::pow(1.0 - pt, m_gamma);

    return -alphaT * focalWeight * std::log(pt);
}

double FocalLoss::binaryLossBatch(const QVector<double>& predictions,
                                     const QVector<int>& targets) const
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(predictions.size(), targets.size());
    if (n == 0) return 0.0;

    double totalLoss = 0.0;
    for (int i = 0; i < n; ++i)
        totalLoss += binaryLoss(predictions[i], targets[i]);

    const_cast<FocalLoss*>(this)->m_stats.totalComputed++;
    const_cast<FocalLoss*>(this)->m_timeSum += timer.elapsed();
    const_cast<FocalLoss*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalComputed;

    emit const_cast<FocalLoss*>(this)->computed(n, totalLoss / n);
    return totalLoss / n;
}

double FocalLoss::multiClassLoss(const QVector<double>& classProbs,
                                    int targetClass) const
{
    if (targetClass < 0 || targetClass >= classProbs.size()) return 0.0;

    double pt = qBound(1e-7, classProbs[targetClass], 1.0 - 1e-7);
    double focalWeight = std::pow(1.0 - pt, m_gamma);

    return -focalWeight * std::log(pt);
}

double FocalLoss::multiClassLossBatch(const QVector<QVector<double>>& batchProbs,
                                         const QVector<int>& targets) const
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(batchProbs.size(), targets.size());
    if (n == 0) return 0.0;

    double totalLoss = 0.0;
    for (int i = 0; i < n; ++i)
        totalLoss += multiClassLoss(batchProbs[i], targets[i]);

    const_cast<FocalLoss*>(this)->m_stats.totalComputed++;
    const_cast<FocalLoss*>(this)->m_timeSum += timer.elapsed();
    const_cast<FocalLoss*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalComputed;

    return totalLoss / n;
}

double FocalLoss::gradient(double prediction, int target) const
{
    double p = qBound(1e-7, prediction, 1.0 - 1e-7);
    double pt = (target == 1) ? p : (1.0 - p);
    double alphaT = (target == 1) ? m_alpha : (1.0 - m_alpha);

    double focalWeight = std::pow(1.0 - pt, m_gamma);
    double deriv = (target == 1) ? -1.0 : 1.0;

    return alphaT * deriv * focalWeight *
           (m_gamma * std::log(pt) * (target == 1 ? p : (1.0 - p)) + 1.0 - pt);
}

void FocalLoss::setGamma(double gamma) { m_gamma = gamma; }
void FocalLoss::setAlpha(double alpha) { m_alpha = alpha; }
double FocalLoss::gamma() const { return m_gamma; }
double FocalLoss::alpha() const { return m_alpha; }

FocalLoss::Stats FocalLoss::stats() const { return m_stats; }

void FocalLoss::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
