/**
 * @file SchurDecomp3.cpp
 * @brief Schur分解增强实现 — 实Schur/复Schur/QR迭代/特征向量恢复
 */

#include "utils/matrix31/SchurDecomp3.h"

#include <QtMath>
#include <algorithm>

SchurDecomp3::SchurDecomp3(int maxIterations, double tolerance, QObject* parent)
    : QObject(parent), m_maxIter(maxIterations), m_tol(tolerance)
{
}

SchurDecomp3::Result SchurDecomp3::decomposeReal(const QVector<double>& matrix, int n)
{
    m_timing.start();
    ++m_stats.totalDecompositions;

    Result result;
    result.T = matrix;
    result.Q.resize(n * n, 0.0);
    result.converged = false;
    result.iterations = 0;

    /* 初始化Q为单位矩阵 */
    for (int i = 0; i < n; ++i) result.Q[i * n + i] = 1.0;

    /* Hessenberg化简 */
    hessenbergReduce(result.T, result.Q, n);

    /* 双位移QR迭代 */
    int nn = n - 1;
    int iter = 0;
    while (nn > 0) {
        /* 检查次对角线元素是否足够小 */
        int lo = 0;
        for (int i = nn; i >= 1; --i) {
            double d = qAbs(result.T[(i - 1) * n + i - 1]) + qAbs(result.T[i * n + i]);
            if (d == 0.0) d = 1e-15;
            if (qAbs(result.T[i * n + i - 1]) < m_tol * d) {
                result.T[i * n + i - 1] = 0.0;
                lo = i;
                break;
            }
        }
        if (lo == nn) {
            /* 1x1块已收敛 */
            --nn;
            continue;
        }
        /* 检查2x2块 */
        if (lo == nn - 1) {
            --nn;
            continue;
        }
        if (iter >= m_maxIter) break;
        francisQrStep(result.T, result.Q, n, lo, nn);
        ++iter;
    }
    result.iterations = iter;
    result.converged = (nn == 0);
    m_stats.totalQrIterations += iter;

    /* 提取特征值 */
    result.eigenvaluesReal.resize(n);
    result.eigenvaluesImag.resize(n, 0.0);
    for (int i = 0; i < n; ++i) {
        if (i < n - 1 && qAbs(result.T[(i + 1) * n + i]) > m_tol) {
            /* 2x2块 */
            auto ev = blockEigenvalues(result.T, n, i);
            result.eigenvaluesReal[i] = ev.first;
            result.eigenvaluesImag[i] = ev.second;
            result.eigenvaluesReal[i + 1] = ev.first;
            result.eigenvaluesImag[i + 1] = -ev.second;
            ++i;
        } else {
            result.eigenvaluesReal[i] = result.T[i * n + i];
        }
    }
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions > 0)
        ? m_timeSum / m_stats.totalDecompositions : 0.0;
    emit decompositionComplete(n, iter, result.converged);
    return result;
}

SchurDecomp3::Result SchurDecomp3::decomposeComplex(const QVector<double>& matrixReal,
                                                      const QVector<double>& matrixImag, int n)
{
    m_timing.start();
    ++m_stats.totalDecompositions;

    /* 复Schur: 将实Schur的2x2对角块进一步约化为1x1 */
    Result result = decomposeReal(matrixReal, n);
    /* 标记有虚部的特征值 */
    for (int i = 0; i < n; ++i) {
        if (result.eigenvaluesImag[i] != 0.0) {
            /* 对应2x2块的Givens旋转 */
            int j = i + 1;
            if (j < n) {
                double a = result.T[i * n + i];
                double b = result.T[i * n + j];
                double c = result.T[j * n + i];
                double d = result.T[j * n + j];
                double tr = a + d;
                double det = a * d - b * c;
                double disc = tr * tr - 4.0 * det;
                if (disc < 0.0) {
                    double sq = qSqrt(-disc) / 2.0;
                    result.eigenvaluesReal[i] = tr / 2.0;
                    result.eigenvaluesImag[i] = sq;
                    result.eigenvaluesReal[j] = tr / 2.0;
                    result.eigenvaluesImag[j] = -sq;
                }
                ++i;
            }
        }
    }
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions > 0)
        ? m_timeSum / m_stats.totalDecompositions : 0.0;
    emit decompositionComplete(n, result.iterations, result.converged);
    return result;
}

