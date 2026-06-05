#include "GeneralizedEigen5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化广义特征值求解器
 * @param parent 父对象指针
 */
GeneralizedEigen5::GeneralizedEigen5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GeneralizedEigen5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置矩阵A(对称)和B(对称正定)
 *
 * @param A 对称矩阵A
 * @param B 对称正定矩阵B
 */
void GeneralizedEigen5::setMatrices(const QVector<QVector<double>>& A,
                                     const QVector<QVector<double>>& B)
{
    m_A = A;
    m_B = B;
}

/**
 * @brief 执行广义特征值分解
 *
 * 通过Cholesky分解B = L*L^T，将广义特征值问题 Ax = λBx
 * 变换为标准特征值问题 Cy = λy，其中 C = L^{-1} A L^{-T}。
 * 对C执行Jacobi对角化获取特征值和特征向量。
 *
 * @return 特征值向量(升序排列)
 */
QVector<double> GeneralizedEigen5::decompose()
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> eigenvalues;
    const int n = m_A.size();
    if (n == 0 || n != m_B.size()) {
        emit decompositionCompleted(0);
        return eigenvalues;
    }

    /* Step 1: Cholesky分解 B = L * L^T */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = m_B[i][j];
            for (int k = 0; k < j; ++k)
                sum -= L[i][k] * L[j][k];
            if (i == j) {
                if (sum <= 0.0) {
                    emit decompositionCompleted(0);
                    return eigenvalues;
                }
                L[i][j] = qSqrt(sum);
            } else {
                L[i][j] = sum / L[j][j];
            }
        }
    }

    /* Step 2: 计算 L^{-1}，前代法 */
    QVector<QVector<double>> Linv(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        Linv[i][i] = 1.0 / L[i][i];
        for (int j = i + 1; j < n; ++j) {
            double sum = 0.0;
            for (int k = i; k < j; ++k)
                sum -= L[j][k] * Linv[k][i];
            Linv[j][i] = sum / L[j][j];
        }
    }

    /* Step 3: 构造 C = Linv * A * Linv^T */
    /* 先计算 T = Linv * A */
    QVector<QVector<double>> T(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += Linv[i][k] * m_A[k][j];
            T[i][j] = sum;
        }
    }
    /* 再计算 C = T * Linv^T */
    QVector<QVector<double>> C(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += T[i][k] * Linv[j][k];
            C[i][j] = sum;
        }
    }

    /* Step 4: Jacobi迭代求C的特征值和特征向量 */
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    const int maxIter = 100;
    const double eps = 1e-12;
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 找最大非对角元素 */
        double maxOff = 0.0;
        int pi = 0, pj = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (qAbs(C[i][j]) > maxOff) {
                    maxOff = qAbs(C[i][j]);
                    pi = i;
                    pj = j;
                }
            }
        }
        if (maxOff < eps) break;

        /* 计算旋转角度 */
        double theta = 0.0;
        if (qFuzzyIsNull(C[pi][pi] - C[pj][pj])) {
            theta = M_PI_4;
        } else {
            theta = 0.5 * qAtan2(2.0 * C[pi][pj], C[pi][pi] - C[pj][pj]);
        }
        double c = qCos(theta);
        double s = qSin(theta);

        /* 应用Givens旋转 */
        for (int k = 0; k < n; ++k) {
            double aki = C[k][pi];
            double akj = C[k][pj];
            C[k][pi] = c * aki + s * akj;
            C[k][pj] = -s * aki + c * akj;
        }
        for (int k = 0; k < n; ++k) {
            double aip = C[pi][k];
            double ajp = C[pj][k];
            C[pi][k] = c * aip + s * ajp;
            C[pj][k] = -s * aip + c * ajp;
        }
        for (int k = 0; k < n; ++k) {
            double vki = V[k][pi];
            double vkj = V[k][pj];
            V[k][pi] = c * vki + s * vkj;
            V[k][pj] = -s * vki + c * vkj;
        }
    }

    /* 提取特征值并排序 */
    eigenvalues.resize(n);
    for (int i = 0; i < n; ++i)
        eigenvalues[i] = C[i][i];

    QVector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return eigenvalues[a] < eigenvalues[b];
    });

    QVector<double> sortedEigs(n);
    QVector<QVector<double>> sortedVecs(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        sortedEigs[i] = eigenvalues[idx[i]];
        for (int j = 0; j < n; ++j)
            sortedVecs[j][i] = V[j][idx[i]];
    }

    /* Step 5: 还原原始特征向量 x = L^{-T} y */
    m_eigenvectors.resize(n);
    for (int i = 0; i < n; ++i) {
        m_eigenvectors[i].resize(n);
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += Linv[k][i] * sortedVecs[k][j];
            m_eigenvectors[i][j] = sum;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecomposed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecomposed;

    emit decompositionCompleted(n);
    return sortedEigs;
}
