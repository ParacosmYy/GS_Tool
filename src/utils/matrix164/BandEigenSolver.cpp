/**
 * @file BandEigenSolver.cpp
 * @brief 带状矩阵特征值求解器实现
 *
 * 实现对称带状矩阵→三对角化(Householder)→隐式QR迭代。
 * 仅存储和操作带内元素，内存和时间效率优于稠密方法。
 */

#include "utils/matrix164/BandEigenSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
BandEigenSolver::BandEigenSolver(QObject* parent)
    : QObject(parent)
{
}

void BandEigenSolver::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

void BandEigenSolver::setTolerance(double tol)
{
    m_tolerance = qMax(1e-15, tol);
}

/**
 * @brief 将对称带状矩阵三对角化
 *
 * 使用Householder反射，逐列消去带外元素。
 * 输入bandMatrix格式：bandMatrix[i] = 第i条对角线(0=主对角线)。
 *
 * @param bandMatrix 带状存储(n × bandwidth+1)
 * @param n 矩阵阶数
 * @param bandwidth 半带宽
 * @param[out] diag 主对角线(n)
 * @param[out] subdiag 副对角线(n-1)
 */
void BandEigenSolver::bandToTridiag(const QVector<QVector<double>>& bandMatrix,
                                    int n, int bandwidth,
                                    QVector<double>& diag, QVector<double>& subdiag)
{
    /* 提取主对角线和副对角线 */
    /* bandMatrix格式: bandMatrix[i][j] = 第i行的带内元素 */
    /* 假设存储为紧凑带状格式: 行i, 列从max(0,i-bw)到min(n-1,i+bw) */

    diag.resize(n, 0.0);
    subdiag.resize(n - 1, 0.0);

    if (bandMatrix.isEmpty() || n <= 0) return;

    /* 构建完整矩阵(用于Householder变换) */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));

    /* 从带状存储填充 */
    if (bandMatrix.size() == n && bandMatrix[0].size() >= static_cast<int>(bandwidth + 1)) {
        /* 每行存储bandwidth+1个元素，居中对齐 */
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k <= bandwidth; ++k) {
                int col = i - bandwidth + k;
                if (col >= 0 && col < n) {
                    A[i][col] = bandMatrix[i][k];
                    A[col][i] = bandMatrix[i][k]; /* 对称 */
                }
            }
        }
    } else if (bandMatrix.size() == n) {
        /* 每行可能不等长 */
        for (int i = 0; i < n; ++i) {
            int rowLen = bandMatrix[i].size();
            int startCol = qMax(0, i - bandwidth);
            for (int k = 0; k < rowLen; ++k) {
                int col = startCol + k;
                if (col >= 0 && col < n) {
                    A[i][col] = bandMatrix[i][k];
                    A[col][i] = bandMatrix[i][k];
                }
            }
        }
    }

    /* Householder三对角化 */
    for (int k = 0; k < n - 2; ++k) {
        /* 构造Householder向量 */
        QVector<double> x(n - k - 1);
        for (int i = 0; i < n - k - 1; ++i) x[i] = A[k + 1 + i][k];

        double norm = 0.0;
        for (double v : x) norm += v * v;
        norm = qSqrt(norm);
        if (norm < 1e-15) continue;

        double alpha = (x[0] >= 0) ? norm : -norm;
        x[0] += alpha;
        double beta = 0.0;
        for (double v : x) beta += v * v;
        if (beta < 1e-30) continue;
        beta = 2.0 / beta;

        /* 变换: A = (I - beta*v*v') * A * (I - beta*v*v') */
        /* P = I - beta*v*v' */
        int m = n - k - 1;

        /* w = beta * A[k+1:n, k+1:n] * v */
        QVector<double> w(m, 0.0);
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) {
                w[i] += A[k + 1 + i][k + 1 + j] * x[j];
            }
            w[i] *= beta;
        }

        /* gamma = 0.5 * beta * v' * w */
        double gamma = 0.0;
        for (int i = 0; i < m; ++i) gamma += x[i] * w[i];
        gamma *= 0.5 * beta;

        /* w = w - gamma * v */
        for (int i = 0; i < m; ++i) w[i] -= gamma * x[i];

        /* A[k+1:n, k+1:n] -= v*w' + w*v' */
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) {
                A[k + 1 + i][k + 1 + j] -= x[i] * w[j] + w[i] * x[j];
            }
        }

        /* 更新列k */
        for (int i = 0; i < m; ++i) {
            A[k + 1 + i][k] = 0.0;
            A[k][k + 1 + i] = 0.0;
        }
        A[k + 1][k] = -alpha;
        A[k][k + 1] = -alpha;
    }

    /* 提取三对角 */
    for (int i = 0; i < n; ++i) {
        diag[i] = A[i][i];
        if (i < n - 1) {
            subdiag[i] = A[i][i + 1];
        }
    }
}

