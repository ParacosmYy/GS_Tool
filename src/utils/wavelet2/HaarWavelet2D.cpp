/**
 * @file HaarWavelet2D.cpp
 * @brief 二维Haar小波变换实现
 */

#include "utils/wavelet2/HaarWavelet2D.h"

#include <QElapsedTimer>
#include <cmath>

HaarWavelet2D::HaarWavelet2D(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

HaarWavelet2D::Matrix HaarWavelet2D::forward(const Matrix& matrix)
{
    QElapsedTimer timer;
    timer.start();

    Matrix result = forwardRows(matrix);
    result = forwardCols(result);

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    int rows = result.size();
    int cols = rows > 0 ? result[0].size() : 0;
    emit transformCompleted(rows, cols);
    return result;
}

HaarWavelet2D::Matrix HaarWavelet2D::inverse(const Matrix& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    Matrix result = inverseCols(coeffs);
    result = inverseRows(result);

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return result;
}

HaarWavelet2D::Matrix HaarWavelet2D::denoise(
    const Matrix& matrix, double threshold)
{
    Matrix coeffs = forward(matrix);

    /* 软阈值：对每个系数应用阈值 */
    for (auto& row : coeffs) {
        for (auto& v : row) {
            if (v > threshold) v -= threshold;
            else if (v < -threshold) v += threshold;
            else v = 0.0;
        }
    }

    return inverse(coeffs);
}

HaarWavelet2D::Matrix HaarWavelet2D::forwardRows(const Matrix& m)
{
    if (m.isEmpty()) return m;
    int rows = m.size();
    int cols = m[0].size();
    Matrix result(rows, QVector<double>(cols, 0.0));

    for (int r = 0; r < rows; ++r) {
        int half = cols / 2;
        for (int c = 0; c < half; ++c) {
            double a = m[r][2 * c];
            double b = m[r][2 * c + 1];
            result[r][c] = (a + b) / std::sqrt(2.0);
            result[r][half + c] = (a - b) / std::sqrt(2.0);
        }
    }
    return result;
}

HaarWavelet2D::Matrix HaarWavelet2D::forwardCols(const Matrix& m)
{
    return transpose(forwardRows(transpose(m)));
}

HaarWavelet2D::Matrix HaarWavelet2D::inverseRows(const Matrix& m)
{
    if (m.isEmpty()) return m;
    int rows = m.size();
    int cols = m[0].size();
    Matrix result(rows, QVector<double>(cols, 0.0));
    double sq2 = std::sqrt(2.0);

    for (int r = 0; r < rows; ++r) {
        int half = cols / 2;
        for (int c = 0; c < half; ++c) {
            double avg = m[r][c];
            double diff = m[r][half + c];
            result[r][2 * c] = (avg + diff) / sq2;
            result[r][2 * c + 1] = (avg - diff) / sq2;
        }
    }
    return result;
}

HaarWavelet2D::Matrix HaarWavelet2D::inverseCols(const Matrix& m)
{
    return transpose(inverseRows(transpose(m)));
}

HaarWavelet2D::Matrix HaarWavelet2D::transpose(const Matrix& m)
{
    if (m.isEmpty()) return m;
    int rows = m.size();
    int cols = m[0].size();
    Matrix result(cols, QVector<double>(rows, 0.0));
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result[c][r] = m[r][c];
    return result;
}

void HaarWavelet2D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
