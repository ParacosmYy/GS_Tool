/**
 * @file GaussianMixtureModel.cpp
 * @brief 高斯混合模型实现 — EM算法
 */

#include "utils/gmm/GaussianMixtureModel.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>
#include <limits>

GaussianMixtureModel::GaussianMixtureModel(QObject* parent)
    : QObject(parent), m_k(2), m_timeSum(0.0) {}

void GaussianMixtureModel::setComponents(int k) { m_k = qMax(1, k); }

double GaussianMixtureModel::train(const QVector<double>& data,
                                    int maxIterations, double tolerance)
{
    if (data.size() < m_k) return -std::numeric_limits<double>::infinity();

    QElapsedTimer timer;
    timer.start();

    initialize(data);

    int n = data.size();
    double prevLogLik = -std::numeric_limits<double>::infinity();
    int iterUsed = 0;

    for (int iter = 0; iter < maxIterations; ++iter) {
        iterUsed = iter + 1;

        /* E-step: 计算每个样本属于每个分量的后验概率 */
        QVector<QVector<double>> resp(n, QVector<double>(m_k, 0.0));
        for (int i = 0; i < n; ++i) {
            double total = 0.0;
            for (int j = 0; j < m_k; ++j) {
                resp[i][j] = m_components[j].weight *
                    gaussianPdf(data[i], m_components[j].mean, m_components[j].variance);
                total += resp[i][j];
            }
            if (total > 0) for (int j = 0; j < m_k; ++j) resp[i][j] /= total;
        }

        /* M-step: 更新参数 */
        for (int j = 0; j < m_k; ++j) {
            double nk = 0.0;
            for (int i = 0; i < n; ++i) nk += resp[i][j];

            if (nk < 1e-10) continue;

            m_components[j].weight = nk / n;

            double newMean = 0.0;
            for (int i = 0; i < n; ++i) newMean += resp[i][j] * data[i];
            newMean /= nk;
            m_components[j].mean = newMean;

            double newVar = 0.0;
            for (int i = 0; i < n; ++i) {
                double diff = data[i] - newMean;
                newVar += resp[i][j] * diff * diff;
            }
            newVar /= nk;
            m_components[j].variance = qMax(newVar, 1e-6);
        }

        /* 计算log似然 */
        double logLik = 0.0;
        for (int i = 0; i < n; ++i) {
            double p = 0.0;
            for (int j = 0; j < m_k; ++j) {
                p += m_components[j].weight *
                    gaussianPdf(data[i], m_components[j].mean, m_components[j].variance);
            }
            if (p > 0) logLik += qLn(p);
        }

        if (qAbs(logLik - prevLogLik) < tolerance) break;
        prevLogLik = logLik;
    }

    m_stats.totalTrainings++;
    m_stats.totalIterationsUsed += iterUsed;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTrainings;

    emit trainingCompleted(iterUsed, prevLogLik);
    return prevLogLik;
}

double GaussianMixtureModel::probabilityDensity(double x) const
{
    double p = 0.0;
    for (const auto& c : m_components) {
        p += c.weight * gaussianPdf(x, c.mean, c.variance);
    }
    return p;
}

int GaussianMixtureModel::assignComponent(double x) const
{
    if (m_components.isEmpty()) return -1;
    int best = 0;
    double bestResp = 0.0;
    for (int j = 0; j < m_components.size(); ++j) {
        double r = m_components[j].weight *
            gaussianPdf(x, m_components[j].mean, m_components[j].variance);
        if (r > bestResp) { bestResp = r; best = j; }
    }
    return best;
}

double GaussianMixtureModel::gaussianPdf(double x, double mean, double variance)
{
    double diff = x - mean;
    return qExp(-diff * diff / (2.0 * variance)) / (qSqrt(2.0 * M_PI * variance));
}

void GaussianMixtureModel::initialize(const QVector<double>& data)
{
    m_components.clear();
    if (data.isEmpty()) return;

    double dMin = data[0], dMax = data[0];
    for (double v : data) { dMin = qMin(dMin, v); dMax = qMax(dMax, v); }
    double range = (dMax - dMin > 0) ? dMax - dMin : 1.0;

    for (int j = 0; j < m_k; ++j) {
        Component c;
        c.mean = dMin + range * (j + 0.5) / m_k;
        c.variance = range * range / (4.0 * m_k);
        c.weight = 1.0 / m_k;
        m_components.append(c);
    }
}

void GaussianMixtureModel::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
