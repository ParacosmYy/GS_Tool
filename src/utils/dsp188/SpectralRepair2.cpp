/**
 * @file SpectralRepair2.cpp
 * @brief SpectralRepair2 实现
 *
 * 实现频谱修复：AR模型参数估计、Levinson-Durbin递推、前向后向插值、间隙填充。
 */

#include "utils/dsp188/SpectralRepair2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralRepair2::SpectralRepair2(QObject *parent) : QObject(parent) {}
SpectralRepair2::~SpectralRepair2() = default;

/* ---- Configuration ---- */

void SpectralRepair2::setArOrder(int order) { m_arOrder = qMax(2, order); }
void SpectralRepair2::setMaxGapLength(int len) { m_maxGapLength = qMax(1, len); }

/* ---- Detect gaps (NaN markers) ---- */

QVector<QPair<int, int>> SpectralRepair2::detectGaps(
    const QVector<double>& signal) const
{
    QVector<QPair<int, int>> gaps;
    int i = 0;
    while (i < signal.size()) {
        if (qIsNaN(signal[i])) {
            int start = i;
            while (i < signal.size() && qIsNaN(signal[i])) ++i;
            gaps.append({start, i - 1});
        } else {
            ++i;
        }
    }
    return gaps;
}

/* ---- Autocorrelation estimation ---- */

QVector<double> SpectralRepair2::autocorrelation(
    const QVector<double>& seg, int maxLag) const
{
    int n = seg.size();
    QVector<double> acf(maxLag + 1, 0.0);
    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < n - lag; ++i) {
            if (!qIsNaN(seg[i]) && !qIsNaN(seg[i + lag])) {
                sum += seg[i] * seg[i + lag];
                count++;
            }
        }
        acf[lag] = (count > 0) ? sum / count : 0.0;
    }
    return acf;
}

/* ---- Levinson-Durbin recursion ---- */

QVector<double> SpectralRepair2::levinsonDurbin(
    const QVector<double>& autocorr) const
{
    int p = m_arOrder;
    QVector<double> a(p + 1, 0.0);
    a[0] = 1.0;
    double err = autocorr[0];

    for (int m = 1; m <= p; ++m) {
        double km = 0.0;
        for (int j = 0; j < m; ++j)
            km += a[j] * autocorr[m - j];
        if (qFabs(err) < 1e-30) break;
        km = -km / err;

        // Update coefficients
        QVector<double> aNew = a;
        for (int j = 1; j < m; ++j)
            aNew[j] = a[j] + km * a[m - j];
        aNew[m] = km;
        a = aNew;
        err *= (1.0 - km * km);
    }

    // Return AR coefficients (skip a[0]=1)
    QVector<double> ar(p);
    for (int i = 0; i < p; ++i) ar[i] = a[i + 1];
    return ar;
}

/* ---- AR parameter estimation ---- */

QVector<double> SpectralRepair2::estimateArParams(
    const QVector<double>& segment) const
{
    auto acf = autocorrelation(segment, m_arOrder);
    return levinsonDurbin(acf);
}

/* ---- Forward prediction ---- */

double SpectralRepair2::forwardPredict(const QVector<double>& signal,
                                        int pos,
                                        const QVector<double>& ar) const
{
    double pred = 0.0;
    int p = ar.size();
    for (int i = 0; i < p; ++i) {
        int idx = pos - 1 - i;
        if (idx >= 0 && idx < signal.size() && !qIsNaN(signal[idx]))
            pred -= ar[i] * signal[idx];
    }
    return pred;
}

/* ---- Backward prediction ---- */

double SpectralRepair2::backwardPredict(const QVector<double>& signal,
                                         int pos,
                                         const QVector<double>& ar) const
{
    double pred = 0.0;
    int p = ar.size();
    for (int i = 0; i < p; ++i) {
        int idx = pos + 1 + i;
        if (idx >= 0 && idx < signal.size() && !qIsNaN(signal[idx]))
            pred -= ar[i] * signal[idx];
    }
    return pred;
}

/* ---- Train AR model from known data ---- */

QVector<double> SpectralRepair2::trainArModel(
    const QVector<double>& signal) const
{
    // Collect known samples for training
    QVector<double> known;
    for (double v : signal)
        if (!qIsNaN(v)) known.append(v);

    if (known.size() < m_arOrder * 2) return QVector<double>(m_arOrder, 0.0);
    return estimateArParams(known);
}

/* ---- Fill a single gap with blended forward/backward ---- */

void SpectralRepair2::fillGap(QVector<double>& signal, int start, int end,
                               const QVector<double>& ar) const
{
    int gapLen = end - start + 1;
    for (int i = start; i <= end; ++i) {
        double fwd = forwardPredict(signal, i, ar);
        double bwd = backwardPredict(signal, i, ar);
        // Linear blend: weight forward more near start, backward near end
        double alpha = static_cast<double>(i - start) / gapLen;
        signal[i] = (1.0 - alpha) * fwd + alpha * bwd;
    }
}

/* ---- Main repair ---- */

QVector<double> SpectralRepair2::repair(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = signal;
    auto gaps = detectGaps(signal);

    // Train AR model from available data
    auto ar = trainArModel(signal);

    int filled = 0;
    for (const auto& gap : gaps) {
        int start = gap.first;
        int end = gap.second;
        if (end - start + 1 > m_maxGapLength) {
            // Fill only the edges for very large gaps
            int edgeLen = m_maxGapLength / 2;
            fillGap(result, start, qMin(start + edgeLen - 1, end), ar);
            fillGap(result, qMax(end - edgeLen + 1, start), end, ar);
        } else {
            fillGap(result, start, end, ar);
        }
        filled++;
    }

    m_stats.totalRepairs++;
    m_stats.signalLength = signal.size();
    m_stats.gapCount = gaps.size();
    m_stats.arOrder = m_arOrder;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRepairs;

    emit repairCompleted(filled, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void SpectralRepair2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
