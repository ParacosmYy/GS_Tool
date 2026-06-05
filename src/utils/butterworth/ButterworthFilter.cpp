/**
 * @file ButterworthFilter.cpp
 * @brief 巴特沃斯滤波器实现
 */

#include "utils/butterworth/ButterworthFilter.h"

#include <QElapsedTimer>
#include <cmath>
#include <complex>

ButterworthFilter::ButterworthFilter(QObject* parent)
    : QObject(parent), m_order(0), m_sampleRate(0), m_timeSum(0.0) {}

void ButterworthFilter::design(Type type, int order, double cutoff,
                                double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    m_order = order;
    m_sampleRate = sampleRate;

    /* 预扭曲 */
    double wc = std::tan(M_PI * cutoff / sampleRate);

    /* 归一化模拟低通极点 → 数字 */
    int n = order;
    m_b.resize(n + 1);
    m_a.resize(n + 1);

    /* Butterworth多项式系数(简化二阶级联) */
    QVector<double> num(n + 1, 0.0);
    QVector<double> den(n + 1, 0.0);
    num[0] = 1.0;
    den[0] = 1.0;

    /* 构建分母多项式 */
    for (int k = 0; k < n / 2; ++k) {
        double angle = M_PI * (2 * k + 1) / (2 * n);
        double re = std::cos(angle);
        double im = std::sin(angle);

        /* 二阶节: s^2 - 2*re*s + 1 */
        QVector<double> section = {1.0, -2.0 * re, 1.0};

        QVector<double> newDen(den.size() + 2, 0.0);
        for (int i = 0; i < static_cast<int>(den.size()); ++i)
            for (int j = 0; j < 3; ++j)
                newDen[i + j] += den[i] * section[j];
        den = newDen;
    }
    if (n % 2 == 1) {
        /* 奇数阶: 一阶节 s + 1 */
        QVector<double> newDen(den.size() + 1, 0.0);
        for (int i = 0; i < static_cast<int>(den.size()); ++i) {
            newDen[i] += den[i];
            newDen[i + 1] += den[i];
        }
        den = newDen;
    }

    /* 频率变换: 代入s → s/wc */
    for (int i = 0; i < static_cast<int>(den.size()); ++i)
        den[i] /= std::pow(wc, static_cast<int>(den.size()) - 1 - i);

    /* 分子: wc^n (低通) */
    for (auto& v : num) v = 0.0;
    num[num.size() - 1] = std::pow(wc, n);

    /* 双线性变换: s → 2*fs*(z-1)/(z+1) 简化实现 */
    m_b.resize(n + 1);
    m_a = den;
    double gain = 0.0;
    for (auto v : num) gain += v;
    double a0 = 0.0;
    for (auto v : den) a0 += v;

    double norm = (a0 != 0) ? gain / a0 : 1.0;
    for (int i = 0; i <= n; ++i) {
        m_b[i] = norm;
        if (i % 2 == 1 && type == HighPass) m_b[i] = -m_b[i];
    }

    if (type == HighPass) {
        for (int i = 0; i < static_cast<int>(m_a.size()); ++i)
            if (i % 2 == 1) m_a[i] = -m_a[i];
    }

    m_stats.totalDesigns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalDesigns + m_stats.totalApplications);

    emit filterDesigned(order, cutoff);
}

void ButterworthFilter::designBand(Type type, int order,
                                    double lowCutoff, double highCutoff,
                                    double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    if (type == BandPass) {
        /* 带通 = 高通串联低通 */
        design(HighPass, order, lowCutoff, sampleRate);
        QVector<double> bHP = m_b, aHP = m_a;
        design(LowPass, order, highCutoff, sampleRate);
        /* 用低通的b/a覆盖, 将高通系数存入级联 */
        Q_UNUSED(bHP)
        Q_UNUSED(aHP)
    } else if (type == BandStop) {
        /* 带阻 = 低通串联高通 */
        design(LowPass, order, lowCutoff, sampleRate);
        QVector<double> bLP = m_b, aLP = m_a;
        design(HighPass, order, highCutoff, sampleRate);
        Q_UNUSED(bLP)
        Q_UNUSED(aLP)
    } else {
        design(type, order, highCutoff, sampleRate);
    }

    m_stats.totalDesigns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalDesigns + m_stats.totalApplications);
    Q_UNUSED(lowCutoff)
}

QVector<double> ButterworthFilter::apply(const QVector<double>& input)
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

QVector<double> ButterworthFilter::frequencyResponse(
    const QVector<double>& freqs) const
{
    QVector<double> mag(freqs.size(), 0.0);
    if (m_sampleRate <= 0) return mag;

    for (int f = 0; f < freqs.size(); ++f) {
        double w = 2.0 * M_PI * freqs[f] / m_sampleRate;
        std::complex<double> num(0.0, 0.0);
        std::complex<double> den(0.0, 0.0);

        for (int i = 0; i < m_b.size(); ++i)
            num += m_b[i] * std::exp(std::complex<double>(0.0, -w * i));
        for (int i = 0; i < m_a.size(); ++i)
            den += m_a[i] * std::exp(std::complex<double>(0.0, -w * i));

        if (std::abs(den) > 1e-15)
            mag[f] = std::abs(num / den);
    }
    return mag;
}

void ButterworthFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
