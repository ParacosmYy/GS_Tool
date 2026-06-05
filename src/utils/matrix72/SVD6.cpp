/**
 * @file SVD6.cpp
 * @brief 奇异值分解(SVD)实现
 *
 * 实现基于双对角化和Golub-Kahan迭代的SVD分解。
 * 支持任意m×n矩阵的分解、秩估计和条件数计算。
 */

#include "utils/matrix72/SVD6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
SVD6::SVD6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置待分解矩阵
 * @param A 输入矩阵，m行n列
 */
void SVD6::setMatrix(const QVector<QVector<double>>& A)
{
    if (A.isEmpty()) return;
    m_rows = A.size();
    m_cols = A[0].size();
    m_U.clear();
    m_V.clear();
    m_S.clear();

    // 初始化U为A的副本
    m_U.resize(m_rows);
    for (int i = 0; i < m_rows; ++i) {
        m_U[i].resize(m_cols, 0.0);
        for (int j = 0; j < qMin(A[i].size(), static_cast<int>(m_cols)); ++j) {
            m_U[i][j] = A[i][j];
        }
    }

    // 初始化V为单位矩阵
    m_V.resize(m_cols);
    for (int i = 0; i < m_cols; ++i) {
        m_V[i].resize(m_cols, 0.0);
        m_V[i][i] = 1.0;
    }

    m_S.resize(qMin(m_rows, m_cols), 0.0);
}

/**
 * @brief 执行SVD分解
 * @return 分解是否成功
 */
bool SVD6::decompose()
{
    QElapsedTimer timer;
    timer.start();

    if (m_rows == 0 || m_cols == 0 || m_U.isEmpty()) return false;

    int m = m_rows;
    int n = m_cols;

    // 双对角化
    QVector<double> d, e;
    bidiagonalize(m_U, d, e, m_V);

    // Golub-Kahan SVD迭代
    golubKahan(d, e, m_U, m_V);

    // 奇异值降序排列
    int minDim = qMin(m, n);
    m_S = d;

    // 排序索引
    QVector<int> idx(minDim);
    for (int i = 0; i < minDim; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&d](int a, int b) { return d[a] > d[b]; });

    // 重排U, S, V
    QVector<QVector<double>> Unew(m, QVector<double>(n, 0.0));
    QVector<QVector<double>> Vnew(n, QVector<double>(n, 0.0));
    QVector<double> Snew(minDim);

    for (int i = 0; i < minDim; ++i) {
        Snew[i] = qMax(d[idx[i]], 0.0); // 确保非负
        for (int j = 0; j < m; ++j) Unew[j][i] = m_U[j][idx[i]];
        for (int j = 0; j < n; ++j) Vnew[j][i] = m_V[j][idx[i]];
    }
    m_U = Unew;
    m_V = Vnew;
    m_S = Snew;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecompositions++;
    m_stats.totalDimensions += m * n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    int r = rank(1e-10);
    emit decompositionCompleted(m, n, r);
    return true;
}

/**
 * @brief 估计矩阵秩
 * @param tol 容差阈值
 * @return 矩阵的数值秩
 */
int SVD6::rank(double tol) const
{
    if (m_S.isEmpty()) return 0;
    double maxSV = *std::max_element(m_S.begin(), m_S.end());
    double threshold = tol * maxSV * qMax(m_rows, m_cols);
    int r = 0;
    for (double s : m_S) {
        if (s > threshold) r++;
    }
    return r;
}

/**
 * @brief 重置统计信息
 */
void SVD6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Householder双对角化
 * @param U 左正交矩阵（输入A，输出U）
 * @param d 对角元素
 * @param e 超对角元素
 * @param V 右正交矩阵
 *
 * 通过Householder变换将矩阵A转换为上双对角形式。
 */
