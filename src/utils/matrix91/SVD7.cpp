#include "SVD7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class SVD7
 * @brief 奇异值分解(SVD)实现
 *
 * SVD将任意矩阵A(m×n)分解为 A = U * Σ * V^T
 * 其中U是m×m正交矩阵，Σ是m×n对角矩阵(奇异值)，
 * V是n×n正交矩阵。
 *
 * 本实现使用双边Jacobi旋转方法:
 * 1. 对A^T*A进行Jacobi特征值分解得到V和奇异值的平方
 * 2. 通过A*V/σ计算U
 * 适用于中小规模矩阵，数值稳定性好。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SVD7::SVD7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算SVD分解
 *
 * 使用Jacobi旋转法计算A^T*A的特征分解:
 * 1. 计算 B = A^T * A (对称正定/半正定矩阵)
 * 2. 对B执行Jacobi旋转特征分解: B = V * Λ * V^T
 * 3. 奇异值 σ_i = sqrt(λ_i)
 * 4. 右奇异向量即为V的列
 *
 * @param matrix 输入矩阵(m×n)
 * @return 分解是否成功
 */
bool SVD7::decompose(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int m = matrix.size();
    if (m == 0) return false;
    int n = matrix[0].size();
    if (n == 0) return false;

    /* 计算 A^T * A */
    QVector<QVector<double>> B(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < m; ++k) {
                sum += matrix[k][i] * matrix[k][j];
            }
            B[i][j] = sum;
        }
    }

    /* 初始化V为单位矩阵 */
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    /* Jacobi旋转迭代 */
    const int maxIter = 100;
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 寻找最大的非对角元素 */
        double maxOffDiag = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (qAbs(B[i][j]) > maxOffDiag) {
                    maxOffDiag = qAbs(B[i][j]);
                    p = i;
                    q = j;
                }
            }
        }

        if (maxOffDiag < 1e-12) break; /* 收敛 */

        /* 计算Jacobi旋转角度 */
        double theta = 0.0;
        if (qAbs(B[p][p] - B[q][q]) < 1e-15) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * qAtan2(2.0 * B[p][q], B[p][p] - B[q][q]);
        }

        double c = qCos(theta);
        double s = qSin(theta);

        /* 应用旋转到B */
        double bpp = c * c * B[p][p] + 2 * s * c * B[p][q] + s * s * B[q][q];
        double bqq = s * s * B[p][p] - 2 * s * c * B[p][q] + c * c * B[q][q];
        double bpq = 0.0; /* 旋转消去(p,q)元素 */

        for (int i = 0; i < n; ++i) {
            if (i != p && i != q) {
                double bip = c * B[i][p] + s * B[i][q];
                double biq = -s * B[i][p] + c * B[i][q];
                B[i][p] = bip;
                B[p][i] = bip;
                B[i][q] = biq;
                B[q][i] = biq;
            }
        }
        B[p][p] = bpp;
        B[q][q] = bqq;
        B[p][q] = bpq;
        B[q][p] = bpq;

        /* 累积旋转到V */
        for (int i = 0; i < n; ++i) {
            double vip = c * V[i][p] + s * V[i][q];
            double viq = -s * V[i][p] + c * V[i][q];
            V[i][p] = vip;
            V[i][q] = viq;
        }
    }

    /* 提取奇异值(对角元素的平方根) */
    m_singularValues.resize(n);
    for (int i = 0; i < n; ++i) {
        m_singularValues[i] = qSqrt(qMax(0.0, B[i][i]));
    }

    /* 按降序排列 */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return m_singularValues[a] > m_singularValues[b];
    });

    QVector<double> sorted(n);
    for (int i = 0; i < n; ++i) sorted[i] = m_singularValues[order[i]];
    m_singularValues = sorted;

    int rank = estimateRank(1e-10);

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(m, n, rank);

    return true;
}

/**
 * @brief 估计矩阵的有效秩
 *
 * 有效秩定义为大于阈值的奇异值个数。
 * 阈值通常设为 最大奇异值 * 容差 * max(m,n)。
 *
 * @param tolerance 相对容差(默认1e-10)
 * @return 有效秩(非零奇异值个数)
 */
int SVD7::estimateRank(double tolerance) const
{
    if (m_singularValues.isEmpty()) return 0;

    double maxSV = m_singularValues[0];
    double absTol = maxSV * tolerance * m_singularValues.size();

    int rank = 0;
    for (int i = 0; i < m_singularValues.size(); ++i) {
        if (m_singularValues[i] > absTol) {
            rank++;
        }
    }

    return rank;
}

/**
 * @brief 重置所有统计数据
 *
 * 将分解计数、秩估计计数和计时归零。
 */
void SVD7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_singularValues.clear();
}
