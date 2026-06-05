/**
 * @file KernelPca.cpp
 * @brief 核主成分分析实现 — 非线性降维
 */

#include "utils/pca2/KernelPca.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
KernelPca::KernelPca(QObject* parent)
    : QObject(parent)
{
}

/** @brief 拟合核PCA模型 */
int KernelPca::fit(const QVector<QVector<double>>& data,
                   Kernel kernel,
                   const KernelParams& params,
                   int nComponents)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return 0;

    m_trainData = data;
    m_kernel = kernel;
    m_params = params;

    int maxComp = qMin(n, 10);
    m_nComponents = (nComponents > 0) ? qMin(nComponents, maxComp) : maxComp;

    /* 构建核矩阵 */
    QVector<QVector<double>> K(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            double kv = kernelValue(data[i], data[j]);
            K[i][j] = kv;
            K[j][i] = kv;
        }
    }

    /* 中心化核矩阵 */
    QVector<double> rowMean(n, 0.0);
    double totalMean = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            rowMean[i] += K[i][j];
            totalMean += K[i][j];
        }
        rowMean[i] /= n;
    }
    totalMean /= (n * n);

    QVector<double> colMean(n, 0.0);
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) colMean[j] += K[i][j];
        colMean[j] /= n;
    }

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            K[i][j] -= rowMean[i] + colMean[j] - totalMean;

    /* 幂迭代提取主成分 */
    m_alphas.clear();
    m_eigenvalues.clear();

    QVector<QVector<double>> deflatedK = K;
    for (int c = 0; c < m_nComponents; ++c) {
        QVector<double> alpha = powerIteration(deflatedK, 100);
        double eigenval = 0.0;
        for (int i = 0; i < n; ++i)
            eigenval += alpha[i] * deflatedK[i][i] * alpha[i];
        eigenval = qMax(eigenval, 1e-10);

        double norm = 0.0;
        for (double v : alpha) norm += v * v;
        norm = std::sqrt(norm);
        if (norm > 1e-15)
            for (auto& v : alpha) v /= norm;

        m_alphas.append(alpha);
        m_eigenvalues.append(eigenval);

        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                deflatedK[i][j] -= eigenval * alpha[i] * alpha[j];
    }

    m_timeSum += timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_nComponents);
    return m_nComponents;
}

/** @brief 变换单个样本 */
QVector<double> KernelPca::transform(const QVector<double>& sample) const
{
    int n = m_trainData.size();
    if (n == 0 || m_nComponents == 0) return {};

    QVector<double> kvec(n);
    for (int i = 0; i < n; ++i)
        kvec[i] = kernelValue(sample, m_trainData[i]);

    QVector<double> result(m_nComponents, 0.0);
    for (int c = 0; c < m_nComponents; ++c)
        for (int i = 0; i < n; ++i)
            result[c] += m_alphas[c][i] * kvec[i];
    return result;
}

/** @brief 批量变换 */
QVector<QVector<double>> KernelPca::transformBatch(
    const QVector<QVector<double>>& data) const
{
    QVector<QVector<double>> results;
    results.reserve(data.size());
    for (const auto& sample : data)
        results.append(transform(sample));
    return results;
}

/** @brief 拟合核PCA(使用默认参数) */
int KernelPca::fit(const QVector<QVector<double>>& data, int nComponents)
{
    return fit(data, Kernel::Rbf, KernelParams{}, nComponents);
}

/** @brief 重置统计 */
void KernelPca::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算核函数值 */
double KernelPca::kernelValue(const QVector<double>& x,
                               const QVector<double>& y) const
{
    int d = qMin(x.size(), y.size());
    if (d == 0) return 0.0;

    switch (m_kernel) {
    case Kernel::Rbf: {
        double sqDist = 0.0;
        for (int i = 0; i < d; ++i) {
            double diff = x[i] - y[i];
            sqDist += diff * diff;
        }
        return std::exp(-m_params.gamma * sqDist);
    }
    case Kernel::Polynomial: {
        double dot = 0.0;
        for (int i = 0; i < d; ++i) dot += x[i] * y[i];
        return std::pow(dot + m_params.coef0, m_params.degree);
    }
    case Kernel::Linear: {
        double dot = 0.0;
        for (int i = 0; i < d; ++i) dot += x[i] * y[i];
        return dot;
    }
    }
    return 0.0;
}

/** @brief 幂迭代求最大特征向量 */
QVector<double> KernelPca::powerIteration(
    const QVector<QVector<double>>& matrix, int maxIter) const
{
    int n = matrix.size();
    if (n == 0) return {};

    QVector<double> v(n, 1.0 / std::sqrt(static_cast<double>(n)));

    for (int iter = 0; iter < maxIter; ++iter) {
        QVector<double> vNew(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                vNew[i] += matrix[i][j] * v[j];

        double norm = 0.0;
        for (double val : vNew) norm += val * val;
        norm = std::sqrt(norm);
        if (norm < 1e-15) break;
        for (auto& val : vNew) val /= norm;

        double diff = 0.0;
        for (int i = 0; i < n; ++i)
            diff += (vNew[i] - v[i]) * (vNew[i] - v[i]);
        v = vNew;
        if (diff < 1e-12) break;
    }
    return v;
}