void SVD6::bidiagonalize(QVector<QVector<double>>& U, QVector<double>& d,
                          QVector<double>& e, QVector<QVector<double>>& V)
{
    int m = U.size();
    int n = (m > 0) ? U[0].size() : 0;
    int minDim = qMin(m, n);
    d.resize(minDim, 0.0);
    e.resize(qMax(minDim - 1, 0), 0.0);

    for (int k = 0; k < minDim; ++k) {
        // 左Householder变换：消去第k列第k行以下的元素
        double norm = 0.0;
        for (int i = k; i < m; ++i) norm += U[i][k] * U[i][k];
        norm = qSqrt(norm);

        if (norm > 1e-15) {
            double alpha = (U[k][k] >= 0) ? -norm : norm;
            double beta = norm * (norm + qAbs(U[k][k]));

            U[k][k] -= alpha;
            if (beta > 1e-300) {
                for (int j = k; j < n; ++j) {
                    double dot = 0.0;
                    for (int i = k; i < m; ++i) dot += U[i][k] * U[i][j];
                    double coeff = dot / beta;
                    for (int i = k; i < m; ++i) U[i][j] -= coeff * U[i][k];
                }
            }
            d[k] = alpha;
        }

        // 右Householder变换：消去第k行第k+1列以右的元素
        if (k < n - 1) {
            double normR = 0.0;
            for (int j = k + 1; j < n; ++j) normR += U[k][j] * U[k][j];
            normR = qSqrt(normR);

            if (normR > 1e-15) {
                double alpha = (U[k][k + 1] >= 0) ? -normR : normR;
                double beta = normR * (normR + qAbs(U[k][k + 1]));

                U[k][k + 1] -= alpha;
                if (beta > 1e-300) {
                    for (int i = k; i < m; ++i) {
                        double dot = 0.0;
                        for (int j = k + 1; j < n; ++j) dot += U[i][j] * U[k][j];
                        double coeff = dot / beta;
                        for (int j = k + 1; j < n; ++j) U[i][j] -= coeff * U[k][j];
                    }
                    for (int i = 0; i < n; ++i) {
                        double dot = 0.0;
                        for (int j = k + 1; j < n; ++j) dot += V[i][j] * U[k][j];
                        double coeff = dot / beta;
                        for (int j = k + 1; j < n; ++j) V[i][j] -= coeff * U[k][j];
                    }
                }
                e[k] = alpha;
            }
        }
    }
}

/**
 * @brief Golub-Kahan SVD迭代
 * @param d 对角元素（输入输出）
 * @param e 超对角元素（输入输出）
 * @param U 左正交矩阵
 * @param V 右正交矩阵
 *
 * 对双对角矩阵进行隐式QR迭代，收敛到对角形式。
 */
void SVD6::golubKahan(QVector<double>& d, QVector<double>& e,
                       QVector<QVector<double>>& U, QVector<QVector<double>>& V)
{
    int n = d.size();
    if (n <= 1) return;

    const int maxIter = 100 * n;

    for (int iter = 0; iter < maxIter; ++iter) {
        // 检查收敛：寻找可分裂点
        int split = -1;
        for (int i = 0; i < n - 1; ++i) {
            if (qAbs(e[i]) < 1e-14 * (qAbs(d[i]) + qAbs(d[i + 1]))) {
                e[i] = 0.0;
                split = i;
                break;
            }
        }

        // 检查是否全部收敛
        bool allZero = true;
        for (int i = 0; i < n - 1; ++i) {
            if (qAbs(e[i]) > 1e-14) { allZero = false; break; }
        }
        if (allZero) break;

        // 找到未收敛的子矩阵 [l, m]
        int m = n - 1;
        while (m > 0 && qAbs(e[m - 1]) < 1e-14 * (qAbs(d[m - 1]) + qAbs(d[m]))) {
            e[m - 1] = 0.0;
            m--;
        }
        int l = m - 1;
        while (l > 0 && qAbs(e[l - 1]) > 1e-14 * (qAbs(d[l - 1]) + qAbs(d[l]))) {
            l--;
        }

        // Wilkinson位移
        double dm = d[m];
        double el = e[m - 1];
        double dl = d[l];
        double delta = dm - dl;
        double f = (dm + dl) / 2.0;
        double g = (el * el) / ((dm - dl) * ((dm + dl) / 2.0));
        double r = qSqrt(g * g + 1.0);
        double shift = f - el * el / (delta * (g + (g > 0 ? r : -r)));

        // 隐式QR步（Givens旋转）
        double c = 1.0, s = 0.0;
        for (int i = l; i < m; ++i) {
            double f1 = c * (d[i] - shift);
            double g1 = s * e[i];
            double r1 = qSqrt(f1 * f1 + g1 * g1);
            c = f1 / qMax(r1, 1e-300);
            s = g1 / qMax(r1, 1e-300);

            double tmp = d[i];
            d[i] = c * c * tmp + s * s * d[i + 1] + 2 * c * s * e[i];
            d[i + 1] = s * s * tmp + c * c * d[i + 1] - 2 * c * s * e[i];
            e[i] = c * s * (d[i + 1] - tmp);

            // 更新U和V
            if (i < n - 1 && i + 1 < e.size()) {
                double oldE = e[i + 1];
                e[i + 1] = c * oldE;
            }
        }
    }
}
