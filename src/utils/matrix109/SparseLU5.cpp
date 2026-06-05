#include "SparseLU5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化稀疏LU分解求解器
 * @param parent 父对象指针
 */
SparseLU5::SparseLU5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SparseLU5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行稀疏LU分解 (带部分主元选取)
 *
 * 将稀疏矩阵A分解为PA=LU形式，L为单位下三角矩阵，U为上三角矩阵，
 * P为行排列矩阵。通过部分主元选取保证数值稳定性。
 * 保持稀疏结构，仅存储非零元素。
 *
 * @param triplets 稀疏矩阵的三元组表示(行,列,值)
 * @param n 矩阵维度
 * @return 是否分解成功
 */
bool SparseLU5::factorize(const QVector<Triplet>& triplets, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0) {
        emit factorizationCompleted(0);
        return false;
    }

    m_n = n;

    /* 将三元组转换为稠密矩阵进行分解（简化实现） */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (const auto& t : triplets) {
        if (t.row >= 0 && t.row < n && t.col >= 0 && t.col < n) {
            A[t.row][t.col] = t.value;
        }
    }

    /* 初始化排列向量和行列式 */
    m_permutation.resize(n);
    for (int i = 0; i < n; ++i) m_permutation[i] = i;
    m_determinant = 1.0;

    /* 初始化L和U的稠密表示 */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> U = A;

    /* 带部分主元选取的LU分解 */
    for (int k = 0; k < n; ++k) {
        /* 查找主元 */
        int maxRow = k;
        double maxVal = qAbs(U[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(U[i][k]) > maxVal) {
                maxVal = qAbs(U[i][k]);
                maxRow = i;
            }
        }

        if (maxVal < 1e-15) {
            /* 奇异矩阵 */
            m_determinant = 0.0;
            emit factorizationCompleted(n);
            return false;
        }

        /* 交换行 */
        if (maxRow != k) {
            std::swap(U[k], U[maxRow]);
            std::swap(L[k], L[maxRow]);
            std::swap(m_permutation[k], m_permutation[maxRow]);
            m_determinant = -m_determinant;
        }

        L[k][k] = 1.0;
        m_determinant *= U[k][k];

        /* 消元 */
        for (int i = k + 1; i < n; ++i) {
            double factor = U[i][k] / U[k][k];
            L[i][k] = factor;
            for (int j = k; j < n; ++j) {
                U[i][j] -= factor * U[k][j];
            }
        }
    }

    /* 提取稀疏结构 */
    m_lEntries.clear();
    m_uEntries.clear();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i > j && qAbs(L[i][j]) > 1e-15) {
                m_lEntries.append({i, j, L[i][j]});
            }
            if (j >= i && qAbs(U[i][j]) > 1e-15) {
                m_uEntries.append({i, j, U[i][j]});
            }
        }
    }

    m_stats.nnzL = m_lEntries.size();
    m_stats.nnzU = m_uEntries.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFactorizations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFactorizations;

    emit factorizationCompleted(n);
    return true;
}

/**
 * @brief 求解线性方程组 Ax=b
 *
 * 利用已有的LU分解，通过前代和回代求解：Ly=Pb, Ux=y。
 *
 * @param rhs 右端向量b
 * @return 解向量x
 */
QVector<double> SparseLU5::solve(const QVector<double>& rhs) const
{
    if (m_n == 0 || rhs.size() != m_n) return {};

    /* 重建稠密L和U（简化） */
    QVector<QVector<double>> L(m_n, QVector<double>(m_n, 0.0));
    QVector<QVector<double>> U(m_n, QVector<double>(m_n, 0.0));
    for (int i = 0; i < m_n; ++i) { L[i][i] = 1.0; }
    for (const auto& e : m_lEntries) L[e.row][e.col] = e.value;
    for (const auto& e : m_uEntries) U[e.row][e.col] = e.value;

    /* 应用排列 Pb */
    QVector<double> pb(m_n);
    for (int i = 0; i < m_n; ++i) {
        pb[i] = rhs[m_permutation[i]];
    }

    /* 前代 Ly = Pb */
    QVector<double> y(m_n);
    for (int i = 0; i < m_n; ++i) {
        y[i] = pb[i];
        for (int j = 0; j < i; ++j) {
            y[i] -= L[i][j] * y[j];
        }
    }

    /* 回代 Ux = y */
    QVector<double> x(m_n);
    for (int i = m_n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < m_n; ++j) {
            x[i] -= U[i][j] * x[j];
        }
        x[i] /= U[i][i];
    }

    return x;
}

/**
 * @brief 获取分解的填充比
 * @return 填充比(nnz(L+U)/nnz(A))
 */
double SparseLU5::fillRatio() const
{
    int origNnz = m_lEntries.size() + m_uEntries.size();
    if (origNnz == 0) return 0.0;
    return static_cast<double>(m_stats.nnzL + m_stats.nnzU) / origNnz;
}
