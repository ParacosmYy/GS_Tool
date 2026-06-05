/**
 * @file RaisedCosineFilter.cpp
 * @brief 升余弦滤波器实现 — 脉冲成形滤波器设计与滤波
 */

#include "utils/raised_cos/RaisedCosineFilter.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
RaisedCosineFilter::RaisedCosineFilter(QObject* parent)
    : QObject(parent)
    , m_filterType(FilterType::RaisedCosine)
    , m_timeSum(0.0)
{
}

/** @brief 设置滤波器类型 @param type 类型 */
void RaisedCosineFilter::setFilterType(FilterType type)
{
    m_filterType = type;
}

/** @brief 设计滤波器系数 @param taps 抽头数 @param rolloff 滚降系数 @param samplesPerSymbol 每符号采样数 @return 系数 */
QVector<double> RaisedCosineFilter::design(int taps, double rolloff,
                                            double samplesPerSymbol)
{
    QElapsedTimer timer;
    timer.start();

    taps = qMax(1, taps);
    rolloff = qBound(0.0, rolloff, 1.0);
    samplesPerSymbol = qMax(1.0, samplesPerSymbol);

    m_coeffs.resize(taps);
    double center = static_cast<double>(taps - 1) / 2.0;

    if (m_filterType == FilterType::RaisedCosine) {
        /* 升余弦滤波器 */
        for (int i = 0; i < taps; ++i) {
            double t = static_cast<double>(i) - center;
            double nT = t / samplesPerSymbol;

            if (qAbs(t) < 1e-10) {
                /* t = 0 */
                m_coeffs[i] = 1.0;
            } else if (qAbs(qAbs(nT) - 1.0 / (2.0 * rolloff)) < 1e-10) {
                /* t = ±T/(2*rolloff) */
                m_coeffs[i] = M_PI / 4.0;
            } else {
                /* 一般情况 */
                double num = qSin(M_PI * nT) * qCos(M_PI * rolloff * nT);
                double den = M_PI * nT * (1.0 - qPow(2.0 * rolloff * nT, 2));
                m_coeffs[i] = (qAbs(den) < 1e-15) ? 0.0 : num / den;
            }
        }
    } else {
        /* 根升余弦滤波器(RRC) */
        for (int i = 0; i < taps; ++i) {
            double t = static_cast<double>(i) - center;
            double nT = t / samplesPerSymbol;

            if (qAbs(t) < 1e-10) {
                /* t = 0 */
                m_coeffs[i] = 1.0 - rolloff + 4.0 * rolloff / M_PI;
            } else if (qAbs(qAbs(nT) - 1.0 / (4.0 * rolloff)) < 1e-10) {
                /* t = ±T/(4*rolloff) */
                double coeff = rolloff / qSqrt(2.0);
                double term1 = (1.0 + 2.0 / M_PI) * qSin(M_PI / (4.0 * rolloff));
                double term2 = (1.0 - 2.0 / M_PI) * qCos(M_PI / (4.0 * rolloff));
                m_coeffs[i] = coeff * (term1 + term2);
            } else {
                /* 一般情况 */
                double num1 = qSin(M_PI * nT * (1.0 - rolloff));
                double num2 = 4.0 * rolloff * nT * qCos(M_PI * nT * (1.0 + rolloff));
                double den = M_PI * nT * (1.0 - qPow(4.0 * rolloff * nT, 2));
                m_coeffs[i] = (qAbs(den) < 1e-15) ? 0.0 : (num1 + num2) / den;
            }
        }
    }

    /* 归一化: 使系数能量为1 */
    double energy = 0.0;
    for (double c : m_coeffs) energy += c * c;
    if (energy > 1e-15) {
        double norm = 1.0 / qSqrt(energy);
        for (auto& c : m_coeffs) c *= norm;
    }

    /* 更新统计 */
    ++m_stats.totalDesigned;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalDesigned + m_stats.totalFiltered;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit filterDesigned(taps);
    return m_coeffs;
}

/** @brief FIR滤波 @param input 输入信号 @return 滤波后信号 */
QVector<double> RaisedCosineFilter::filter(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    int n = input.size();
    int taps = m_coeffs.size();

    if (n == 0 || taps == 0) return output;

    output.resize(n);

    /* 卷积: 保持输出长度与输入相同(因果FIR) */
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < taps; ++k) {
            int idx = i - k;
            if (idx >= 0 && idx < n) {
                sum += m_coeffs[k] * input[idx];
            }
        }
        output[i] = sum;
    }

    /* 更新统计 */
    ++m_stats.totalFiltered;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalDesigned + m_stats.totalFiltered;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return output;
}

/** @brief 重置统计 */
void RaisedCosineFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
