/**
 * @file DctTransform.cpp
 * @brief DCT变换实现
 */

#include "utils/dct/DctTransform.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
DctTransform::DctTransform(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 正向DCT-II */
QVector<double> DctTransform::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    QVector<double> output(N, 0.0);

    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n)
            sum += input[n] * qCos(M_PI * static_cast<double>(2 * n + 1) *
                                   static_cast<double>(k) / (2.0 * N));

        double scale = (k == 0) ? qSqrt(1.0 / N) : qSqrt(2.0 / N);
        output[k] = sum * scale;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalForward;
    double total = static_cast<double>(m_stats.totalForward + m_stats.totalInverse);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit transformCompleted(N, true);
    return output;
}

/** @brief 逆DCT */
QVector<double> DctTransform::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    int N = coefficients.size();
    QVector<double> output(N, 0.0);

    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            double scale = (k == 0) ? qSqrt(1.0 / N) : qSqrt(2.0 / N);
            sum += scale * coefficients[k] *
                   qCos(M_PI * static_cast<double>(2 * n + 1) *
                        static_cast<double>(k) / (2.0 * N));
        }
        output[n] = sum;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalInverse;
    double total = static_cast<double>(m_stats.totalForward + m_stats.totalInverse);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit transformCompleted(N, false);
    return output;
}

/** @brief 计算第k个DCT系数 */
double DctTransform::coefficient(const QVector<double>& input, int k) const
{
    int N = input.size();
    double sum = 0.0;
    for (int n = 0; n < N; ++n)
        sum += input[n] * qCos(M_PI * static_cast<double>(2 * n + 1) *
                               static_cast<double>(k) / (2.0 * N));

    double scale = (k == 0) ? qSqrt(1.0 / N) : qSqrt(2.0 / N);
    return sum * scale;
}

/** @brief 重置统计 */
void DctTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
