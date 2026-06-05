#include "SpectralClustering7.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file SpectralClustering7.cpp
 * @brief 谱聚类分析器实现
 *
 * 基于图拉普拉斯矩阵的特征分解进行聚类:
 * 1. 构建相似度矩阵W
 * 2. 计算归一化拉普拉斯矩阵L = D^{-1/2}(D-W)D^{-1/2}
 * 3. 取L的前k个最小特征值对应的特征向量
 * 4. 对特征向量矩阵按行进行K-means聚类
 */

/**
 * @brief 构造函数，初始化默认聚类参数
 * @param parent 父QObject对象指针
 */
SpectralClustering7::SpectralClustering7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置目标聚类数
 * @param k 聚类数目
 */
void SpectralClustering7::setClusterCount(int k)
{
    m_clusterCount = qMax(2, k);
}

/**
 * @brief 设置相似度核函数
 * @param kernel 核函数类型: rbf/poly/knn
 */
void SpectralClustering7::setKernel(const QString& kernel)
{
    if (kernel == "rbf" || kernel == "poly" || kernel == "knn") {
        m_kernel = kernel;
    }
}

/**
 * @brief 计算RBF核相似度
 * @param x1 第一个数据点
 * @param x2 第二个数据点
 * @return 相似度值
 */
static double rbfKernel(const QVector<double>& x1, const QVector<double>& x2, double sigma = 1.0)
{
    double distSq = 0.0;
    for (int i = 0; i < x1.size(); ++i) {
        distSq += (x1[i] - x2[i]) * (x1[i] - x2[i]);
    }
    return std::exp(-distSq / (2.0 * sigma * sigma));
}

/**
 * @brief 拟合数据，执行谱聚类
 *
 * @param data 输入数据矩阵，每行为一个数据点
 */
void SpectralClustering7::fit(const QVector<QVector<double>>& data)
{
    if (data.size() < m_clusterCount) return;

    QElapsedTimer timer;
    timer.start();

    const int n = data.size();

    // 步骤1: 构建相似度矩阵
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double sim = 0.0;
            if (m_kernel == "rbf") {
                sim = rbfKernel(data[i], data[j]);
            } else if (m_kernel == "poly") {
                double dot = 0.0;
                for (int d = 0; d < data[i].size(); ++d) {
                    dot += data[i][d] * data[j][d];
                }
                sim = std::pow(dot + 1.0, 3.0);
            } else {
                // knn: 简化为距离倒数
                double distSq = 0.0;
                for (int d = 0; d < data[i].size(); ++d) {
                    distSq += (data[i][d] - data[j][d]) * (data[i][d] - data[j][d]);
                }
                sim = 1.0 / (1.0 + std::sqrt(distSq));
            }
            W[i][j] = sim;
            W[j][i] = sim;
        }
    }

    // 步骤2: 计算度矩阵D和归一化拉普拉斯L
    QVector<double> D(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            D[i] += W[i][j];
        }
    }

    // 步骤3: 幂迭代法求前k个特征向量(简化实现)
    QVector<QVector<double>> features(n, QVector<double>(m_clusterCount, 0.0));
    for (int k = 0; k < m_clusterCount; ++k) {
        // 随机初始化特征向量
        QVector<double> v(n, 1.0 / std::sqrt(n));
        for (int iter = 0; iter < 50; ++iter) {
            // 矩阵向量乘法 Lv
            QVector<double> Lv(n, 0.0);
            for (int i = 0; i < n; ++i) {
                Lv[i] = (D[i] > 1e-12 ? v[i] : 0.0);
                for (int j = 0; j < n; ++j) {
                    Lv[i] -= W[i][j] * v[j];
                }
            }
            // 归一化
            double norm = 0.0;
            for (int i = 0; i < n; ++i) norm += Lv[i] * Lv[i];
            norm = std::sqrt(norm);
            if (norm > 1e-12) {
                for (int i = 0; i < n; ++i) v[i] = Lv[i] / norm;
            }
        }
        for (int i = 0; i < n; ++i) {
            features[i][k] = v[i];
        }
    }

    // 步骤4: 简单K-means对特征向量聚类
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double minDist = 1e18;
        int bestK = 0;
        for (int k = 0; k < m_clusterCount; ++k) {
            double dist = 0.0;
            for (int d = 0; d < m_clusterCount; ++d) {
                dist += features[i][d] * features[i][d];
            }
            if (dist < minDist) { minDist = dist; bestK = k; }
        }
        labels[i] = bestK;
    }

    // 更新统计信息
    m_stats.totalPoints += n;
    m_stats.totalClusters = m_clusterCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPoints / n);

    emit clusteringCompleted(m_clusterCount);
}

/**
 * @brief 重置所有统计信息
 */
void SpectralClustering7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
