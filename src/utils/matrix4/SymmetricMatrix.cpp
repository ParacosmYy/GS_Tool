/**
 * @file SymmetricMatrix.cpp
 * @brief 对称矩阵实现 — 紧凑存储+Jacobi特征值分解
 */

#include "utils/matrix4/SymmetricMatrix.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param size 矩阵维度 @param parent 父对象 */
SymmetricMatrix::SymmetricMatrix(int size, QObject* parent)
    : QObject(parent)
    , m_size(qMax(0, size))
    , m_timeSum(0.0)
{
    /* 下三角元素数 = n*(n+1)/2 */
    int count = m_size * (m_size + 1) / 2;
    m_data.resize(count);
    m_data.fill(0.0);
}

/** @brief 设置矩阵元素值 @param i 行索引 @param j 列索引 @param value 元素值 */
void SymmetricMatrix::setValue(int i, int j, double value)
{
    if (i < 0 || i >= m_size || j < 0 || j >= m_size) return;
    m_data[index(i, j)] = value;
}

/** @brief 获取矩阵元素值 @param i 行索引 @param j 列索引 @return 元素值 */
double SymmetricMatrix::value(int i, int j) const
{
    if (i < 0 || i >= m_size || j < 0 || j >= m_size) return 0.0;
    return m_data[index(i, j)];
}

/** @brief 矩阵-向量乘法 y = A * vec @param vec 输入向量 @return 乘积向量 */
QVector<double> SymmetricMatrix::multiply(const QVector<double>& vec) const
{
    m_timer.start();

    QVector<double> result;
    if (vec.size() != m_size) {
        return result;
    }

    result.resize(m_size);
    result.fill(0.0);

    for (int i = 0; i < m_size; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m_size; ++j) {
            sum += m_data[index(i, j)] * vec[j];
        }
        result[i] = sum;
    }

    ++m_stats.totalOperations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("Multiply"));
    return result;
}

/** @brief 计算全部特征值(Jacobi旋转法) @return 特征值列表(降序) */
QVector<double> SymmetricMatrix::eigenvalues() const
{
    m_timer.start();

    QVector<double> eigenvals;

    if (m_size == 0) {
        return eigenvals;
    }

    if (m_size == 1) {
        eigenvals.append(m_data[0]);
        return eigenvals;
    }

    /* 展开为全矩阵用于Jacobi旋转 */
    int n = m_size;
    QVector<QVector<double>> a(n);
    for (int i = 0; i < n; ++i) {
        a[i].resize(n);
        for (int j = 0; j < n; ++j) {
            a[i][j] = m_data[index(i, j)];
        }
    }

    int maxIter = 100 * n * n;
    double eps  = 1e-12;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 查找最大非对角元素 */
        double maxOff = 0.0;
        int pi = 0, pj = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double v = qAbs(a[i][j]);
                if (v > maxOff) {
                    maxOff = v;
                    pi = i;
                    pj = j;
                }
            }
        }

        /* 收敛判断 */
        if (maxOff < eps) break;

        /* 计算旋转参数 */
        double app = a[pi][pi];
        double aqq = a[pj][pj];
        double apq = a[pi][pj];

        double theta;
        if (qFuzzyIsNull(app - aqq)) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * qAtan2(2.0 * apq, app - aqq);
        }

        double c = qCos(theta);
        double s = qSin(theta);

        /* 应用Givens旋转 */
        for (int k = 0; k < n; ++k) {
            if (k == pi || k == pj) continue;
            double aki = a[k][pi];
            double akj = a[k][pj];
            a[k][pi] = a[pi][k] = c * aki + s * akj;
            a[k][pj] = a[pj][k] = -s * aki + c * akj;
        }

        double newPP = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        double newQQ = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        a[pi][pi] = newPP;
        a[pj][pj] = newQQ;
        a[pi][pj] = a[pj][pi] = 0.0;
    }

    /* 提取对角线上的特征值 */
    for (int i = 0; i < n; ++i) {
        eigenvals.append(a[i][i]);
    }

    /* 降序排列 */
    std::sort(eigenvals.begin(), eigenvals.end(), std::greater<double>());

    ++m_stats.totalOperations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("Eigenvalues"));
    return eigenvals;
}

/** @brief 计算行列式 @return 行列式值 */
double SymmetricMatrix::determinant() const
{
    m_timer.start();

    if (m_size == 0) {
        return 0.0;
    }

    /* 对于小矩阵直接计算 */
    if (m_size == 1) {
        return m_data[0];
    }

    if (m_size == 2) {
        return m_data[index(0, 0)] * m_data[index(1, 1)]
             - m_data[index(0, 1)] * m_data[index(0, 1)];
    }

    /* 对于大矩阵，行列式 = 特征值之积 */
    QVector<double> evals = eigenvalues();
    double det = 1.0;
    for (double ev : evals) {
        det *= ev;
    }

    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("Determinant"));
    return det;
}

/** @brief 重置统计 */
void SymmetricMatrix::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}

/** @brief 将(i,j)映射到紧凑存储索引 @param i 行 @param j 列 @return 线性索引 */
int SymmetricMatrix::index(int i, int j) const
{
    /* 确保i >= j，存下三角 */
    if (i < j) {
        std::swap(i, j);
    }
    return i * (i + 1) / 2 + j;
}