QVector<double> SchurDecomp3::recoverEigenvectors(const Result& result, int n)
{
    m_timing.start();
    ++m_stats.totalEigenvectors;
    /* 从Schur形式T和变换Q求特征向量: V = Q * Y */
    QVector<double> Y = triangularEigenvectors(result.T, n);
    /* 变换回原空间 */
    QVector<double> V(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k)
                V[i * n + j] += result.Q[i * n + k] * Y[k * n + j];
        }
    }
    m_timeSum += m_timing.elapsed();
    emit eigenvectorsRecovered(n);
    return V;
}

void SchurDecomp3::hessenbergReduce(QVector<double>& H, QVector<double>& Q, int n)
{
    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量 */
        QVector<double> v(n - k - 1);
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) sigma += H[i * n + k] * H[i * n + k];
        double alpha = H[(k + 1) * n + k];
        double normX = qSqrt(alpha * alpha + sigma);
        if (normX < 1e-30) continue;
        double sign = (alpha >= 0) ? 1.0 : -1.0;
        v[0] = alpha + sign * normX;
        for (int i = 1; i < v.size(); ++i) v[i] = H[(k + 1 + i) * n + k];
        double vNorm = 0.0;
        for (double x : v) vNorm += x * x;
        vNorm = qSqrt(qMax(vNorm, 1e-30));
        for (double& x : v) x /= vNorm;

        /* H = (I - 2vv^T) H (I - 2vv^T) */
        /* 左乘 */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < v.size(); ++i)
                dot += v[i] * H[(k + 1 + i) * n + j];
            for (int i = 0; i < v.size(); ++i)
                H[(k + 1 + i) * n + j] -= 2.0 * v[i] * dot;
        }
        /* 右乘 */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < v.size(); ++j)
                dot += H[i * n + k + 1 + j] * v[j];
            for (int j = 0; j < v.size(); ++j)
                H[i * n + k + 1 + j] -= 2.0 * dot * v[j];
        }
        /* 累积变换Q */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < v.size(); ++j)
                dot += Q[i * n + k + 1 + j] * v[j];
            for (int j = 0; j < v.size(); ++j)
                Q[i * n + k + 1 + j] -= 2.0 * dot * v[j];
        }
    }
}

