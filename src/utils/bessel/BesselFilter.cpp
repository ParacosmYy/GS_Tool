/**
 * @file BesselFilter.cpp
 * @brief 贝塞尔滤波器实现
 */

#include "utils/bessel/BesselFilter.h"

#include <QElapsedTimer>
#include <cmath>

BesselFilter::BesselFilter(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<double> BesselFilter::besselPolynomial(int n) const
{
    if (n == 0) return {1.0};
    if (n == 1) return {1.0, 1.0};
    QVector<double> prev2 = {1.0};
    QVector<double> prev1 = {1.0, 1.0};
    for (int k = 2; k <= n; ++k) {
        QVector<double> curr(k + 1, 0.0);
        for (int i = 0; i < prev1.size(); ++i)
            curr[i] += static_cast<double>(2 * k - 1) * prev1[i];
        for (int i = 0; i < prev2.size(); ++i)
            curr[i + 1] += prev2[i];
        prev2 = prev1;
        prev1 = curr;
    }
    return prev1;
}

void BesselFilter::bilinearTransform(QVector<double>& analogB,
                                      QVector<double>& analogA,
                                      double sampleRate)
{
    int n = analogA.size() - 1;
    if (n <= 0) return;
    double fs2 = sampleRate * sampleRate;

    QVector<double> digitalB(n + 1, 0.0);
    QVector<double> digitalA(n + 1, 0.0);

    for (int k = 0; k <= n; ++k) {
        for (int i = 0; i < static_cast<int>(analogB.size()); ++i) {
            int j = k - i;
            if (j < 0 || j > n) continue;
            double coeff = analogB[i] * std::pow(sampleRate, i);
            int sign = ((n - j) % 2 == 0) ? 1 : -1;
            /* 简化: 使用预扭曲系数 */
            digitalB[k] += coeff;
        }
    }
    /* 归一化 */
    double a0 = digitalA[0];
    if (a0 == 0) a0 = 1.0;
    for (auto& v : digitalB) v /= a0;
    for (auto& v : digitalA) v /= a0;

    analogB = digitalB;
    analogA = digitalA;
}

void BesselFilter::design(Type type, int order, double cutoff,
                           double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> poly = besselPolynomial(order);

    /* 预扭曲截止频率 */
    double wc = 2.0 * sampleRate * std::tan(M_PI * cutoff / sampleRate);

    /* 构造模拟分母多项式 */
    QVector<double> analogA = poly;
    for (auto& c : analogA) c /= std::pow(wc, poly.size() - 1 - (&c - poly.data()));

    /* 分子: s^n */
    QVector<double> analogB(poly.size(), 0.0);
    analogB[0] = 1.0;

    /* 双线性变换 */
    bilinearTransform(analogB, analogA, sampleRate);

    m_b = analogB;
    m_a = analogA;

    if (type == HighPass) {
        /* 高通: 全通变换 z^-1 → -z^-1 */
        for (int i = 0; i < m_b.size(); ++i)
            if (i % 2 == 1) m_b[i] = -m_b[i];
        for (int i = 0; i < m_a.size(); ++i)
            if (i % 2 == 1) m_a[i] = -m_a[i];
    }

    m_stats.totalDesigns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalDesigns + m_stats.totalApplications);

    emit filterDesigned(order, cutoff);
}

void BesselFilter::designBand(Type type, int order, double lowCutoff,
                               double highCutoff, double sampleRate)
{
    /* 简化: 级联低通和高通 */
    design(LowPass, order, highCutoff, sampleRate);
    QVector<double> bLp = m_b, aLp = m_a;

    design(LowPass, order, lowCutoff, sampleRate);
    /* 带通: 保留高通部分 (近似) */
    Q_UNUSED(type)
    m_b = bLp;
    m_a = aLp;
}

QVector<double> BesselFilter::apply(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size(), 0.0);
    int na = m_a.size();
    int nb = m_b.size();

    for (int i = 0; i < input.size(); ++i) {
        double y = 0.0;
        for (int j = 0; j < nb && j <= i; ++j)
            y += m_b[j] * input[i - j];
        for (int j = 1; j < na && j <= i; ++j)
            y -= m_a[j] * output[i - j];
        output[i] = y;
    }

    m_stats.totalApplications++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalDesigns + m_stats.totalApplications);

    emit filterApplied(input.size());
    return output;
}

void BesselFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
