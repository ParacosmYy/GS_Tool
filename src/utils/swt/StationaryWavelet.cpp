/**
 * @file StationaryWavelet.cpp
 * @brief 静态小波变换实现
 */

#include "StationaryWavelet.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

StationaryWavelet::StationaryWavelet(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

StationaryWavelet::Decomposition StationaryWavelet::decompose(
    const QVector<double>& signal, int level, WaveletType type)
{
    QElapsedTimer timer;
    timer.start();

    Decomposition result;
    int n = signal.size();

    if (n < 4 || level < 1) {
        result.approximation = signal;
        m_stats.totalDecomposed++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalDecomposed);
        return result;
    }

    QVector<double> loD, hiD;
    getFilters(type, loD, hiD);
    int fLen = loD.size();

    QVector<double> current = signal;

    for (int lev = 0; lev < level; ++lev) {
        int curN = current.size();
        QVector<double> approx(curN, 0.0);
        QVector<double> detail(curN, 0.0);

        /* 上采样滤波器(每隔2^lev位置插入0) */
        int step = 1 << lev;
        for (int i = 0; i < curN; ++i) {
            double aSum = 0.0, dSum = 0.0;
            for (int k = 0; k < fLen; ++k) {
                int idx = (i - k * step) % curN;
                if (idx < 0) idx += curN;
                aSum += loD[k] * current[idx];
                dSum += hiD[k] * current[idx];
            }
            approx[i] = aSum;
            detail[i] = dSum;
        }

        result.details.append(detail);
        current = approx;
    }

    result.approximation = current;

    m_stats.totalDecomposed++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalDecomposed + m_stats.totalReconstructed;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit decomposed(level, signal.size());
    return result;
}

QVector<double> StationaryWavelet::reconstruct(const Decomposition& decomp,
                                                  WaveletType type)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> loD, hiD;
    getFilters(type, loD, hiD);
    int fLen = loD.size();

    QVector<double> current = decomp.approximation;
    int level = decomp.details.size();

    for (int lev = level - 1; lev >= 0; --lev) {
        int n = current.size();
        QVector<double> reconstructed(n, 0.0);
        const auto& detail = decomp.details[lev];
        int step = 1 << lev;

        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < fLen; ++k) {
                int idx = (i + k * step) % n;
                if (idx < 0) idx += n;
                reconstructed[idx] += loD[k] * current[i] + hiD[k] * detail[i];
            }
        }
        current = reconstructed;
    }

    m_stats.totalReconstructed++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalDecomposed + m_stats.totalReconstructed;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return current;
}

QVector<double> StationaryWavelet::denoise(const QVector<double>& signal,
                                              int level, double threshold,
                                              WaveletType type)
{
    auto decomp = decompose(signal, level, type);

    /* 软阈值 */
    for (auto& detail : decomp.details) {
        for (int i = 0; i < detail.size(); ++i) {
            if (std::abs(detail[i]) <= threshold) {
                detail[i] = 0.0;
            } else {
                detail[i] = (detail[i] > 0 ? 1.0 : -1.0) * (std::abs(detail[i]) - threshold);
            }
        }
    }

    return reconstruct(decomp, type);
}

double StationaryWavelet::universalThreshold(const QVector<double>& detailCoeffs)
{
    int n = detailCoeffs.size();
    if (n == 0) return 0.0;

    double sumSq = 0.0;
    for (double c : detailCoeffs)
        sumSq += c * c;
    double sigma = std::sqrt(sumSq / n);

    return sigma * std::sqrt(2.0 * std::log(static_cast<double>(n)));
}

void StationaryWavelet::getFilters(WaveletType type,
                                     QVector<double>& lo, QVector<double>& hi) const
{
    switch (type) {
    case Haar:
        lo = {1.0 / std::sqrt(2.0), 1.0 / std::sqrt(2.0)};
        hi = {1.0 / std::sqrt(2.0), -1.0 / std::sqrt(2.0)};
        break;
    case DB2:
        lo = {(1.0 + std::sqrt(3.0)) / (4.0 * std::sqrt(2.0)),
              (3.0 + std::sqrt(3.0)) / (4.0 * std::sqrt(2.0)),
              (3.0 - std::sqrt(3.0)) / (4.0 * std::sqrt(2.0)),
              (1.0 - std::sqrt(3.0)) / (4.0 * std::sqrt(2.0))};
        hi = {lo[3], -lo[2], lo[1], -lo[0]};
        break;
    case DB4:
        lo = {-0.010597401784997278, 0.03288301166698298,
              0.030841381835986965, -0.18703481171888144,
              -0.02798376941698385, 0.6308807679295904,
              0.7148465705525415, 0.23037781330885523};
        hi = {-lo[7], lo[6], -lo[5], lo[4], -lo[3], lo[2], -lo[1], lo[0]};
        break;
    }
}

StationaryWavelet::Stats StationaryWavelet::stats() const { return m_stats; }

void StationaryWavelet::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
