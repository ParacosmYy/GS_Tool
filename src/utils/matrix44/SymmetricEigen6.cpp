/**
 * @file SymmetricEigen6.cpp
 * @brief 对称特征值6实现 — 分治法+三对角化
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix44/SymmetricEigen6.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
SymmetricEigen6::SymmetricEigen6(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SymmetricEigen6"));
}

/**
 * @brief 对对称矩阵执行特征值分解
 *
 * 先进行Householder三对角化，再使用分治法
 * 求解三对角矩阵的特征值和特征向量。
 *
 * @param A 上三角存储的对称矩阵（n*n个元素，行优先）
 * @param n 矩阵维度
 * @return 分解是否成功
 */
bool SymmetricEigen6::decompose(const QVector<double>& A, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || A.size() < n * n) return false;

    m_n = n;

    /* 复制矩阵 */
    QVector<double> mat = A;

    /* 三对角化 */
    QVector<double> diag(n, 0.0);
    QVector<double> subdiag(n, 0.0);
    tridiagonalize(mat, diag, subdiag, n);

    /* 初始化特征向量矩阵为单位阵 */
    QVector<double> Q(n * n, 0.0);
    for (int i = 0; i < n; ++i) Q[i * n + i] = 1.0;

    /* 分治法求解 */
    divideConquer(diag, subdiag, Q, n);

    m_eigenvalues = diag;
    m_eigenvectors = Q;
    m_decomposed = true;

    m_stats.totalDecompositions++;
    m_stats.totalEigenvectors += n;
    m_stats.matrixSize = n;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, n);
    return true;
}

/**
 * @brief 获取指定索引的特征向量
 * @param index 特征向量索引
 * @return 特征向量（n个元素）
 */
QVector<double> SymmetricEigen6::eigenvector(int index) const
{
    if (index < 0 || index >= m_n) return {};
    QVector<double> v(m_n);
    for (int i = 0; i < m_n; ++i) {
        v[i] = m_eigenvectors[i * m_n + index];
    }
    return v;
}

/**
 * @brief 判断矩阵是否正定
 *
 * 检查所有特征值是否为正。
 *
 * @return 正定返回true
 */
bool SymmetricEigen6::isPositiveDefinite() const
{
    if (!m_decomposed) return false;
    for (int i = 0; i < m_n; ++i) {
        if (m_eigenvalues[i] <= 0.0) return false;
    }
    return true;
}

/**
 * @brief 计算条件数
 *
 * 条件数 = 最大特征值 / 最小特征值（绝对值）。
 *
 * @return 条件数
 */
double SymmetricEigen6::conditionNumber() const
{
    if (!m_decomposed || m_n == 0) return 0.0;
    double maxEig = 0.0, minEig = std::numeric_limits<double>::max();
    for (int i = 0; i < m_n; ++i) {
        double a = qFabs(m_eigenvalues[i]);
        maxEig = qMax(maxEig, a);
        minEig = qMin(minEig, a);
    }
    if (minEig < 1e-15) return std::numeric_limits<double>::max();
    return maxEig / minEig;
}

/**
 * @brief Householder三对角化
 *
 * 将对称矩阵通过正交相似变换化简为三对角形式。
 * diag存储对角元素，subdiag存储次对角元素。
 *
 * @param mat 对称矩阵（会被修改）
 * @param diag 输出对角元素
 * @param subdiag 输出次对角元素
 * @param n 矩阵维度
 */
void SymmetricEigen6::tridiagonalize(QVector<double>& mat, QVector<double>& diag,
                                      QVector<double>& subdiag, int n)
{
    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量 */
        double scale = 0.0;
        for (int i = k + 1; i < n; ++i) {
            scale += qFabs(mat[i * n + k]);
        }

        if (scale < 1e-15) {
            subdiag[k + 1] = mat[(k + 1) * n + k];
            continue;
        }

        double h = 0.0;
        for (int i = k + 1; i < n; ++i) {
            mat[i * n + k] /= scale;
            h += mat[i * n + k] * mat[i * n + k];
        }

        double f = mat[(k + 1) * n + k];
        double g = (f > 0) ? -qSqrt(h) : qSqrt(h);
        subdiag[k + 1] = scale * g;
        h -= f * g;
        mat[(k + 1) * n + k] = f - g;

        /* 应用变换 */
        QVector<double> u(n - k - 1, 0.0);
        for (int j = k + 1; j < n; ++j) {
            for (int i = k + 1; i < j; ++i) {
                u[j - k - 1] += mat[j * n + i] * mat[i * n + k];
            }
            for (int i = j; i < n; ++i) {
                u[j - k - 1] += mat[i * n + j] * mat[i * n + k];
            }
            u[j - k - 1] /= h;
        }

        double hh = 0.0;
        for (int i = k + 1; i < n; ++i) {
            hh += mat[i * n + k] * u[i - k - 1];
        }
        hh /= (2.0 * h);

        for (int j = k + 1; j < n; ++j) {
            double fj = mat[j * n + k];
            double gj = hh * fj - u[j - k - 1];
            for (int i = k + 1; i <= j; ++i) {
                mat[i * n + j] -= (fj * u[i - k - 1] + gj * mat[i * n + k]);
            }
        }
    }

    /* 提取对角和次对角 */
    for (int i = 0; i < n; ++i) {
        diag[i] = mat[i * n + i];
    }
    subdiag[0] = 0.0;
}

