/**
 * @file SymmetricEigen9.cpp
 * @brief 对称矩阵特征值分解实现
 *
 * 实现基于三对角化和隐式QR迭代的对称矩阵特征值分解，
 * 所有特征值为实数，特征向量正交。
 */

#include "utils/matrix76/SymmetricEigen9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
SymmetricEigen9::SymmetricEigen9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置待分解矩阵
 * @param A 对称矩阵
 */
void SymmetricEigen9::setMatrix(const QVector<QVector<double>>& A)
{
    if (A.isEmpty()) return;
    m_n = A.size();
    m_A = A;
    // 确保对称
    for (int i = 0; i < m_n; ++i) {
        m_A[i].resize(m_n, 0.0);
        for (int j = i + 1; j < m_n; ++j) {
            double avg = (m_A[i][j] + m_A[j][i]) / 2.0;
            m_A[i][j] = avg;
            m_A[j][i] = avg;
        }
    }
}

/**
 * @brief 执行特征值分解
 * @return 分解是否成功
 */
bool SymmetricEigen9::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || m_A.isEmpty()) return false;

    // 初始化特征向量为单位矩阵
    m_eigvecs.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_eigvecs[i].resize(m_n, 0.0);
        m_eigvecs[i][i] = 1.0;
    }

    // 步骤1：三对角化（Householder变换）
    tridiagonalize();

    // 步骤2：QR迭代求特征值
    qrIteration();

    // 特征值排序（降序）
    QVector<int> idx(m_n);
    for (int i = 0; i < m_n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [this](int a, int b) {
        return m_eigenvalues[a] > m_eigenvalues[b];
    });

    QVector<double> sortedVals(m_n);
    QVector<QVector<double>> sortedVecs(m_n, QVector<double>(m_n));
    for (int i = 0; i < m_n; ++i) {
        sortedVals[i] = m_eigenvalues[idx[i]];
        for (int j = 0; j < m_n; ++j) {
            sortedVecs[j][i] = m_eigvecs[j][idx[i]];
        }
    }
    m_eigenvalues = sortedVals;
    m_eigvecs = sortedVecs;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecompositions++;
    m_stats.totalDimensions += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    int posCount = 0;
    for (double v : m_eigenvalues) { if (v > 0) posCount++; }
    emit decompositionCompleted(m_n, posCount);
    return true;
}

/**
 * @brief 估计矩阵的数值秩
 * @param tol 容差
 * @return 秩
 */
int SymmetricEigen9::rank(double tol) const
{
    if (m_eigenvalues.isEmpty()) return 0;
    double maxEV = *std::max_element(m_eigenvalues.begin(), m_eigenvalues.end());
    double threshold = tol * qMax(maxEV, 1.0);
    int r = 0;
    for (double v : m_eigenvalues) {
        if (qAbs(v) > threshold) r++;
    }
    return r;
}

/**
 * @brief 重置统计信息
 */
void SymmetricEigen9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Householder三对角化
 *
 * 通过Householder变换将对称矩阵A转化为三对角形式 T = Q^T*A*Q。
 * 对角线元素存入m_eigenvalues初始值，超对角线存入临时数组。
 */
void SymmetricEigen9::tridiagonalize()
{
    int n = m_n;
    m_eigenvalues.resize(n, 0.0);

    QVector<double> e(n, 0.0); // 超对角线

    for (int k = 0; k < n - 2; ++k) {
        // 提取第k列的k+2到n-1元素
        double norm = 0.0;
        for (int i = k + 2; i < n; ++i) {
            norm += m_A[i][k] * m_A[i][k];
        }
        norm = qSqrt(norm + m_A[k + 1][k] * m_A[k + 1][k]);

        if (norm < 1e-15) continue;

        double alpha = (m_A[k + 1][k] >= 0) ? -norm : norm;
        double beta = norm * (norm + qAbs(m_A[k + 1][k]));

        m_A[k + 1][k] -= alpha;
        m_A[k][k + 1] = m_A[k + 1][k];

        // P = I - 2vv^T/beta, A = P*A*P
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += m_A[j][k] * m_A[j][i];
            double coeff = dot / beta;
            for (int j = k + 1; j < n; ++j) m_A[j][i] -= coeff * m_A[j][k];
        }
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += m_A[j][k] * m_A[i][j];
            double coeff = dot / beta;
            for (int j = k + 1; j < n; ++j) m_A[i][j] -= coeff * m_A[j][k];
        }

        // 更新特征向量矩阵
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += m_A[j][k] * m_eigvecs[i][j];
            double coeff = dot / beta;
            for (int j = k + 1; j < n; ++j) m_eigvecs[i][j] -= coeff * m_A[j][k];
        }

        m_A[k + 1][k] = alpha;
        m_A[k][k + 1] = alpha;
    }

    // 提取对角线和超对角线
    for (int i = 0; i < n; ++i) {
        m_eigenvalues[i] = m_A[i][i];
    }
}

/**
 * @brief 隐式QR迭代
 *
 * 对三对角矩阵执行QR迭代，带Wilkinson位移，
 * 直到所有超对角线元素收敛到零。
 */
void SymmetricEigen9::qrIteration()
{
    int n = m_n;
    if (n <= 1) return;

    // 提取超对角线
    QVector<double> e(n, 0.0);
    for (int i = 0; i < n - 1; ++i) {
        e[i] = m_A[i][i + 1];
    }

    int maxIter = 30 * n;
    int m = n - 1;

    for (int iter = 0; iter < maxIter && m > 0; ++iter) {
        // 检查e[m-1]是否收敛
        for (; m > 0; --m) {
            if (qAbs(e[m - 1]) <= 1e-14 * (qAbs(m_eigenvalues[m - 1]) + qAbs(m_eigenvalues[m]))) {
                e[m - 1] = 0.0;
            } else {
                break;
            }
        }
        if (m == 0) break;

        // 找到子矩阵的起始
        int l = m - 1;
        for (; l > 0; --l) {
            if (qAbs(e[l - 1]) <= 1e-14 * (qAbs(m_eigenvalues[l - 1]) + qAbs(m_eigenvalues[l]))) {
                e[l - 1] = 0.0;
                break;
            }
        }

        // Wilkinson位移
        double d = (m_eigenvalues[m - 1] - m_eigenvalues[m]) / 2.0;
        double shift = m_eigenvalues[m] - e[m - 1] * e[m - 1] /
                       (d + (d >= 0 ? 1 : -1) * qSqrt(d * d + e[m - 1] * e[m - 1]));

        // Givens旋转消去超对角线
        double c = 1.0, s = 0.0;
        for (int i = l; i < m; ++i) {
            double f = c * (m_eigenvalues[i] - shift) + s * e[i];
            double g = c * e[i];
            double r = qSqrt(f * f + g * g);
            c = f / qMax(r, 1e-300);
            s = g / qMax(r, 1e-300);

            double tmp = m_eigenvalues[i];
            m_eigenvalues[i] = c * c * tmp + s * s * m_eigenvalues[i + 1] + 2 * c * s * e[i];
            m_eigenvalues[i + 1] = s * s * tmp + c * c * m_eigenvalues[i + 1] - 2 * c * s * e[i];

            if (i < m - 1) {
                e[i] = c * e[i + 1];
            }

            // 更新特征向量
            for (int j = 0; j < n; ++j) {
                double v1 = m_eigvecs[j][i];
                double v2 = m_eigvecs[j][i + 1];
                m_eigvecs[j][i] = c * v1 + s * v2;
                m_eigvecs[j][i + 1] = -s * v1 + c * v2;
            }
        }
    }
}
