/**
 * @file ChebyshevFilter.cpp
 * @brief 切比雪夫I型滤波器实现
 */

#include "utils/chebyshev/ChebyshevFilter.h"

#include <QElapsedTimer>
#include <cmath>

ChebyshevFilter::ChebyshevFilter(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

double ChebyshevFilter::chebyshevPoly(int n, double x) const
{
    if (n == 0) return 1.0;
    if (n == 1) return x;
    double prev2 = 1.0, prev1 = x;
    for (int i = 2; i <= n; ++i) {
        double curr = 2.0 * x * prev1 - prev2;
        prev2 = prev1;
        prev1 = curr;
    }
    return prev1;
}

void ChebyshevFilter::design(Type type, int order, double cutoff,
                              double sampleRate, double rippleDb)
{
    QElapsedTimer timer;
    timer.start();

    /* 预扭曲 */
    double wc = std::tan(M_PI * cutoff / sampleRate);
    double eps = std::sqrt(std::pow(10.0, rippleDb / 10.0) - 1.0);

    /* Chebyshev极点计算 */
    m_b.resize(order + 1);
    m_a.resize(order + 1, 0.0);

    /* 简化实现: 使用一阶/二阶级联 */
    double a0 = 1.0;
    QVector<double> aCoeffs(order + 1, 0.0);
    aCoeffs[0] = 1.0;

    for (int k = 0; k < order / 2; ++k) {
        double angle = M_PI * (2 * k + 1) / (2 * order);
        double sigma = -std::sinh(std::asinh(1.0 / eps) / order) * std::sin(angle);
        double omega = std::cosh(std::asinh(1.0 / eps) / order) * std::cos(angle);

        double w0sq = sigma * sigma + omega * omega;
        double w0 = std::sqrt(w0sq);
        double Q = w0 / (2.0 * std::abs(sigma));

        /* 频率缩放 */
        double w0n = w0 * wc;
        double alpha = std::sin(M_PI * cutoff / sampleRate);
        double cosw0 = std::cos(M_PI * cutoff / sampleRate);

        /* 双线性变换系数 */
        double beta = 0.5 * (1.0 - alpha) / (1.0 + alpha);
        double gamma = (0.5 + beta) * std::cos(M_PI * cutoff / sampleRate);

        double b0 = (0.5 + beta - gamma) / 2.0;
        double b1 = 0.5 + beta - gamma;
        double b2 = b0;
        double a1 = -2.0 * gamma / (0.5 + beta);
        double a2 = 2.0 * beta / (0.5 + beta);

        /* 与现有系数卷积 */
        QVector<double> newA(aCoeffs.size() + 2, 0.0);
        QVector<double> section = {1.0, a1, a2};
        for (int i = 0; i < static_cast<int>(aCoeffs.size()); ++i)
            for (int j = 0; j < 3; ++j)
                newA[i + j] += aCoeffs[i] * section[j];
        aCoeffs = newA;
        a0 *= (0.5 + beta);
    }

    if (order % 2 == 1) {
        double alpha = std::sin(M_PI * cutoff / sampleRate);
        double K = 2.0 * alpha / (1.0 + alpha);
        QVector<double> newA(aCoeffs.size() + 1, 0.0);
        for (int i = 0; i < static_cast<int>(aCoeffs.size()); ++i) {
            newA[i] += aCoeffs[i];
            newA[i + 1] += aCoeffs[i] * (K - 1.0);
        }
        aCoeffs = newA;
    }

    m_a = aCoeffs;

    /* 分子: 全部为gain */
    double gain = 1.0 / a0;
    for (int i = 0; i <= order; ++i)
        m_b[i] = gain;

    if (type == HighPass) {
        for (int i = 0; i < m_b.size(); ++i)
            if (i % 2 == 1) m_b[i] = -m_b[i];
        for (int i = 0; i < m_a.size(); ++i)
            if (i % 2 == 1) m_a[i] = -m_a[i];
    }

    /* 归一化 */
    double aSum = 0.0;
    for (auto v : m_a) aSum += v;
    if (aSum != 0) {
        double norm = 1.0 / aSum;
        for (auto& v : m_a) v *= norm;
        for (auto& v : m_b) v *= norm;
    }

    m_stats.totalDesigns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalDesigns + m_stats.totalApplications);

    emit filterDesigned(order, rippleDb);
}

QVector<double> ChebyshevFilter::apply(const QVector<double>& input)
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

void ChebyshevFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
