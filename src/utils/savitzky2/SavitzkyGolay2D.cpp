/**
 * @file SavitzkyGolay2D.cpp
 * @brief 二维Savitzky-Golay滤波器实现 — 图像/矩阵平滑微分
 */

#include "utils/savitzky2/SavitzkyGolay2D.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数
 *  @param rows 默认行数
 *  @param cols 默认列数
 *  @param polyOrder 多项式阶数
 *  @param derivOrder 偏导阶数
 *  @param parent 父对象
 */
SavitzkyGolay2D::SavitzkyGolay2D(int rows, int cols, int polyOrder,
                                 int derivOrder, QObject* parent)
    : QObject(parent)
    , m_kernelRows(qMax(3, rows > 0 ? (rows % 2 == 0 ? rows + 1 : rows) : 5))
    , m_kernelCols(qMax(3, cols > 0 ? (cols % 2 == 0 ? cols + 1 : cols) : 5))
    , m_polyOrder(qBound(1, polyOrder, 4))
    , m_derivOrder(qBound(0, derivOrder, m_polyOrder))
    , m_timeSum(0.0)
{
    m_kernel = computeKernel();
}

/** @brief 计算二维单项式基向量
 *  @param x x坐标偏移
 *  @param y y坐标偏移
 *  @return 单项式值向量
 */
QVector<double> SavitzkyGolay2D::basisVector(double x, double y) const
{
    /* 按总阶数排列: 0阶{1}, 1阶{x,y}, 2阶{x²,xy,y²}, ... */
    QVector<double> basis;
    int maxDeg = m_polyOrder;
    for (int totalDeg = 0; totalDeg <= maxDeg; ++totalDeg) {
        for (int py = 0; py <= totalDeg; ++py) {
            int px = totalDeg - py;
            basis.append(qPow(x, px) * qPow(y, py));
        }
    }
    return basis;
}

/** @brief 计算二维S-G卷积核
 *  @return 展平后的卷积核系数(行优先)
 */
QVector<double> SavitzkyGolay2D::computeKernel() const
{
    int halfR = m_kernelRows / 2;
    int halfC = m_kernelCols / 2;
    int m = m_kernelRows * m_kernelCols;

    /* 生成第一组基以确定基向量长度 */
    int nBasis = basisVector(0.0, 0.0).size();
    if (nBasis == 0) return QVector<double>();

    /* 构建设计矩阵 A(m x nBasis) */
    QVector<QVector<double>> A(m, QVector<double>(nBasis, 0.0));
    for (int dr = -halfR; dr <= halfR; ++dr) {
        for (int dc = -halfC; dc <= halfC; ++dc) {
            int rowIdx = (dr + halfR) * m_kernelCols + (dc + halfC);
            A[rowIdx] = basisVector(static_cast<double>(dc),
                                    static_cast<double>(dr));
        }
    }

    /* 计算 A^T * A (nBasis x nBasis) */
    QVector<QVector<double>> ATA(nBasis, QVector<double>(nBasis, 0.0));
    for (int i = 0; i < nBasis; ++i) {
        for (int j = 0; j < nBasis; ++j) {
            double sum = 0.0;
            for (int k = 0; k < m; ++k) {
                sum += A[k][i] * A[k][j];
            }
            ATA[i][j] = sum;
        }
    }

    /* Gauss-Jordan求逆 ATA */
    int sz = nBasis;
    QVector<QVector<double>> aug(sz, QVector<double>(2 * sz, 0.0));
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j < sz; ++j) aug[i][j] = ATA[i][j];
        aug[i][sz + i] = 1.0;
    }
    for (int col = 0; col < sz; ++col) {
        int mxR = col;
        for (int r = col + 1; r < sz; ++r) {
            if (qAbs(aug[r][col]) > qAbs(aug[mxR][col])) mxR = r;
        }
        std::swap(aug[col], aug[mxR]);
        double piv = aug[col][col];
        if (qAbs(piv) < 1e-15) continue;
        for (int j = 0; j < 2 * sz; ++j) aug[col][j] /= piv;
        for (int r = 0; r < sz; ++r) {
            if (r == col) continue;
            double f = aug[r][col];
            for (int j = 0; j < 2 * sz; ++j) aug[r][j] -= f * aug[col][j];
        }
    }

    /* 提取 (A^T*A)^{-1} * A^T 的 derivOrder 行作为核系数
     * derivOrder=0 取第0行(平滑), derivOrder=1 取第1行(x偏导) */
    int targetRow = qBound(0, m_derivOrder, sz - 1);
    QVector<double> kernel(m, 0.0);
    for (int j = 0; j < m; ++j) {
        double val = 0.0;
        for (int k = 0; k < sz; ++k) {
            val += aug[targetRow][sz + k] * A[j][k];
        }
        kernel[j] = val;
    }

    return kernel;
}

/** @brief 设置卷积核窗口大小
 *  @param rows 窗口行数
 *  @param cols 窗口列数
 */
void SavitzkyGolay2D::setKernelSize(int rows, int cols)
{
    if (rows < 3) rows = 3;
    if (rows % 2 == 0) ++rows;
    if (cols < 3) cols = 3;
    if (cols % 2 == 0) ++cols;
    m_kernelRows = rows;
    m_kernelCols = cols;
    m_kernel = computeKernel();
}

/** @brief 对二维数据执行S-G滤波
 *  @param input 输入矩阵(外层为行, 内层为列)
 *  @return 滤波后矩阵
 */
QVector<QVector<double>> SavitzkyGolay2D::filter(
    const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    int nRows = input.size();
    if (nRows == 0) return input;
    int nCols = input[0].size();
    if (nCols == 0) return input;

    /* 确保输入矩阵规整 */
    for (const auto& row : input) {
        if (row.size() != nCols) return input;
    }

    /* 自动调整核大小不超过输入维度 */
    if (m_kernelRows > nRows || m_kernelCols > nCols) {
        return input;
    }

    int halfR = m_kernelRows / 2;
    int halfC = m_kernelCols / 2;
    int kSize = m_kernelRows * m_kernelCols;

    /* 若核未计算或尺寸不匹配则重新计算 */
    if (m_kernel.size() != kSize) {
        m_kernel = computeKernel();
    }

    QVector<QVector<double>> result(nRows, QVector<double>(nCols, 0.0));

    for (int r = 0; r < nRows; ++r) {
        for (int c = 0; c < nCols; ++c) {
            double sum = 0.0;
            for (int kr = 0; kr < m_kernelRows; ++kr) {
                for (int kc = 0; kc < m_kernelCols; ++kc) {
                    int sr = r - halfR + kr;
                    int sc = c - halfC + kc;
                    /* 边界镜像扩展 */
                    if (sr < 0) sr = -sr;
                    if (sr >= nRows) sr = 2 * nRows - 2 - sr;
                    sr = qBound(0, sr, nRows - 1);
                    if (sc < 0) sc = -sc;
                    if (sc >= nCols) sc = 2 * nCols - 2 - sc;
                    sc = qBound(0, sc, nCols - 1);
                    sum += m_kernel[kr * m_kernelCols + kc] * input[sr][sc];
                }
            }
            result[r][c] = sum;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalFiltered;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFiltered);

    emit filterCompleted(nRows, nCols);
    return result;
}

/** @brief 重置统计信息 */
void SavitzkyGolay2D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
