#include "SymmetricEigen10.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class SymmetricEigen10
 * @brief 对称矩阵特征值分解实现
 *
 * 使用经典Jacobi旋转迭代法求解对称矩阵的全部特征值和特征向量。
 * Jacobi方法通过一系列平面旋转变换逐步消去非对角元素，
 * 最终使矩阵对角化，对角元素即为特征值。
 *
 * 每步选择绝对值最大的非对角元素a[p][q]，构造旋转矩阵
 * 使变换后a[p][q] = a[q][p] = 0。旋转角度:
 * tan(2θ) = 2*a[p][q] / (a[p][p] - a[q][q])
 *
 * 收敛速度: 二次收敛，通常需要O(n^2)次旋转。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SymmetricEigen10::SymmetricEigen10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算对称矩阵的特征值和特征向量
 *
 * Jacobi迭代过程:
 * 1. 初始化V为单位矩阵(累积旋转)
 * 2. 循环: 找到最大的非对角元素|a[p][q]|
 * 3. 如果|a[p][q]| < ε，则已收敛
 * 4. 计算旋转角度θ，构造Givens旋转矩阵
 * 5. 应用旋转: A' = G^T * A * G
 * 6. 累积旋转: V = V * G
 * 7. 重复直到收敛或达到最大迭代次数
 *
 * @param matrix 对称输入矩阵(n×n)
 * @return 分解是否成功
 */
bool SymmetricEigen10::decompose(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return false;

    /* 检查方阵 */
    for (int i = 0; i < n; ++i) {
        if (matrix[i].size() != n) return false;
    }

    /* 复制到工作矩阵A */
    QVector<QVector<double>> A = matrix;

    /* 初始化特征向量矩阵V为单位矩阵 */
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    int totalRotations = 0;
    const int maxIter = 100 * n * n; /* 最大迭代次数 */
    const double tol = 1e-12;        /* 收敛容差 */

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 寻找最大的非对角元素 */
        double maxOffDiag = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (qAbs(A[i][j]) > maxOffDiag) {
                    maxOffDiag = qAbs(A[i][j]);
                    p = i;
                    q = j;
                }
            }
        }

        /* 检查收敛 */
        if (maxOffDiag < tol) break;

        /* 计算旋转参数 */
        double app = A[p][p];
        double aqq = A[q][q];
        double apq = A[p][q];

        double theta;
        if (qAbs(app - aqq) < 1e-15) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * qAtan2(2.0 * apq, app - aqq);
        }

        double c = qCos(theta);
        double s = qSin(theta);

        /* 应用旋转变换到A */
        /* 更新第p行和第q行 */
        for (int i = 0; i < n; ++i) {
            if (i == p || i == q) continue;
            double aip = A[i][p];
            double aiq = A[i][q];
            A[i][p] = c * aip + s * aiq;
            A[p][i] = A[i][p];
            A[i][q] = -s * aip + c * aiq;
            A[q][i] = A[i][q];
        }

        /* 更新对角和(p,q)位置 */
        double newApp = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        double newAqq = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[p][p] = newApp;
        A[q][q] = newAqq;
        A[p][q] = 0.0;
        A[q][p] = 0.0;

        /* 累积旋转到V */
        for (int i = 0; i < n; ++i) {
            double vip = V[i][p];
            double viq = V[i][q];
            V[i][p] = c * vip + s * viq;
            V[i][q] = -s * vip + c * viq;
        }

        totalRotations++;
    }

    /* 提取特征值(对角元素) */
    m_eigenvalues.resize(n);
    for (int i = 0; i < n; ++i) {
        m_eigenvalues[i] = A[i][i];
    }

    /* 按升序排列 */
    std::sort(m_eigenvalues.begin(), m_eigenvalues.end());

    m_stats.totalDecompositions++;
    m_stats.totalRotations += totalRotations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, totalRotations);

    return true;
}

/**
 * @brief 获取特征值(升序排列)
 *
 * 返回经过Jacobi迭代计算后按升序排列的特征值。
 * 必须在decompose()调用之后使用。
 *
 * @return 特征值向量(升序)
 */
QVector<double> SymmetricEigen10::eigenvalues() const
{
    return m_eigenvalues;
}

/**
 * @brief 重置所有统计数据
 *
 * 将分解计数、旋转计数和计时归零。
 */
void SymmetricEigen10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_eigenvalues.clear();
}
