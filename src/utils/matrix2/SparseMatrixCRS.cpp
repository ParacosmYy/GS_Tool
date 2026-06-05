/**
 * @file SparseMatrixCRS.cpp
 * @brief 稀疏矩阵(CRS)实现
 */

#include "SparseMatrixCRS.h"
#include <QElapsedTimer>
#include <algorithm>

SparseMatrixCRS::SparseMatrixCRS(int rows, int cols, QObject* parent)
    : QObject(parent)
    , m_rows(qMax(0, rows))
    , m_cols(qMax(0, cols))
    , m_rowPtr(m_rows + 1, 0)
    , m_timeSum(0.0)
{
}

void SparseMatrixCRS::setValue(int row, int col, double value)
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return;

    /* 在行中找插入位置 */
    int start = m_rowPtr[row];
    int end = m_rowPtr[row + 1];
    int pos = start;

    while (pos < end && m_colIdx[pos] < col) pos++;

    if (pos < end && m_colIdx[pos] == col) {
        if (std::abs(value) < 1e-15) {
            /* 删除零值 */
            m_values.remove(pos);
            m_colIdx.remove(pos);
            for (int i = row + 1; i <= m_rows; ++i) m_rowPtr[i]--;
        } else {
            m_values[pos] = value;
        }
    } else if (std::abs(value) >= 1e-15) {
        m_values.insert(pos, value);
        m_colIdx.insert(pos, col);
        for (int i = row + 1; i <= m_rows; ++i) m_rowPtr[i]++;
    }
}

double SparseMatrixCRS::value(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return 0.0;

    int start = m_rowPtr[row];
    int end = m_rowPtr[row + 1];

    for (int i = start; i < end; ++i) {
        if (m_colIdx[i] == col) return m_values[i];
        if (m_colIdx[i] > col) break;
    }
    return 0.0;
}

QVector<double> SparseMatrixCRS::multiply(const QVector<double>& vec) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result(m_rows, 0.0);
    if (vec.size() != m_cols) return result;

    for (int i = 0; i < m_rows; ++i) {
        double sum = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            sum += m_values[j] * vec[m_colIdx[j]];
        result[i] = sum;
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return result;
}

SparseMatrixCRS* SparseMatrixCRS::multiply(const SparseMatrixCRS& other) const
{
    QElapsedTimer timer;
    timer.start();

    SparseMatrixCRS* result = new SparseMatrixCRS(m_rows, other.m_cols, parent());

    for (int i = 0; i < m_rows; ++i) {
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            int k = m_colIdx[j];
            double aVal = m_values[j];
            for (int l = other.m_rowPtr[k]; l < other.m_rowPtr[k + 1]; ++l) {
                int col = other.m_colIdx[l];
                double curVal = result->value(i, col);
                result->setValue(i, col, curVal + aVal * other.m_values[l]);
            }
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("multiply"), timer.elapsed());
    return result;
}

SparseMatrixCRS* SparseMatrixCRS::transpose() const
{
    QElapsedTimer timer;
    timer.start();

    SparseMatrixCRS* result = new SparseMatrixCRS(m_cols, m_rows, parent());

    for (int i = 0; i < m_rows; ++i) {
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            result->setValue(m_colIdx[j], i, m_values[j]);
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return result;
}

int SparseMatrixCRS::rows() const { return m_rows; }
int SparseMatrixCRS::cols() const { return m_cols; }
int SparseMatrixCRS::nonZeroCount() const { return m_values.size(); }

double SparseMatrixCRS::density() const
{
    if (m_rows * m_cols == 0) return 0.0;
    return static_cast<double>(m_values.size()) / (m_rows * m_cols);
}

QVector<QVector<double>> SparseMatrixCRS::toDense() const
{
    QVector<QVector<double>> dense(m_rows, QVector<double>(m_cols, 0.0));
    for (int i = 0; i < m_rows; ++i) {
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            dense[i][m_colIdx[j]] = m_values[j];
    }
    return dense;
}

SparseMatrixCRS::Stats SparseMatrixCRS::stats() const { return m_stats; }

void SparseMatrixCRS::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