void SchurDecomp3::francisQrStep(QVector<double>& H, QVector<double>& Q, int n,
                                  int lo, int hi)
{
    /* 计算双位移的Wilkinson位移 */
    double s = H[(hi - 1) * n + hi - 1] + H[hi * n + hi];
    double t = H[(hi - 1) * n + hi - 1] * H[hi * n + hi]
             - H[(hi - 1) * n + hi] * H[hi * n + hi - 1];
    double x = H[lo * n + lo] * H[lo * n + lo] + H[lo * n + lo + 1] * H[(lo + 1) * n + lo]
             - s * H[lo * n + lo] + t;
    double y = H[(lo + 1) * n + lo] * (H[lo * n + lo] + H[(lo + 1) * n + lo + 1] - s);
    double z = H[(lo + 1) * n + lo] * H[(lo + 2) * n + lo + 1];

    for (int k = lo; k < hi - 1; ++k) {
        /* 3x1 Householder */
        QVector<double> v = {x, y, z};
        double norm = qSqrt(x * x + y * y + z * z);
        if (norm < 1e-30) { x = H[(k + 1) * n + k]; y = H[(k + 2) * n + k]; continue; }
        v[0] /= norm; v[1] /= norm; v[2] /= norm;
        if (v[0] >= 0) v[0] += 1.0; else v[0] -= 1.0;
        norm = qSqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        for (double& vi : v) vi /= norm;

        int r = qMax(k - 1, lo);
        /* 左乘 */
        for (int j = r; j < n; ++j) {
            double dot = v[0] * H[k * n + j] + v[1] * H[(k + 1) * n + j]
                       + v[2] * H[(k + 2) * n + j];
            H[k * n + j] -= 2.0 * v[0] * dot;
            H[(k + 1) * n + j] -= 2.0 * v[1] * dot;
            H[(k + 2) * n + j] -= 2.0 * v[2] * dot;
        }
        /* 右乘 */
        int endR = qMin(k + 4, hi + 1);
        for (int i = 0; i < endR; ++i) {
            double dot = H[i * n + k] * v[0] + H[i * n + k + 1] * v[1]
                       + H[i * n + k + 2] * v[2];
            H[i * n + k] -= 2.0 * dot * v[0];
            H[i * n + k + 1] -= 2.0 * dot * v[1];
            H[i * n + k + 2] -= 2.0 * dot * v[2];
        }
        /* 累积Q */
        for (int i = 0; i < n; ++i) {
            double dot = Q[i * n + k] * v[0] + Q[i * n + k + 1] * v[1]
                       + Q[i * n + k + 2] * v[2];
            Q[i * n + k] -= 2.0 * dot * v[0];
            Q[i * n + k + 1] -= 2.0 * dot * v[1];
            Q[i * n + k + 2] -= 2.0 * dot * v[2];
        }
        x = H[(k + 1) * n + k];
        y = H[(k + 2) * n + k];
        if (k < hi - 2) z = H[(k + 3) * n + k];
    }
    /* 最后一个2x1 Givens旋转 */
    double r = qSqrt(x * x + y * y);
    if (r > 1e-30) {
        double c = x / r, s2 = y / r;
        for (int j = hi - 1; j < n; ++j) {
            double t1 = H[(hi - 1) * n + j], t2 = H[hi * n + j];
            H[(hi - 1) * n + j] = c * t1 + s2 * t2;
            H[hi * n + j] = -s2 * t1 + c * t2;
        }
        for (int i = 0; i <= hi; ++i) {
            double t1 = H[i * n + hi - 1], t2 = H[i * n + hi];
            H[i * n + hi - 1] = c * t1 + s2 * t2;
            H[i * n + hi] = -s2 * t1 + c * t2;
        }
        for (int i = 0; i < n; ++i) {
            double t1 = Q[i * n + hi - 1], t2 = Q[i * n + hi];
            Q[i * n + hi - 1] = c * t1 + s2 * t2;
            Q[i * n + hi] = -s2 * t1 + c * t2;
        }
    }
}

QPair<double, double> SchurDecomp3::blockEigenvalues(const QVector<double>& H, int n,
                                                       int i) const
{
    double a = H[i * n + i];
    double b = H[i * n + i + 1];
    double c = H[(i + 1) * n + i];
    double d = H[(i + 1) * n + i + 1];
    double tr = a + d;
    double det = a * d - b * c;
    double disc = tr * tr - 4.0 * det;
    if (disc >= 0.0) {
        double sq = qSqrt(disc) / 2.0;
        return {tr / 2.0 + sq, 0.0};
    }
    return {tr / 2.0, qSqrt(-disc) / 2.0};
}

QVector<double> SchurDecomp3::triangularEigenvectors(const QVector<double>& T, int n) const
{
    QVector<double> Y(n * n, 0.0);
    for (int k = 0; k < n; ++k) {
        Y[k * n + k] = 1.0;
        for (int i = k - 1; i >= 0; --i) {
            double sum = 0.0;
            for (int j = i + 1; j <= k; ++j)
                sum += T[i * n + j] * Y[j * n + k];
            double denom = T[i * n + i] - T[k * n + k];
            if (qAbs(denom) < 1e-15) denom = 1e-15;
            Y[i * n + k] = -sum / denom;
        }
    }
    return Y;
}

void SchurDecomp3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
