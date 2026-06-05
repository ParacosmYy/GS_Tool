#include "SpectralClustering6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化谱聚类器
 * @param parent 父对象指针
 */
SpectralClustering6::SpectralClustering6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置聚类簇数量
 * @param count 目标簇数量
 */
void SpectralClustering6::setClusterCount(int count)
{
    m_clusterCount = qMax(1, count);
}

/**
 * @brief 设置RBF核参数sigma
 * @param sigma 高斯核带宽参数，值越小聚类边界越紧
 */
void SpectralClustering6::setSigma(double sigma)
{
    m_sigma = qMax(0.01, sigma);
}

/**
 * @brief 对输入数据集执行谱聚类
 *
 * 1. 计算RBF相似度矩阵 W
 * 2. 构建归一化拉普拉斯矩阵 L = D^{-1/2} W D^{-1/2}
 * 3. 提取前k个特征向量
 * 4. 在特征向量空间执行K-Means聚类
 *
 * @param data 输入数据集
 */
void SpectralClustering6::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalClusterings++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
        emit clusteringCompleted(0);
        return;
    }

    const int n = data.size();
    const int k = qMin(m_clusterCount, n);

    /* 第一步：计算RBF相似度矩阵 */
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            double dist = 0.0;
            int dim = qMin(data[i].size(), data[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - data[j][d];
                dist += diff * diff;
            }
            double sim = std::exp(-dist / (2.0 * m_sigma * m_sigma));
            W[i][j] = sim;
            W[j][i] = sim;
        }
    }

    /* 第二步：计算度矩阵D和归一化拉普拉斯 */
    QVector<double> D(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) D[i] += W[i][j];
    }

    /* L_norm = D^{-1/2} W D^{-1/2} */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double di = (D[i] > 1e-15) ? 1.0 / std::sqrt(D[i]) : 0.0;
            double dj = (D[j] > 1e-15) ? 1.0 / std::sqrt(D[j]) : 0.0;
            L[i][j] = di * W[i][j] * dj;
        }
    }

    /* 第三步：幂迭代法提取前k个特征向量 */
    QVector<QVector<double>> features(n, QVector<double>(k, 0.0));
    for (int ev = 0; ev < k; ++ev) {
        QVector<double> v(n, 1.0 / std::sqrt(n));
        for (int iter = 0; iter < 200; ++iter) {
            /* 矩阵乘法 */
            QVector<double> newV(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) newV[i] += L[i][j] * v[j];
            }
            /* 减去之前特征向量分量(正交化) */
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += newV[i] * features[i][prev];
                for (int i = 0; i < n; ++i) newV[i] -= dot * features[i][prev];
            }
            /* 归一化 */
            double norm = 0.0;
            for (double val : newV) norm += val * val;
            norm = std::sqrt(norm);
            if (norm > 1e-15) {
                for (int i = 0; i < n; ++i) v[i] = newV[i] / norm;
            }
        }
        for (int i = 0; i < n; ++i) features[i][ev] = v[i];
    }

    /* 第四步：在特征空间执行简单K-Means */
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestDist = 1e18;
        for (int c = 0; c < k; ++c) {
            double d = 0.0;
            for (int j = 0; j < k; ++j) {
                double diff = features[i][j] - (c * 1.0 / k);
                d += diff * diff;
            }
            if (d < bestDist) { bestDist = d; labels[i] = c; }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
    emit clusteringCompleted(n);
}

/**
 * @brief 重置统计数据
 */
void SpectralClustering6::resetStatistics()
{
    m_stats.totalClusterings = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
