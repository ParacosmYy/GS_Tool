/**
 * @file BandMatrix.cpp
 * @brief 带状矩阵实现 — 紧凑存储与高效运算
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/matrix3/BandMatrix.h"

#include <QElapsedTimer>

/** @brief 构造函数
 *  @param size 矩阵维度(方阵)
 *  @param lowerBandwidth 下带宽kl
 *  @param upperBandwidth 上带宽ku
 *  @param parent 父对象 */
BandMatrix::BandMatrix(int size, int lowerBandwidth, int upperBandwidth,
                       QObject *parent)
    : QObject(parent)
    , m_size(qMax(1, size))
    , m_kl(qMax(0, lowerBandwidth))
    , m_ku(qMax(0, upperBandwidth))
    , m_bandwidth(m_kl + m_ku + 1)
    , m_data(static_cast<int>(m_size) * m_bandwidth, 0.0)
{
}

/** @brief 设置矩阵元素值(仅允许带宽范围内设置)
 *  @param row 行索引(0-based)
 *  @param col 列索引(0-based)
 *  @param value 元素值 */
void BandMatrix::setValue(int row, int col, double value)
{
    if (row < 0 || row >= m_size || col < 0 || col >= m_size) return;
    int offset = col - row + m_ku;
    if (offset < 0 || offset >= m_bandwidth) return; /* 带外元素忽略 */
    m_data[row * m_bandwidth + offset] = value;
}

/** @brief 获取矩阵元素值
 *  @param row 行索引(0-based)
 *  @param col 列索引(0-based)
 *  @return 元素值，带外元素返回0.0 */
double BandMatrix::value(int row, int col) const
{
    if (row < 0 || row >= m_size || col < 0 || col >= m_size) return 0.0;
    int offset = col - row + m_ku;
    if (offset < 0 || offset >= m_bandwidth) return 0.0;
    return m_data[row * m_bandwidth + offset];
}

/** @brief 矩阵-向量乘法 y = A*x
 *  @param vec 输入向量(长度=size)
 *  @return 乘积向量 */
QVector<double> BandMatrix::multiply(const QVector<double> &vec) const
{
    if (vec.size() != m_size) return {};

    QVector<double> result(m_size, 0.0);
    for (int i = 0; i < m_size; ++i) {
        double sum = 0.0;
        /* 遍历该行的带宽内列 */
        int jStart = qMax(0, i - m_kl);
        int jEnd = qMin(m_size - 1, i + m_ku);
        for (int j = jStart; j <= jEnd; ++j) {
            sum += value(i, j) * vec[j];
        }
        result[i] = sum;
    }
    return result;
}

/** @brief 求解线性方程组 A*x = b，使用带状高斯消去法(不选主元)
 *  @param rhs 右端项向量(长度=size)
 *  @return 解向量，失败返回空 */
QVector<double> BandMatrix::solve(const QVector<double> &rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (rhs.size() != m_size || m_size == 0) {
        return {};
    }

    /* 创建增广矩阵的副本进行消元 */
    int bw = m_bandwidth;
    QVector<double> ab(m_data); /* 矩阵副本 */
    QVector<double> b(rhs);     /* 右端项副本 */

    /* 前向消元 */
    for (int k = 0; k < m_size; ++k) {
        double diag = ab[k * bw + m_ku]; /* 主对角线元素 */
        if (qFuzzyIsNull(diag)) {
            return {}; /* 奇异矩阵 */
        }

        /* 消去第k列下方带宽范围内的元素 */
        int maxRow = qMin(k + m_kl, m_size - 1);
        for (int i = k + 1; i <= maxRow; ++i) {
            int offset_i = (i - k); /* 行偏移 */
            double factor = ab[i * bw + m_ku - offset_i] / diag;
            if (qFuzzyIsNull(factor)) continue;

            /* 消去行i的第k列 */
            ab[i * bw + m_ku - offset_i] = 0.0;

            /* 更新行i的剩余元素(从k+1到k+ku) */
            int maxCol = qMin(k + m_ku + m_kl, m_size - 1);
            for (int j = k + 1; j <= maxCol; ++j) {
                int colOffset = j - i + m_ku;
                if (colOffset >= 0 && colOffset < bw) {
                    int srcColOffset = j - k + m_ku;
                    if (srcColOffset >= 0 && srcColOffset < bw) {
                        ab[i * bw + colOffset] -= factor * ab[k * bw + srcColOffset];
                    }
                }
            }

            /* 更新右端项 */
            b[i] -= factor * b[k];
        }
    }

    /* 回代 */
    QVector<double> x(m_size);
    x[m_size - 1] = b[m_size - 1] / ab[(m_size - 1) * bw + m_ku];

    for (int i = m_size - 2; i >= 0; --i) {
        double sum = b[i];
        int maxCol = qMin(i + m_ku, m_size - 1);
        for (int j = i + 1; j <= maxCol; ++j) {
            int colOffset = j - i + m_ku;
            if (colOffset >= 0 && colOffset < bw) {
                sum -= ab[i * bw + colOffset] * x[j];
            }
        }
        double diag = ab[i * bw + m_ku];
        if (qFuzzyIsNull(diag)) {
            return {};
        }
        x[i] = sum / diag;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalOperations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit solveCompleted(m_size, elapsed);
    return x;
}

/** @brief 重置统计计数器 */
void BandMatrix::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
