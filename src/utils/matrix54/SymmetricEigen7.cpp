/**
 * @file SymmetricEigen7.cpp
 * @brief 对称矩阵特征值分解实现
 *
 * 实现对称矩阵的Jacobi特征值算法。通过一系列旋转相似变换
 * 将对称矩阵对角化，对角元素即为特征值，旋转变换的累积
 * 构成特征向量矩阵。适用于中小规模稠密对称矩阵。
 * 使用QElapsedTimer计时并累积统计信息。
 */

#include "utils/matrix54/SymmetricEigen7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/**
 * @class SymmetricEigen7
 * @brief 对称矩阵特征值分解器（Jacobi方法）
 *
 * Jacobi方法反复选择矩阵中绝对值最大的非对角线元素，
 * 通过Givens旋转将其消为零。虽然每次旋转会引入新的
 * 非零元素，但总体上非对角线元素的范数单调递减，
 * 最终收敛到对角矩阵。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject指针
 */
SymmetricEigen7::SymmetricEigen7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 对对称矩阵执行特征值分解
 *
 * 输入矩阵以行优先一维数组形式存储（仅下三角即可）。
 * 算法执行Jacobi迭代直至收敛（非对角线元素小于阈值）
 * 或达到最大迭代次数。
 *
 * @param A 对称矩阵数据，长度为n*n
 * @param n 矩阵维度
 * @return 分解成功返回true
 */
bool SymmetricEigen7::decompose(const QVector<double>& A, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || A.size() < n * n) {
        m_timeSum += timer.elapsed();
        return false;
    }

    m_n = n;

    /* 复制输入矩阵（Jacobi迭代会修改矩阵） */
    QVector<double> mat = A;

    /* 初始化特征向量矩阵为单位矩阵 */
    m_eigenvectors.fill(0.0, n * n);
    for (int i = 0; i < n; ++i) {
        m_eigenvectors[i * n + i] = 1.0;
    }

    /* 执行Jacobi迭代 */
    jacobiIteration(mat, m_eigenvectors, n);

    /* 提取特征值（对角线元素） */
    m_eigenvalues.resize(n);
    for (int i = 0; i < n; ++i) {
        m_eigenvalues[i] = mat[i * n + i];
    }

    m_decomposed = true;

    m_stats.totalDecompositions++;
    m_stats.matrixSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions > 0)
        ? m_timeSum / m_stats.totalDecompositions : 0.0;

    emit decompositionCompleted(n);
    return true;
}

/**
 * @brief 检查矩阵是否正定
 *
 * 矩阵正定当且仅当所有特征值严格大于0。
 * 必须先调用decompose()执行分解。
 *
 * @return 所有特征值>0返回true
 */
bool SymmetricEigen7::isPositiveDefinite() const
{
    if (!m_decomposed) return false;
    for (double val : m_eigenvalues) {
        if (val <= 0.0) return false;
    }
    return true;
}

/**
 * @brief 重置统计数据
 */
void SymmetricEigen7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief Jacobi迭代核心算法
 *
 * 反复执行以下步骤直至收敛：
 * 1. 在非对角线元素中找到绝对值最大的元素 a[p][q]
 * 2. 计算Givens旋转角度 theta = 0.5 * atan2(2*a[p][q], a[q][q]-a[p][p])
 * 3. 对第p行/列和第q行/列执行旋转相似变换
 * 4. 累积旋转变换到特征向量矩阵
 *
 * @param mat 输入/输出矩阵，最终趋于对角矩阵
 * @param ev 特征向量矩阵（累积旋转变换）
 * @param n 矩阵维度
 */
