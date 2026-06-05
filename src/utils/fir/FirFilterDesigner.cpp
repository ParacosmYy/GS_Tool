/**
 * @file FirFilterDesigner.cpp
 * @brief FIR滤波器设计器实现 — 窗口法设计
 */

#include "utils/fir/FirFilterDesigner.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

FirFilterDesigner::FirFilterDesigner(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

FirFilterDesigner::DesignResult FirFilterDesigner::design(
    FilterType type, int order, double cutoffNorm, WindowType window) const
{
    DesignResult result;
    if (order < 1 || cutoffNorm <= 0.0 || cutoffNorm >= 0.5) return result;

    QElapsedTimer timer;
    timer.start();

    int n = order + 1;
    int half = order / 2;

    /* 理想脉冲响应 */
    QVector<double> ideal(n, 0.0);
    double wc = 2.0 * M_PI * cutoffNorm;

    for (int i = 0; i < n; ++i) {
        if (i == half) {
            ideal[i] = 2.0 * cutoffNorm;
        } else {
            ideal[i] = qSin(wc * (i - half)) / (M_PI * (i - half));
        }
    }

    if (type == FilterType::HighPass) {
        for (int i = 0; i < n; ++i) {
            ideal[i] = (i == half ? 1.0 : 0.0) - ideal[i];
        }
    }

    result.coefficients = applyWindow(ideal, window, 0.0);
    result.cutoffNorm = cutoffNorm;

    m_stats.totalDesigns++;
    m_stats.totalCoefficientsGenerated += n;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDesigns;

    emit designCompleted(order, cutoffNorm);
    return result;
}

FirFilterDesigner::DesignResult FirFilterDesigner::designBand(
    FilterType type, int order, double lowNorm, double highNorm,
    WindowType window) const
{
    DesignResult result;
    if (order < 1 || lowNorm >= highNorm || lowNorm <= 0.0 || highNorm >= 0.5)
        return result;

    QElapsedTimer timer;
    timer.start();

    int n = order + 1;
    int half = order / 2;
    double wl = 2.0 * M_PI * lowNorm;
    double wh = 2.0 * M_PI * highNorm;

    QVector<double> ideal(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double hiPulse = (i == half) ? wh / M_PI : qSin(wh * (i - half)) / (M_PI * (i - half));
        double loPulse = (i == half) ? wl / M_PI : qSin(wl * (i - half)) / (M_PI * (i - half));
        ideal[i] = hiPulse - loPulse;
    }

    if (type == FilterType::BandStop) {
        for (int i = 0; i < n; ++i) {
            ideal[i] = (i == half ? 1.0 : 0.0) - ideal[i];
        }
    }

    result.coefficients = applyWindow(ideal, window, 0.0);

    m_stats.totalDesigns++;
    m_stats.totalCoefficientsGenerated += n;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDesigns;

    emit designCompleted(order, (lowNorm + highNorm) / 2.0);
    return result;
}

QPair<int, double> FirFilterDesigner::kaiserParams(double rippleDb,
                                                    double transitionWidth) const
{
    double beta = 0.0;
    if (rippleDb > 50.0) beta = 0.1102 * (rippleDb - 8.7);
    else if (rippleDb >= 21.0) beta = 0.5842 * qPow(rippleDb - 21.0, 0.4) + 0.07886 * (rippleDb - 21.0);

    int order = qCeil((rippleDb - 7.95) / (14.36 * transitionWidth));
    order = qMax(order, 1);

    return {order, beta};
}

QVector<double> FirFilterDesigner::generateWindow(int length, WindowType type,
                                                   double beta) const
{
    QVector<double> w(length, 1.0);
    if (length < 1) return w;

    int half = (length - 1) / 2;

    for (int i = 0; i < length; ++i) {
        double n = static_cast<double>(i - half) / half;
        switch (type) {
        case WindowType::Rectangular: w[i] = 1.0; break;
        case WindowType::Hamming:     w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (length - 1)); break;
        case WindowType::Hanning:     w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (length - 1))); break;
        case WindowType::Blackman:    w[i] = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (length - 1))
                                              + 0.08 * qCos(4.0 * M_PI * i / (length - 1)); break;
        case WindowType::Kaiser: {
            /* 简化Kaiser — 使用近似的Bessel I0 */
            double arg = beta * qSqrt(qMax(0.0, 1.0 - n * n));
            double sum = 1.0, term = 1.0;
            for (int k = 1; k <= 20; ++k) {
                term *= (arg / (2.0 * k)) * (arg / (2.0 * k));
                sum += term;
            }
            double i0Beta = 1.0;
            arg = beta;
            term = 1.0; sum = 1.0;
            for (int k = 1; k <= 20; ++k) {
                term *= (arg / (2.0 * k)) * (arg / (2.0 * k));
                i0Beta += term;
            }
            w[i] = (i0Beta > 0) ? sum / i0Beta : 1.0;
            break;
        }
        }
    }
    return w;
}

QVector<QPair<double, double>> FirFilterDesigner::frequencyResponse(
    const QVector<double>& coeffs, int numPoints) const
{
    QVector<QPair<double, double>> response;
    response.reserve(numPoints);

    int n = coeffs.size();
    for (int k = 0; k < numPoints; ++k) {
        double freq = static_cast<double>(k) / numPoints;
        double real = 0.0, imag = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * freq * i;
            real += coeffs[i] * qCos(angle);
            imag += coeffs[i] * qSin(angle);
        }
        double mag = qSqrt(real * real + imag * imag);
        double db = (mag > 0) ? 20.0 * std::log10(mag) : -120.0;
        response.append({freq, db});
    }
    return response;
}

QVector<double> FirFilterDesigner::applyWindow(const QVector<double>& ideal,
                                                WindowType type, double beta) const
{
    QVector<double> window = generateWindow(ideal.size(), type, beta);
    QVector<double> result(ideal.size());
    for (int i = 0; i < ideal.size(); ++i) {
        result[i] = ideal[i] * window[i];
    }
    return result;
}

double FirFilterDesigner::sinc(double x)
{
    if (qAbs(x) < 1e-10) return 1.0;
    return qSin(M_PI * x) / (M_PI * x);
}

void FirFilterDesigner::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
