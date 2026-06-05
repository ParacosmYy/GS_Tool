/**
 * @file KernelPca.cpp
 * @brief 核主成分分析实现 — RBF/多项式/线性核函数
 */

#include "utils/pca2/KernelPca.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
KernelPca::KernelPca(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置核函数类型 @param type 核类型 */
void KernelPca::setKernelType(KernelType type)
{
    m_kernelType = type;
}

/** @brief 设置核函数参数
 *  @param gamma RBF的gamma参数
 *  @param degree 多项式的阶数
 *  @param coef0 多项式的常数项 */
void KernelPca::setKernelParameters(double gamma, int degree, double coef0)
{
    m_gamma = qMax(1e-10, gamma);
    m_degree = qMax(1, degree);
    m_coef0 = coef0;
}

/** @brief 设置目标主成分数 @param components 主成分数 */
void KernelPca::setComponentCount(int components)
{
    m_components = qMax(0, components);
}

/** @brief 设置方差保留比例 @param ratio 保留比例 */
void KernelPca::setVarianceRatio(double ratio)
{
    m_varianceRatio = qBound(0.0, ratio, 1.0);
}

/** @brief 拟合模型
 *  @param data 输入数据(每行一个样本) */
void KernelPca::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return;

    m_trainingData = data;

    /* 计算核矩阵 */
    QVector<QVector<double>> K = computeKernelMatrix(data);

    /* 中心化核矩阵 */
    QVector<QVector<double>> Kc = centerKernelMatrix(K);

    /* 确定主成分数 */
    int numComp = m_components;
    if (numComp <= 0 || numComp > n) {
        numComp = n;
    }

    /* 特征分解 */
    m_eigenvalues.clear();
    m_eigenvectors.clear();
    eigenDecompose(Kc, m_eigenvalues, m_eigenvectors, numComp);

    /* 自动选择主成分数 */
    if (m_components <= 0) {
        double totalVar = 0.0;
        for (double ev : m_eigenvalues) {
            if (ev > 0) totalVar += ev;
        }

        if (totalVar > 0) {
            double cumVar = 0.0;
            numComp = 0;
            for (int i = 0; i < m_eigenvalues.size(); ++i) {
                if (m_eigenvalues[i] > 0) {
                    cumVar += m_eigenvalues[i];
                    ++numComp;
                    if (cumVar / totalVar >= m_varianceRatio) break;
                }
            }
            numComp = qMax(1, numComp);
        } else {
            numComp = 1;
        }
    }

    /* 截断到选定的主成分数 */
    if (numComp < m_eigenvalues.size()) {
        m_eigenvalues.resize(numComp);
        m_eigenvectors.resize(numComp);
    }

    /* 归一化特征向量: alpha = v / sqrt(lambda) */
    for (int i = 0; i < m_eigenvectors.size(); ++i) {
        double ev = m_eigenvalues[i];
        if (ev > 1e-10) {
            double scale = 1.0 / qSqrt(ev);
            for (double& val : m_eigenvectors[i]) {
                val *= scale;
            }
        }
    }

    /* 保存核矩阵列均值用于后续变换 */
    m_kColMean.resize(n);
    m_kMean = 0.0;
    for (int j = 0; j < n; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i) {
            colSum += K[i][j];
        }
        m_kColMean[j] = colSum / n;
        m_kMean += colSum;
    }
    m_kMean /= (n * n);

    m_fitted = true;

    ++m_stats.totalFits;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFits);

    double variance = 0.0;
    if (!m_eigenvalues.isEmpty() && m_eigenvalues[0] > 0) {
        double total = 0.0;
        for (double ev : m_eigenvalues) {
            if (ev > 0) total += ev;
        }
        variance = (total > 0) ? m_eigenvalues[0] / total : 0.0;
    }

    emit fitCompleted(m_eigenvalues.size(), variance);
}

/** @brief 变换数据
 *  @param data 输入数据
 *  @return 变换结果 */
KernelPca::TransformResult KernelPca::transform(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    TransformResult result;
    if (!m_fitted || data.isEmpty()) return result;

    int n = data.size();
    int nTrain = m_trainingData.size();
    int numComp = m_eigenvectors.size();

    /* 计算新数据与训练数据的核矩阵 K* (n x nTrain) */
    QVector<QVector<double>> Kstar(n, QVector<double>(nTrain, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < nTrain; ++j) {
            Kstar[i][j] = kernelValue(data[i], m_trainingData[j]);
        }
    }

    /* 中心化并投影 */
    result.projectedData.resize(n);
    for (int i = 0; i < n; ++i) {
        result.projectedData[i].resize(numComp, 0.0);
        for (int k = 0; k < numComp; ++k) {
            double sum = 0.0;
            for (int j = 0; j < nTrain; ++j) {
                double centered = Kstar[i][j] - m_kColMean[j];
                sum += m_eigenvectors[k][j] * centered;
            }
            result.projectedData[i][k] = sum;
        }
    }

    result.eigenvalues = m_eigenvalues;
    result.eigenvectors = m_eigenvectors;
    result.componentsUsed = numComp;

    ++m_stats.totalTransforms;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);

    emit transformCompleted(n, numComp);
    return result;
}

/** @brief 拟合并变换
 *  @param data 输入数据
 *  @return 变换结果 */
KernelPca::TransformResult KernelPca::fitTransform(
    const QVector<QVector<double>>& data)
{
    fit(data);
    return transform(data);
}