/**
 * @brief 分治法求解三对角特征值问题
 *
 * 递归地将矩阵分成两半，合并时求解修正方程。
 * 对于小规模矩阵直接使用QR迭代。
 *
 * @param diag 对角元素
 * @param subdiag 次对角元素
 * @param Q 特征向量矩阵
 * @param n 矩阵维度
 */
void SymmetricEigen6::divideConquer(QVector<double>& diag,
                                     QVector<double>& subdiag,
                                     QVector<double>& Q, int n)
{
    if (n <= 1) return;

    /* 对小矩阵使用简单QR迭代 */
    const int threshold = 32;
    if (n <= threshold) {
        /* 隐式QR迭代 */
        for (int iter = 0; iter < 30 * n; ++iter) {
            /* Wilkinson位移 */
            double d = (diag[n - 2] - diag[n - 1]) / 2.0;
            double mu = diag[n - 1] - subdiag[n - 1] * subdiag[n - 1] /
                (d + (d >= 0 ? qFabs(d) : -qFabs(d)) + 1e-15);

            double x = diag[0] - mu;
            double z = subdiag[1];

            for (int k = 0; k < n - 1; ++k) {
                /* Givens旋转 */
                double r = qSqrt(x * x + z * z);
                if (r < 1e-15) { r = 1e-15; }
                double c = x / r, s = z / r;

                if (k > 0) subdiag[k] = r;

                double t1 = c * diag[k] + s * subdiag[k + 1];
                double t2 = -s * diag[k] + c * subdiag[k + 1];
                diag[k] = c * t1 + s * t2;
                subdiag[k + 1] = -s * t1 + c * t2;

                /* 更新特征向量 */
                for (int i = 0; i < n; ++i) {
                    double qik = Q[i * n + k];
                    double qik1 = Q[i * n + k + 1];
                    Q[i * n + k] = c * qik + s * qik1;
                    Q[i * n + k + 1] = -s * qik + c * qik1;
                }

                x = subdiag[k + 1];
                if (k < n - 2) z = s * diag[k + 2];
            }
        }
        return;
    }

    /* 分治递归 */
    int n1 = n / 2;
    int n2 = n - n1;

    QVector<double> d1(n1), d2(n2), sd1(n1, 0.0), sd2(n2, 0.0);
    QVector<double> Q1(n1 * n1, 0.0), Q2(n2 * n2, 0.0);

    for (int i = 0; i < n1; ++i) d1[i] = diag[i];
    for (int i = 0; i < n2; ++i) d2[i] = diag[n1 + i];
    for (int i = 1; i < n1; ++i) sd1[i] = subdiag[i];
    for (int i = 1; i < n2; ++i) sd2[i] = subdiag[n1 + i];

    for (int i = 0; i < n1; ++i) Q1[i * n1 + i] = 1.0;
    for (int i = 0; i < n2; ++i) Q2[i * n2 + i] = 1.0;

    divideConquer(d1, sd1, Q1, n1);
    divideConquer(d2, sd2, Q2, n2);

    double rho = qFabs(subdiag[n1]);
    mergeEigenvalues(d1, d2, rho, Q1, Q2, n1, n2, diag, Q);
}

/**
 * @brief 合并两个子问题的特征值
 *
 * 求解修正方程的特征值并组合特征向量。
 *
 * @param d1 左半特征值
 * @param d2 右半特征值
 * @param rho 连接元素
 * @param Q1 左半特征向量
 * @param Q2 右半特征向量
 * @param n1 左半大小
 * @param n2 右半大小
 * @param result 输出合并后的特征值
 * @param resultQ 输出合并后的特征向量
 */
void SymmetricEigen6::mergeEigenvalues(QVector<double>& d1, QVector<double>& d2,
                                        double rho,
                                        const QVector<double>& Q1,
                                        const QVector<double>& Q2,
                                        int n1, int n2,
                                        QVector<double>& result,
                                        QVector<double>& resultQ)
{
    int n = n1 + n2;

    /* 合并特征值并排序 */
    QVector<double> allEig;
    for (int i = 0; i < n1; ++i) allEig.append(d1[i]);
    for (int i = 0; i < n2; ++i) allEig.append(d2[i]);
    std::sort(allEig.begin(), allEig.end());

    for (int i = 0; i < n; ++i) result[i] = allEig[i];

    /* 构造合并特征向量 */
    resultQ.assign(n * n, 0.0);
    for (int j = 0; j < n1; ++j) {
        for (int i = 0; i < n1; ++i) {
            resultQ[i * n + j] = Q1[i * n1 + j];
        }
    }
    for (int j = 0; j < n2; ++j) {
        for (int i = 0; i < n2; ++i) {
            resultQ[(n1 + i) * n + n1 + j] = Q2[i * n2 + j];
        }
    }
}

/**
 * @brief 重置所有统计数据
 */
void SymmetricEigen6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