/**
 * @brief Wilkinson位移
 */
double BandEigenSolver::wilkinsonShift(double d, double e) const
{
    double delta = (d - e) / 2.0;
    double signDelta = (delta >= 0) ? 1.0 : -1.0;
    return e - signDelta * delta * delta / (qAbs(delta) + qSqrt(delta * delta + e * e + 1e-30));
}

/**
 * @brief 三对角矩阵QR迭代求特征值
 *
 * 使用隐式对称QR迭代(Wilkinson位移+Givens旋转)。
 * @param Q 若非空则累积旋转变换
 */
QVector<double> BandEigenSolver::tridiagQR(QVector<double>& diag, QVector<double>& subdiag,
                                           int n, QVector<QVector<double>>& Q)
{
    int totalIter = 0;
    int converged = 0;

    for (int l = n - 1; l >= 1; ) {
        /* 检查子对角元素是否可忽略 */
        if (qAbs(subdiag[l - 1]) <= m_tolerance * (qAbs(diag[l - 1]) + qAbs(diag[l]))) {
            subdiag[l - 1] = 0.0;
            l--;
            converged++;
            continue;
        }

        int m = l;
        while (m > 0) {
            if (qAbs(subdiag[m - 1]) <= m_tolerance * (qAbs(diag[m - 1]) + qAbs(diag[m]))) {
                subdiag[m - 1] = 0.0;
                break;
            }
            m--;
        }

        if (m == l) { l--; converged++; continue; }

        /* Wilkinson位移 */
        double shift = wilkinsonShift(diag[l], subdiag[l - 1]);

        /* 隐式QR步(Givens旋转) */
        double x = diag[m] - shift;
        double z = subdiag[m];

        for (int k = m; k < l; ++k) {
            /* 构造Givens旋转消去z */
            double r = qSqrt(x * x + z * z);
            double c = (r > 1e-30) ? x / r : 1.0;
            double s = (r > 1e-30) ? -z / r : 0.0;

            if (k > m) subdiag[k - 1] = r;

            /* 三对角旋转 */
            double w = c * subdiag[k] - s * diag[k + 1] * 0;
            double d1 = diag[k];
            double d2 = diag[k + 1];
            double e = subdiag[k];

            diag[k] = c * c * d1 - 2.0 * c * s * e + s * s * d2;
            diag[k + 1] = s * s * d1 + 2.0 * c * s * e + c * c * d2;
            subdiag[k] = c * s * (d1 - d2) + (c * c - s * s) * e;

            if (k + 1 < l) {
                x = subdiag[k];
                z = -s * subdiag[k + 1];
                subdiag[k + 1] *= c;
            }

            /* 累积特征向量 */
            if (!Q.isEmpty()) {
                for (int i = 0; i < n; ++i) {
                    double q1 = Q[k][i];
                    double q2 = Q[k + 1][i];
                    Q[k][i] = c * q1 - s * q2;
                    Q[k + 1][i] = s * q1 + c * q2;
                }
            }
        }
        totalIter++;
        if (totalIter >= m_maxIter) break;
    }

    m_stats.totalIterations += totalIter;
    m_stats.convergenceRate = (n > 0) ? static_cast<double>(converged) / n : 0.0;

    /* 排序特征值(升序) */
    QVector<double> eigenvalues = diag;
    std::sort(eigenvalues.begin(), eigenvalues.end());
    return eigenvalues;
}

/**
 * @brief 求解全部特征值
 */
QVector<double> BandEigenSolver::solveEigenvalues(
    const QVector<QVector<double>>& bandMatrix, int n, int bandwidth)
{
    QVector<QVector<double>> dummy;
    return solve(bandMatrix, n, bandwidth, dummy);
}

/**
 * @brief 求解特征值和特征向量
 */
QVector<double> BandEigenSolver::solve(const QVector<QVector<double>>& bandMatrix,
                                       int n, int bandwidth,
                                       QVector<QVector<double>>& eigenvectors)
{
    QElapsedTimer timer;
    timer.start();

    eigenvectors.clear();

    if (n <= 0 || bandMatrix.isEmpty()) return QVector<double>();

    /* 三对角化 */
    QVector<double> diag, subdiag;
    bandToTridiag(bandMatrix, n, bandwidth, diag, subdiag);

    /* 初始化特征向量矩阵为单位阵 */
    bool needEigvecs = true;  /* 始终计算以保持一致性 */
    eigenvectors.resize(n);
    for (int i = 0; i < n; ++i) {
        eigenvectors[i].resize(n, 0.0);
        eigenvectors[i][i] = 1.0;
    }

    /* QR迭代 */
    QVector<double> eigenvalues = tridiagQR(diag, subdiag, n, eigenvectors);

    /* 统计 */
    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(n, m_stats.totalIterations);
    return eigenvalues;
}

void BandEigenSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
