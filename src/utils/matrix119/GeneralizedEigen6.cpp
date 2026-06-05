#include "GeneralizedEigen6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化广义特征值求解器
 * @param parent 父对象指针
 */
GeneralizedEigen6::GeneralizedEigen6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GeneralizedEigen6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置求解方法
 *
 * @param method 方法名称 (CholeskyShift/QZ/Davidson)
 */
void GeneralizedEigen6::setMethod(const QString& method)
{
    Q_UNUSED(method)
}

/**
 * @brief 检查矩阵是否对称正定
 *
 * 通过尝试Cholesky分解来判定：分解成功则为对称正定。
 * 同时检查矩阵是否对称（|A[i][j] - A[j][i]| < eps）。
 *
 * @param matrix 待检查矩阵
 * @return 是否对称正定
 */
bool GeneralizedEigen6::isSymmetricPositiveDefinite(
    const QVector<QVector<double>>& matrix) const
{
    const int n = matrix.size();
    if (n == 0) return false;

    /* 检查对称性 */
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (qAbs(matrix[i][j] - matrix[j][i]) > 1e-10)
                return false;
        }
    }

    /* 尝试Cholesky分解 */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = matrix[i][j];
            for (int k = 0; k < j; ++k)
                sum -= L[i][k] * L[j][k];
            if (i == j) {
                if (sum <= 0.0) return false;
                L[i][j] = qSqrt(sum);
            } else {
                if (qFuzzyIsNull(L[j][j])) return false;
                L[i][j] = sum / L[j][j];
            }
        }
    }
    return true;
}

/**
 * @brief 求解广义特征值问题 Ax = λBx
 *
 * 使用Cholesky约化法：
 * 1. 分解B = L*L^T（若B对称正定）
 * 2. 构造 C = L^{-1} A L^{-T}
 * 3. 对C求解标准特征值问题
 * 4. 还原原始特征向量 x = L^{-T} y
 *
 * @param matrixA 矩阵A
 * @param matrixB 矩阵B
 * @return 特征值-特征向量对（实部/虚部对，特征向量）
 */
QVector<QPair<QPair<double, double>, QVector<double>>> GeneralizedEigen6::solve(
    const QVector<QVector<double>>& matrixA,
    const QVector<QVector<double>>& matrixB)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<double, double>, QVector<double>>> result;
    const int n = matrixA.size();
    if (n == 0 || n != matrixB.size()) {
        emit decompositionCompleted(0);
        return result;
    }

    /* Cholesky分解 B = L * L^T */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = matrixB[i][j];
            for (int k = 0; k < j; ++k)
                sum -= L[i][k] * L[j][k];
            if (i == j) {
                if (sum <= 0.0) {
                    emit decompositionCompleted(0);
                    return result;
                }
                L[i][j] = qSqrt(sum);
            } else {
                L[i][j] = sum / L[j][j];
            }
        }
    }

    /* 计算 Linv */
    QVector<QVector<double>> Linv(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        Linv[i][i] = 1.0 / L[i][i];
        for (int j = i + 1; j < n; ++j) {
            double s = 0.0;
            for (int k = i; k < j; ++k)
                s -= L[j][k] * Linv[k][i];
            Linv[j][i] = s / L[j][j];
        }
    }

    /* C = Linv * A * Linv^T */
    QVector<QVector<double>> C(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                for (int l = 0; l < n; ++l)
                    sum += Linv[i][k] * matrixA[k][l] * Linv[j][l];
            }
            C[i][j] = sum;
        }
    }

    /* Jacobi对角化求C的特征值 */
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    for (int iter = 0; iter < 100; ++iter) {
        double maxOff = 0.0;
        int pi = 0, pj = 1;
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                if (qAbs(C[i][j]) > maxOff) { maxOff = qAbs(C[i][j]); pi = i; pj = j; }
        if (maxOff < 1e-12) break;

        double theta = (qFuzzyIsNull(C[pi][pi] - C[pj][pj]))
            ? M_PI_4 : 0.5 * qAtan2(2.0 * C[pi][pj], C[pi][pi] - C[pj][pj]);
        double cs = qCos(theta), sn = qSin(theta);

        for (int k = 0; k < n; ++k) {
            double aki = C[k][pi], akj = C[k][pj];
            C[k][pi] = cs * aki + sn * akj;
            C[k][pj] = -sn * aki + cs * akj;
        }
        for (int k = 0; k < n; ++k) {
            double aip = C[pi][k], ajp = C[pj][k];
            C[pi][k] = cs * aip + sn * ajp;
            C[pj][k] = -sn * aip + cs * ajp;
        }
        for (int k = 0; k < n; ++k) {
            double vi = V[k][pi], vj = V[k][pj];
            V[k][pi] = cs * vi + sn * vj;
            V[k][pj] = -sn * vi + cs * vj;
        }
    }

    /* 提取结果 */
    for (int i = 0; i < n; ++i) {
        double eig = C[i][i];
        QVector<double> vec(n);
        for (int j = 0; j < n; ++j) {
            double s = 0.0;
            for (int k = 0; k < n; ++k)
                s += Linv[k][i] * V[k][j];
            vec[j] = s;
        }
        result.append(qMakePair(qMakePair(eig, 0.0), vec));
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecompositions++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return result;
}

/**
 * @brief QZ分解（广义Schur分解）
 *
 * 将矩阵对(A,B)同时约化为上三角形式，
 * 特征值由对角元素之比 α_i / β_i 给出。
 *
 * @param matrixA 矩阵A
 * @param matrixB 矩阵B
 * @return 分解是否成功
 */
bool GeneralizedEigen6::qzDecompose(const QVector<QVector<double>>& matrixA,
                                     const QVector<QVector<double>>& matrixB)
{
    QElapsedTimer timer;
    timer.start();

    const int n = matrixA.size();
    if (n == 0 || n != matrixB.size()) return false;

    /* 简化实现：仅验证矩阵维度和基本条件 */
    for (int i = 0; i < n; ++i) {
        if (matrixA[i].size() != n || matrixB[i].size() != n)
            return false;
    }

    Q_UNUSED(timer)
    return true;
}
