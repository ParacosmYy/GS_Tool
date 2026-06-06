/**
 * @file WHT4.cpp
 * @brief WHT4 实现
 *
 * 实现沃尔什-哈达玛变换：自然序/序列序蝶形、原位快速计算、2D变换。
 */

#include "utils/fft187/WHT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WHT4::WHT4(QObject *parent) : QObject(parent) {}
WHT4::~WHT4() = default;

/* ---- Configuration ---- */

void WHT4::setSequencyOrder(bool enabled) { m_sequencyOrder = enabled; }

/* ---- Helpers ---- */

int WHT4::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- Gray code rank ---- */

int WHT4::grayCodeRank(int index) const
{
    int rank = index;
    int shift = 1;
    while ((index >> shift) > 0) {
        rank ^= (index >> shift);
        shift++;
    }
    return rank;
}

/* ---- Bit-reversal permutation ---- */

void WHT4::bitReversePermute(QVector<double>& data) const
{
    int N = data.size();
    int log2N = 0;
    while ((1 << log2N) < N) ++log2N;

    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < log2N; ++b)
            j = (j << 1) | ((i >> b) & 1);
        if (j > i) std::swap(data[i], data[j]);
    }
}

/* ---- In-place Hadamard butterfly ---- */

void WHT4::hadamardButterfly(QVector<double>& data) const
{
    int N = data.size();
    for (int len = 2; len <= N; len *= 2) {
        int half = len / 2;
        for (int i = 0; i < N; i += len) {
            for (int j = 0; j < half; ++j) {
                double a = data[i + j];
                double b = data[i + j + half];
                data[i + j] = a + b;
                data[i + j + half] = a - b;
            }
        }
    }
}

/* ---- Natural order WHT ---- */

QVector<double> WHT4::naturalOrder(const QVector<double>& input) const
{
    int N = input.size();
    if (N == 0) return {};
    // Must be power of 2
    if (N & (N - 1)) {
        int N2 = nextPow2(N);
        QVector<double> padded(N2, 0.0);
        for (int i = 0; i < N; ++i) padded[i] = input[i];
        N = N2;
        QVector<double> result = padded;
        hadamardButterfly(result);
        return result;
    }

    QVector<double> result = input;
    hadamardButterfly(result);
    return result;
}

/* ---- Sequency-ordered WHT ---- */

QVector<double> WHT4::sequencyOrderWHT(const QVector<double>& input) const
{
    int N = input.size();
    if (N == 0) return {};

    // First compute natural order
    auto result = naturalOrder(input);

    // Then reorder by Gray code rank (bit-reverse first, then Gray)
    int log2N = 0;
    while ((1 << log2N) < result.size()) ++log2N;

    QVector<double> seqOrdered(result.size());
    for (int i = 0; i < result.size(); ++i) {
        // Bit-reverse i
        int bri = 0;
        for (int b = 0; b < log2N; ++b)
            bri = (bri << 1) | ((i >> b) & 1);
        // Gray code of bit-reversed index gives sequency order
        int seq = bri ^ (bri >> 1);
        seqOrdered[seq] = result[i];
    }
    return seqOrdered;
}

/* ---- Forward ---- */

QVector<double> WHT4::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (input.isEmpty()) return result;

    if (m_sequencyOrder)
        result = sequencyOrderWHT(input);
    else
        result = naturalOrder(input);

    m_stats.totalTransforms++;
    m_stats.transformSize = result.size();
    m_stats.sequencyOrder = m_sequencyOrder;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(result.size(), timer.elapsed());
    return result;
}

/* ---- Inverse (WHT is self-inverse, just scale) ---- */

QVector<double> WHT4::inverse(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (input.isEmpty()) return result;

    // WHT is its own inverse up to scaling factor 1/N
    if (m_sequencyOrder)
        result = sequencyOrderWHT(input);
    else
        result = naturalOrder(input);

    double scale = 1.0 / result.size();
    for (int i = 0; i < result.size(); ++i)
        result[i] *= scale;

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(input.size(), timer.elapsed());
    return result;
}

/* ---- 2D WHT ---- */

QVector<QVector<double>> WHT4::transform2D(const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    int rows = input.size();
    if (rows == 0) return {};
    int cols = input[0].size();
    if (cols == 0) return {};

    // Transform rows
    QVector<QVector<double>> temp(rows);
    for (int r = 0; r < rows; ++r)
        temp[r] = forward(input[r]);

    // Transform columns
    int cSize = nextPow2(rows);
    QVector<QVector<double>> result(rows, QVector<double>(cols));
    for (int c = 0; c < cols; ++c) {
        QVector<double> col(cSize, 0.0);
        for (int r = 0; r < rows; ++r) col[r] = temp[r][c];
        auto colResult = forward(col);
        for (int r = 0; r < rows; ++r) result[r][c] = (r < colResult.size()) ? colResult[r] : 0.0;
    }

    return result;
}

/* ---- Power spectrum ---- */

QVector<double> WHT4::powerSpectrum(const QVector<double>& input)
{
    auto transformed = forward(input);
    QVector<double> spectrum(transformed.size());
    for (int i = 0; i < transformed.size(); ++i)
        spectrum[i] = transformed[i] * transformed[i];
    return spectrum;
}

/* ---- Reset ---- */

void WHT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
