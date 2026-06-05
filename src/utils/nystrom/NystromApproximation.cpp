/**
 * @file NystromApproximation.cpp
 * @brief Nystrom方法实现 — 大规模核矩阵低秩近似
 *
 * 步骤:
 *   1. 从{0,...,n-1}中均匀采样m个landmark索引
 *   2. 计算子矩阵 K_mm (m x m) 和 K_nm (n x m)
 *   3. 对K_mm做特征值分解: K_mm = U * Lambda * U^T
 *   4. 近似核矩阵 K_approx = K_nm * K_mm^+ * K_nm^T
 *   其中 K_mm^+ 使用正则化伪逆 (加小常数防止奇异)。
 */

#include "utils/nystrom/NystromApproximation.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

NystromApproximation::NystromApproximation(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> NystromApproximation::solveSystem(QVector<QVector<double>> A,
                                                  QVector<double> b)
{
    int n = A.size();
    if (n == 0) return {};

    /* 部分主元Gauss消元 */
    for (int k = 0; k < n; ++k) {
        int pivot = k;
        double maxVal = std::abs(A[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (std::abs(A[i][k]) > maxVal) {
                maxVal = std::abs(A[i][k]);
                pivot = i;
            }
        }
        if (maxVal < 1e-15) return {};

        if (pivot != k) {
            std::swap(A[k], A[pivot]);
            std::swap(b[k], b[pivot]);
        }

        for (int i = k + 1; i < n; ++i) {
            double factor = A[i][k] / A[k][k];
            for (int j = k + 1; j < n; ++j)
                A[i][j] -= factor * A[k][j];
            b[i] -= factor * b[k];
            A[i][k] = 0.0;
        }
    }

    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        for (int j = i + 1; j < n; ++j)
            sum -= A[i][j] * x[j];
        if (std::abs(A[i][i]) < 1e-15) return {};
        x[i] = sum / A[i][i];
    }
    return x;
}

QVector<QVector<double>>
NystromApproximation::approximate(std::function<double(int, int)> kernelFunc,
                                  int n, int numLandmarks)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || numLandmarks <= 0) return {};
    int m = qMin(numLandmarks, n);

    /* 使用Fisher-Yates采样选择m个landmark索引 */
    QVector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 gen(42);
    for (int i = n - 1; i > 0 && i >= n - m; --i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(gen);
        std::swap(indices[i], indices[j]);
    }
    QVector<int> landmarks(m);
    for (int i = 0; i < m; ++i)
        landmarks[i] = indices[n - m + i];

    /* 计算 K_mm (m x m) */
    QVector<QVector<double>> Kmm(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            Kmm[i][j] = kernelFunc(landmarks[i], landmarks[j]);

    /* 计算 K_nm (n x m) */
    QVector<QVector<double>> Knm(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            Knm[i][j] = kernelFunc(i, landmarks[j]);

    /* 正则化 K_mm: 添加小对角扰动防止奇异 */
    const double reg = 1e-8;
    QVector<QVector<double>> KmmReg = Kmm;
    for (int i = 0; i < m; ++i)
        KmmReg[i][i] += reg;

    /* 求K_mm^+的列: 对K_mm的每行求解 K_mm^T * x = e_i */
    /* 等价于 K_mm * X = I => X = K_mm^{-1} */
    QVector<QVector<double>> KmmInv(m, QVector<double>(m, 0.0));
    for (int col = 0; col < m; ++col) {
        QVector<double> e(m, 0.0);
        e[col] = 1.0;
        QVector<double> x = solveSystem(KmmReg, e);
        if (x.isEmpty()) {
            /* 求解失败，使用正则化加强版重试 */
            QVector<QVector<double>> KmmExtra = Kmm;
            for (int i = 0; i < m; ++i) KmmExtra[i][i] += 1e-4;
            x = solveSystem(KmmExtra, e);
            if (x.isEmpty()) x = QVector<double>(m, 0.0);
        }
        for (int i = 0; i < m; ++i)
            KmmInv[i][col] = x[i];
    }

    /* 计算 C = K_nm * K_mm^{-1} */
    QVector<QVector<double>> C(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            for (int k = 0; k < m; ++k)
                C[i][j] += Knm[i][k] * KmmInv[k][j];

    /* 计算 K_approx = C * K_nm^T = K_nm * K_mm^{-1} * K_nm^T */
    QVector<QVector<double>> Kapprox(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < m; ++k)
                Kapprox[i][j] += C[i][k] * Knm[j][k];

    m_stats.totalApproximations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalApproximations;

    emit approximationCompleted(m);
    return Kapprox;
}

void NystromApproximation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
