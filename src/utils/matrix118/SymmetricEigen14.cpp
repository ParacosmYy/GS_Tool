#include "SymmetricEigen14.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化对称特征值分解器
 * @param parent 父对象指针
 */
SymmetricEigen14::SymmetricEigen14(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SymmetricEigen14::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置求解算法
 *
 * @param algorithm 算法名称 (QR/DivideConquer/Jacobi/Bisection)
 */
void SymmetricEigen14::setAlgorithm(const QString& algorithm)
{
    Q_UNUSED(algorithm)
}

/**
 * @brief 计算全部特征值和特征向量（Jacobi方法）
 *
 * Jacobi旋转方法：每次迭代选择最大非对角元素，
 * 构造Givens旋转将其消为零，累积旋转矩阵得到特征向量。
 * 迭代直到所有非对角元素小于阈值。
 *
 * @param matrix 对称矩阵
 * @return 特征值-特征向量对，按特征值降序排列
 */
QVector<QPair<double, QVector<double>>> SymmetricEigen14::compute(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, QVector<double>>> result;
    const int n = matrix.size();
    if (n == 0) {
        emit decompositionCompleted(0);
        return result;
    }

    /* 复制矩阵到工作区 */
    QVector<QVector<double>> A = matrix;
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    /* Jacobi迭代 */
    const int maxIter = 100 * n * n;
    const double eps = 1e-12;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 查找最大非对角元素 */
        double maxOff = 0.0;
        int pi = 0, pj = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (qAbs(A[i][j]) > maxOff) {
                    maxOff = qAbs(A[i][j]);
                    pi = i;
                    pj = j;
                }
            }
        }
        if (maxOff < eps) break;

        /* 计算旋转角度 */
        double theta = 0.0;
        if (qFuzzyIsNull(A[pi][pi] - A[pj][pj])) {
            theta = M_PI_4;
        } else {
            theta = 0.5 * qAtan2(2.0 * A[pi][pj], A[pi][pi] - A[pj][pj]);
        }
        double c = qCos(theta);
        double s = qSin(theta);

        /* 应用Givens旋转 A = G^T * A * G */
        /* 更新行pi和pj */
        for (int k = 0; k < n; ++k) {
            if (k == pi || k == pj) continue;
            double aki = A[pi][k];
            double akj = A[pj][k];
            A[pi][k] = c * aki + s * akj;
            A[pj][k] = -s * aki + c * akj;
            A[k][pi] = A[pi][k];
            A[k][pj] = A[pj][k];
        }
        double app = A[pi][pi];
        double aqq = A[pj][pj];
        double apq = A[pi][pj];
        A[pi][pi] = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        A[pj][pj] = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[pi][pj] = 0.0;
        A[pj][pi] = 0.0;

        /* 累积旋转到特征向量矩阵 */
        for (int k = 0; k < n; ++k) {
            double vki = V[k][pi];
            double vkj = V[k][pj];
            V[k][pi] = c * vki + s * vkj;
            V[k][pj] = -s * vki + c * vkj;
        }
    }

    /* 提取结果并按特征值降序排列 */
    QVector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return A[a][a] > A[b][b];
    });

    for (int i = 0; i < n; ++i) {
        double eig = A[idx[i]][idx[i]];
        QVector<double> vec(n);
        for (int j = 0; j < n; ++j)
            vec[j] = V[j][idx[i]];
        result.append(qMakePair(eig, vec));
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecompositions++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return result;
}

/**
 * @brief 仅计算特征值（不计算特征向量，速度更快）
 *
 * 对矩阵执行Jacobi旋转只追踪对角元素，
 * 跳过特征向量累积以节省计算量。
 *
 * @param matrix 对称矩阵
 * @return 特征值列表，降序排列
 */
QVector<double> SymmetricEigen14::eigenvaluesOnly(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> eigs;
    const int n = matrix.size();
    if (n == 0) return eigs;

    QVector<QVector<double>> A = matrix;
    const int maxIter = 100 * n * n;
    const double eps = 1e-12;

    for (int iter = 0; iter < maxIter; ++iter) {
        double maxOff = 0.0;
        int pi = 0, pj = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (qAbs(A[i][j]) > maxOff) {
                    maxOff = qAbs(A[i][j]);
                    pi = i;
                    pj = j;
                }
            }
        }
        if (maxOff < eps) break;

        double theta = 0.0;
        if (qFuzzyIsNull(A[pi][pi] - A[pj][pj])) {
            theta = M_PI_4;
        } else {
            theta = 0.5 * qAtan2(2.0 * A[pi][pj], A[pi][pi] - A[pj][pj]);
        }
        double c = qCos(theta);
        double s = qSin(theta);

        for (int k = 0; k < n; ++k) {
            if (k == pi || k == pj) continue;
            double aki = A[pi][k];
            double akj = A[pj][k];
            A[pi][k] = c * aki + s * akj;
            A[pj][k] = -s * aki + c * akj;
            A[k][pi] = A[pi][k];
            A[k][pj] = A[pj][k];
        }
        double app = A[pi][pi];
        double aqq = A[pj][pj];
        double apq = A[pi][pj];
        A[pi][pi] = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        A[pj][pj] = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[pi][pj] = 0.0;
        A[pj][pi] = 0.0;
    }

    for (int i = 0; i < n; ++i)
        eigs.append(A[i][i]);

    std::sort(eigs.begin(), eigs.end(), std::greater<double>());

    Q_UNUSED(timer)
    return eigs;
}

/**
 * @brief 计算指定范围内的特征值
 *
 * 先计算所有特征值和特征向量，再筛选指定范围。
 *
 * @param matrix 对称矩阵
 * @param minVal 特征值下界
 * @param maxVal 特征值上界
 * @return 范围内的特征值-特征向量对
 */
QVector<QPair<double, QVector<double>>> SymmetricEigen14::eigenvaluesInRange(
    const QVector<QVector<double>>& matrix, double minVal, double maxVal)
{
    QElapsedTimer timer;
    timer.start();

    auto all = compute(matrix);

    QVector<QPair<double, QVector<double>>> filtered;
    for (const auto& pair : all) {
        if (pair.first >= minVal && pair.first <= maxVal)
            filtered.append(pair);
    }

    Q_UNUSED(timer)
    return filtered;
}
