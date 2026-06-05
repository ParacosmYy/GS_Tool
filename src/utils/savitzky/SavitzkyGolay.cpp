/**
 * @file SavitzkyGolay.cpp
 * @brief Savitzky-Golay滤波器实现 — 多项式平滑微分
 */

#include "utils/savitzky/SavitzkyGolay.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SavitzkyGolay::SavitzkyGolay(QObject* parent)
    : QObject(parent)
    , m_windowSize(0)
    , m_timeSum(0.0)
{
}

/** @brief 设计Savitzky-Golay滤波器系数
 *  @param windowSize 窗口大小(必须奇数>=3)
 *  @param polyOrder 多项式阶数(必须<windowSize)
 *  @param derivative 微分阶数
 *  @return 卷积系数 */
QVector<double> SavitzkyGolay::design(int windowSize, int polyOrder,
                                      int derivative)
{
    QElapsedTimer timer;
    timer.start();

    /* 参数校验 */
    if (windowSize < 3) windowSize = 3;
    if (windowSize % 2 == 0) ++windowSize;
    if (polyOrder >= windowSize) polyOrder = windowSize - 1;
    if (polyOrder < 1) polyOrder = 1;
    if (derivative < 0) derivative = 0;
    if (derivative > polyOrder) derivative = polyOrder;

    int halfWin = windowSize / 2;
    m_coeffs = computeCoeffs(halfWin, polyOrder, derivative);
    m_windowSize = windowSize;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDesigns;
    m_timeSum += elapsed;
    double total = static_cast<double>(m_stats.totalDesigns + m_stats.totalApplications);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit designCompleted(windowSize, polyOrder);
    return m_coeffs;
}

/** @brief 应用S-G滤波器
 *  @param signal 输入信号
 *  @return 滤波后信号 */
QVector<double> SavitzkyGolay::apply(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0 || m_coeffs.isEmpty()) { return signal; }

    int halfWin = m_windowSize / 2;
    int cLen = m_coeffs.size();
    QVector<double> result(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < cLen; ++k) {
            int idx = i - halfWin + k;
            /* 边界镜像处理 */
            if (idx < 0) idx = -idx;
            if (idx >= n) idx = 2 * n - 2 - idx;
            idx = qBound(0, idx, n - 1);
            sum += m_coeffs[k] * signal[idx];
        }
        result[i] = sum;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalApplications;
    m_timeSum += elapsed;
    double total = static_cast<double>(m_stats.totalDesigns + m_stats.totalApplications);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit applicationCompleted(n);
    return result;
}

/** @brief 重置统计 */
void SavitzkyGolay::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算卷积系数(最小二乘) */
QVector<double> SavitzkyGolay::computeCoeffs(int halfWin, int polyOrder,
                                              int deriv) const
{
    int m = 2 * halfWin + 1;
    int n = polyOrder + 1;

    /* 构建Vandermonde矩阵 A(m x n) */
    QVector<QVector<double>> A(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        double x = static_cast<double>(i - halfWin);
        A[i][0] = 1.0;
        for (int j = 1; j < n; ++j) {
            A[i][j] = A[i][j - 1] * x;
        }
    }

    /* A^T * A (n x n) */
    QVector<QVector<double>> ATA(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < m; ++k)
                ATA[i][j] += A[k][i] * A[k][j];

    /* Gauss-Jordan求逆 */
    int sz = n;
    QVector<QVector<double>> aug(sz, QVector<double>(2 * sz, 0.0));
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j < sz; ++j) aug[i][j] = ATA[i][j];
        aug[i][sz + i] = 1.0;
    }
    for (int col = 0; col < sz; ++col) {
        int mxR = col;
        for (int r = col + 1; r < sz; ++r)
            if (qAbs(aug[r][col]) > qAbs(aug[mxR][col])) mxR = r;
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

    /* (A^T*A)^{-1} * A^T 的 deriv 行 */
    QVector<double> result(m, 0.0);
    double factorial = 1.0;
    for (int d = 1; d <= deriv; ++d) factorial *= static_cast<double>(d);

    for (int j = 0; j < m; ++j) {
        double val = 0.0;
        for (int k = 0; k < n; ++k) {
            val += aug[deriv][sz + k] * A[j][k];
        }
        result[j] = val * factorial;
    }

    return result;
}
