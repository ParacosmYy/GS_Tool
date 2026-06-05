/**
 * @file WindowFunctionLib.cpp
 * @brief 窗函数库实现
 */

#include "WindowFunctionLib.h"
#include <QElapsedTimer>
#include <cmath>

WindowFunctionLib::WindowFunctionLib(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> WindowFunctionLib::create(Type type, int length, double param) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w(length);
    for (int i = 0; i < length; ++i) {
        double n = static_cast<double>(i) / (length - 1);
        switch (type) {
        case Hanning:
            w[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * n));
            break;
        case Hamming:
            w[i] = 0.54 - 0.46 * std::cos(2.0 * M_PI * n);
            break;
        case Blackman:
            w[i] = 0.42 - 0.5 * std::cos(2.0 * M_PI * n) + 0.08 * std::cos(4.0 * M_PI * n);
            break;
        case BlackmanHarris:
            w[i] = 0.35875 - 0.48829 * std::cos(2.0 * M_PI * n)
                   + 0.14128 * std::cos(4.0 * M_PI * n)
                   - 0.01168 * std::cos(6.0 * M_PI * n);
            break;
        case Kaiser: {
            double beta = (param > 0) ? param : 8.0;
            double x = 2.0 * i / (length - 1) - 1.0;
            /* 简化Bessel函数 */
            double arg = beta * std::sqrt(1.0 - x * x);
            double sum = 1.0;
            double term = 1.0;
            for (int k = 1; k <= 20; ++k) {
                term *= arg / (2.0 * k);
                sum += term * term;
            }
            w[i] = sum;
            break;
        }
        case FlatTop:
            w[i] = 0.21557895 - 0.41663158 * std::cos(2.0 * M_PI * n)
                   + 0.27726316 * std::cos(4.0 * M_PI * n)
                   - 0.08357895 * std::cos(6.0 * M_PI * n)
                   + 0.00694737 * std::cos(8.0 * M_PI * n);
            break;
        case Gaussian: {
            double sigma = (param > 0) ? param : 0.4;
            double x2 = (n - 0.5) / sigma;
            w[i] = std::exp(-0.5 * x2 * x2);
            break;
        }
        case Tukey: {
            double alpha = (param > 0) ? param : 0.5;
            if (n < alpha / 2)
                w[i] = 0.5 * (1.0 + std::cos(2.0 * M_PI / alpha * (n - 0.5)));
            else if (n > 1.0 - alpha / 2)
                w[i] = 0.5 * (1.0 + std::cos(2.0 * M_PI / alpha * (n - 1.0 + 0.5)));
            else
                w[i] = 1.0;
            break;
        }
        case Bartlett:
            w[i] = 1.0 - 2.0 * std::abs(n - 0.5);
            break;
        case Rectangular:
            w[i] = 1.0;
            break;
        }
    }

    m_stats.totalCreated++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalCreated + m_stats.totalApplied);
    return w;
}

QVector<double> WindowFunctionLib::apply(const QVector<double>& signal,
                                           const QVector<double>& window) const
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(signal.size(), window.size());
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) result[i] = signal[i] * window[i];

    m_stats.totalApplied++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalCreated + m_stats.totalApplied);
    return result;
}

QString WindowFunctionLib::typeName(Type type)
{
    switch (type) {
    case Hanning: return QStringLiteral("Hanning");
    case Hamming: return QStringLiteral("Hamming");
    case Blackman: return QStringLiteral("Blackman");
    case BlackmanHarris: return QStringLiteral("Blackman-Harris");
    case Kaiser: return QStringLiteral("Kaiser");
    case FlatTop: return QStringLiteral("Flat-Top");
    case Gaussian: return QStringLiteral("Gaussian");
    case Tukey: return QStringLiteral("Tukey");
    case Bartlett: return QStringLiteral("Bartlett");
    case Rectangular: return QStringLiteral("Rectangular");
    }
    return QStringLiteral("Unknown");
}

void WindowFunctionLib::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