void SymmetricEigen7::jacobiIteration(QVector<double>& mat, QVector<double>& ev, int n)
{
    const int maxIter = 100 * n * n;  /* 最大迭代次数 */
    const double tol = 1e-12;         /* 收敛阈值 */

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 查找绝对值最大的非对角线元素 */
        double maxOff = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double val = qAbs(mat[i * n + j]);
                if (val > maxOff) {
                    maxOff = val;
                    p = i;
                    q = j;
                }
            }
        }

        /* 收敛检查：最大非对角线元素足够小 */
        if (maxOff < tol) break;

        /* 计算旋转角度 */
        double app = mat[p * n + p];
        double aqq = mat[q * n + q];
        double apq = mat[p * n + q];

        double theta;
        if (qAbs(app - aqq) < 1e-15) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * qAtan2(2.0 * apq, aqq - app);
        }

        double c = qCos(theta);
        double s = qSin(theta);

        /* 执行旋转相似变换：A' = J^T * A * J */
        /* 更新第p行和第q行 */
        for (int i = 0; i < n; ++i) {
            if (i == p || i == q) continue;
            double aip = mat[i * n + p];
            double aiq = mat[i * n + q];
            mat[i * n + p] = c * aip - s * aiq;
            mat[p * n + i] = mat[i * n + p];
            mat[i * n + q] = s * aip + c * aiq;
            mat[q * n + i] = mat[i * n + q];
        }

        /* 更新对角线和非对角线元素 */
        double newPP = c * c * app - 2.0 * s * c * apq + s * s * aqq;
        double newQQ = s * s * app + 2.0 * s * c * apq + c * c * aqq;
        mat[p * n + p] = newPP;
        mat[q * n + q] = newQQ;
        mat[p * n + q] = 0.0;
        mat[q * n + p] = 0.0;

        /* 累积旋转变换到特征向量矩阵 */
        for (int i = 0; i < n; ++i) {
            double eip = ev[i * n + p];
            double eiq = ev[i * n + q];
            ev[i * n + p] = c * eip - s * eiq;
            ev[i * n + q] = s * eip + c * eiq;
        }
    }
}

/**
 * @brief 特征值排序
 *
 * 将特征值按绝对值从大到小排序，同时重排对应的特征向量。
 * 便于按主成分重要性截取。
 *
 * @param descending true为降序，false为升序
 */
void SymmetricEigen7::sortEigenvalues(bool descending)
{
    if (!m_decomposed || m_n <= 0) return;

    /* 构建索引-值对并排序 */
    QVector<QPair<double, int>> order(m_n);
    for (int i = 0; i < m_n; ++i) {
        order[i] = {m_eigenvalues[i], i};
    }

    if (descending) {
        std::sort(order.begin(), order.end(),
            [](const QPair<double,int>& a, const QPair<double,int>& b) {
                return a.first > b.first;
            });
    } else {
        std::sort(order.begin(), order.end(),
            [](const QPair<double,int>& a, const QPair<double,int>& b) {
                return a.first < b.first;
            });
    }

    /* 重排特征值和特征向量 */
    QVector<double> sortedVals(m_n);
    QVector<double> sortedVecs(m_n * m_n);

    for (int newIdx = 0; newIdx < m_n; ++newIdx) {
        int oldIdx = order[newIdx].second;
        sortedVals[newIdx] = m_eigenvalues[oldIdx];
        for (int row = 0; row < m_n; ++row) {
            sortedVecs[row * m_n + newIdx] = m_eigenvectors[row * m_n + oldIdx];
        }
    }

    m_eigenvalues = sortedVals;
    m_eigenvectors = sortedVecs;
}

/**
 * @brief 计算矩阵的条件数（最大特征值/最小特征值之比）
 * @return 条件数，分解未完成返回-1
 */
double SymmetricEigen7::conditionNumber() const
{
    if (!m_decomposed || m_eigenvalues.isEmpty()) return -1.0;
    double maxVal = m_eigenvalues[0];
    double minVal = m_eigenvalues[0];
    for (double v : m_eigenvalues) {
        double av = qAbs(v);
        if (av > qAbs(maxVal)) maxVal = v;
        if (av < qAbs(minVal)) minVal = v;
    }
    if (qAbs(minVal) < 1e-15) return std::numeric_limits<double>::infinity();
    return qAbs(maxVal) / qAbs(minVal);
}