/** @brief 变换单个新样本
 *  @param sample 样本向量
 *  @return 投影坐标 */
QVector<double> KernelPca::transformSample(
    const QVector<double>& sample) const
{
    if (!m_fitted) return {};

    QVector<QVector<double>> single = {sample};
    TransformResult result = transform(single);
    if (result.projectedData.isEmpty()) return {};
    return result.projectedData[0];
}

/** @brief 计算核矩阵
 *  @param data 输入数据
 *  @return 核矩阵(n x n) */
QVector<QVector<double>> KernelPca::computeKernelMatrix(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> K(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            double val = kernelValue(data[i], data[j]);
            K[i][j] = val;
            K[j][i] = val; /* 核矩阵对称 */
        }
    }
    return K;
}

/** @brief 获取解释方差比 */
QVector<double> KernelPca::explainedVarianceRatio() const
{
    if (m_eigenvalues.isEmpty()) return {};

    double total = 0.0;
    for (double ev : m_eigenvalues) {
        if (ev > 0) total += ev;
    }

    QVector<double> ratios;
    if (total > 0) {
        for (double ev : m_eigenvalues) {
            ratios.append((ev > 0) ? ev / total : 0.0);
        }
    }
    return ratios;
}

/** @brief 模型是否已拟合 */
bool KernelPca::isFitted() const
{
    return m_fitted;
}

/** @brief 获取统计信息 */
KernelPca::Stats KernelPca::stats() const
{
    return m_stats;
}

/** @brief 重置统计 */
void KernelPca::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算两个向量的核函数值 */
double KernelPca::kernelValue(const QVector<double>& a,
                              const QVector<double>& b) const
{
    int dim = qMin(a.size(), b.size());

    switch (m_kernelType) {
    case KernelType::RBF: {
        /* K(x,y) = exp(-gamma * ||x-y||^2) */
        double dist2 = 0.0;
        for (int i = 0; i < dim; ++i) {
            double diff = a[i] - b[i];
            dist2 += diff * diff;
        }
        return qExp(-m_gamma * dist2);
    }
    case KernelType::Polynomial: {
        /* K(x,y) = (gamma * x.y + coef0)^degree */
        double dot = 0.0;
        for (int i = 0; i < dim; ++i) {
            dot += a[i] * b[i];
        }
        return qPow(m_gamma * dot + m_coef0, m_degree);
    }
    case KernelType::Linear: {
        /* K(x,y) = x.y */
        double dot = 0.0;
        for (int i = 0; i < dim; ++i) {
            dot += a[i] * b[i];
        }
        return dot;
    }
    }
    return 0.0;
}

/** @brief 特征值分解(幂迭代+Deflation)
 *  @param matrix 对称矩阵
 *  @param eigenvalues 输出特征值
 *  @param eigenvectors 输出特征向量
 *  @param numComponents 分解的分量数 */
void KernelPca::eigenDecompose(
    QVector<QVector<double>>& matrix,
    QVector<double>& eigenvalues,
    QVector<QVector<double>>& eigenvectors,
    int numComponents) const
{
    int n = matrix.size();
    if (n == 0) return;

    numComponents = qMin(numComponents, n);

    for (int k = 0; k < numComponents; ++k) {
        /* 幂迭代求第k大特征向量 */
        QVector<double> v(n, 0.0);
        v[k % n] = 1.0; /* 初始向量 */

        double eigenvalue = 0.0;

        for (int iter = 0; iter < 200; ++iter) {
            /* 矩阵-向量乘: w = K * v */
            QVector<double> w(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    w[i] += matrix[i][j] * v[j];
                }
            }

            /* 计算范数并归一化 */
            double norm2 = 0.0;
            for (int i = 0; i < n; ++i) {
                norm2 += w[i] * w[i];
            }
            double norm = qSqrt(qMax(norm2, 1e-30));

            for (int i = 0; i < n; ++i) {
                v[i] = w[i] / norm;
            }

            /* 更新特征值估计(Rayleigh商) */
            double newEigenvalue = 0.0;
            for (int i = 0; i < n; ++i) {
                newEigenvalue += v[i] * w[i];
            }

            if (qAbs(newEigenvalue - eigenvalue) < 1e-8) {
                eigenvalue = newEigenvalue;
                break;
            }
            eigenvalue = newEigenvalue;
        }

        eigenvalues.append(eigenvalue);
        eigenvectors.append(v);

        /* Deflation: 移除此特征向量的贡献 */
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                matrix[i][j] -= eigenvalue * v[i] * v[j];
            }
        }
    }
}

/** @brief 中心化核矩阵 */
QVector<QVector<double>> KernelPca::centerKernelMatrix(
    const QVector<QVector<double>>& K) const
{
    int n = K.size();
    if (n == 0) return K;

    /* 计算行均值/列均值/总均值 */
    QVector<double> rowMean(n, 0.0);
    QVector<double> colMean(n, 0.0);
    double totalMean = 0.0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            rowMean[i] += K[i][j];
            colMean[j] += K[i][j];
            totalMean += K[i][j];
        }
    }

    for (int i = 0; i < n; ++i) {
        rowMean[i] /= n;
        colMean[i] /= n;
    }
    totalMean /= (n * n);

    /* K_c = K - 1*colMean^T - rowMean*1^T + totalMean */
    QVector<QVector<double>> Kc(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            Kc[i][j] = K[i][j] - rowMean[i] - colMean[j] + totalMean;
        }
    }

    return Kc;
}
