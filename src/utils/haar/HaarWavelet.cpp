/**
 * @file HaarWavelet.cpp
 * @brief Haar小波变换实现
 */

#include "utils/haar/HaarWavelet.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

HaarWavelet::HaarWavelet(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QPair<QVector<double>, QVector<double>> HaarWavelet::forward(
    const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size() / 2;
    QVector<double> approx(n), detail(n);

    for (int i = 0; i < n; ++i) {
        approx[i] = (data[2 * i] + data[2 * i + 1]) / qSqrt(2.0);
        detail[i] = (data[2 * i] - data[2 * i + 1]) / qSqrt(2.0);
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalTransforms + m_stats.totalInverseTransforms, 1ULL);

    emit transformCompleted(data.size(), 1);
    return {approx, detail};
}

QVector<double> HaarWavelet::inverse(
    const QVector<double>& approx, const QVector<double>& detail)
{
    QElapsedTimer timer;
    timer.start();

    int n = approx.size();
    QVector<double> data(n * 2);

    for (int i = 0; i < n; ++i) {
        data[2 * i] = (approx[i] + detail[i]) / qSqrt(2.0);
        data[2 * i + 1] = (approx[i] - detail[i]) / qSqrt(2.0);
    }

    m_stats.totalInverseTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalTransforms + m_stats.totalInverseTransforms, 1ULL);

    emit inverseCompleted(data.size());
    return data;
}

HaarWavelet::Decomposition HaarWavelet::decompose(
    const QVector<double>& data, int levels)
{
    QElapsedTimer timer;
    timer.start();

    Decomposition result;
    int maxLevels = static_cast<int>(qLn(data.size()) / qLn(2.0));
    levels = qBound(1, levels, maxLevels);
    result.levels = levels;

    QVector<double> current = data;
    for (int l = 0; l < levels; ++l) {
        int half = current.size() / 2;
        if (half < 1) break;

        QVector<double> approx(half), detail(half);
        for (int i = 0; i < half; ++i) {
            approx[i] = (current[2 * i] + current[2 * i + 1]) / qSqrt(2.0);
            detail[i] = (current[2 * i] - current[2 * i + 1]) / qSqrt(2.0);
        }
        result.details.append(detail);
        current = approx;
    }
    result.approximation = current;

    m_stats.totalTransforms++;
    m_stats.totalLevelsProcessed += levels;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalTransforms + m_stats.totalInverseTransforms, 1ULL);

    emit transformCompleted(data.size(), levels);
    return result;
}

QVector<double> HaarWavelet::reconstruct(const Decomposition& decomp)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> current = decomp.approximation;
    for (int l = decomp.levels - 1; l >= 0; --l) {
        const QVector<double>& detail = decomp.details[l];
        int n = current.size();
        QVector<double> up(n * 2);

        for (int i = 0; i < n; ++i) {
            up[2 * i] = (current[i] + detail[i]) / qSqrt(2.0);
            up[2 * i + 1] = (current[i] - detail[i]) / qSqrt(2.0);
        }
        current = up;
    }

    m_stats.totalInverseTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalTransforms + m_stats.totalInverseTransforms, 1ULL);

    emit inverseCompleted(current.size());
    return current;
}

QVector<double> HaarWavelet::denoise(
    const QVector<double>& data, int levels, double threshold)
{
    Decomposition decomp = decompose(data, levels);

    /* 软阈值处理细节系数 */
    for (auto& detail : decomp.details) {
        for (int i = 0; i < detail.size(); ++i) {
            if (qAbs(detail[i]) <= threshold) {
                detail[i] = 0.0;
            } else {
                detail[i] = (detail[i] > 0 ? 1.0 : -1.0) *
                            (qAbs(detail[i]) - threshold);
            }
        }
    }

    return reconstruct(decomp);
}

void HaarWavelet::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
