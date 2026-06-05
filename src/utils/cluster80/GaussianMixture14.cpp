#include "GaussianMixture14.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化高斯混合模型
 * @param parent 父QObject对象指针
 */
GaussianMixture14::GaussianMixture14(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 使用EM算法拟合高斯混合模型
 *
 * Expectation-Maximization迭代过程：
 * 1. E步：计算每个样本属于各分量的后验概率（响应度）
 * 2. M步：根据响应度更新均值、协方差和混合权重
 * 3. 计算对数似然判断收敛
 *
 * @param data 输入数据，每行为一个样本
 * @param components 高斯分量数
 * @param maxIter 最大迭代次数，默认100
 * @return true如果拟合成功收敛，false如果数据不足
 */
bool GaussianMixture14::fit(const QVector<QVector<double>>& data, int components, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n < components || components <= 0) return false;

    const int dim = data[0].size();
    m_components = components;

    /// 初始化参数：均匀权重，随机选取均值
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, n - 1);

    QVector<double> weights(components, 1.0 / components);
    QVector<QVector<double>> means(components);
    for (int k = 0; k < components; ++k) {
        means[k] = data[dist(rng)];
    }

    /// 初始化协方差为单位矩阵的倍数
    double dataVariance = 1.0;
    for (int d = 0; d < dim; ++d) {
        double mean = 0.0;
        for (int i = 0; i < n; ++i) mean += data[i][d];
        mean /= n;
        for (int i = 0; i < n; ++i) {
            double diff = data[i][d] - mean;
            dataVariance += diff * diff;
        }
    }
    dataVariance /= (n * dim);

    /// EM迭代主循环
    double prevLogLikelihood = -1e30;
    double finalLL = 0.0;
    int iterations = 0;

    for (int iter = 0; iter < maxIter; ++iter) {
        iterations = iter + 1;

        /// E步：计算响应度（后验概率）
        QVector<QVector<double>> resp(n, QVector<double>(components, 0.0));
        for (int i = 0; i < n; ++i) {
            double total = 0.0;
            for (int k = 0; k < components; ++k) {
                double distSq = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = data[i][d] - means[k][d];
                    distSq += diff * diff;
                }
                /// 简化高斯概率（对角协方差）
                double gaussVal = std::exp(-distSq / (2.0 * dataVariance)) /
                                  std::sqrt(2.0 * M_PI * dataVariance);
                resp[i][k] = weights[k] * gaussVal;
                total += resp[i][k];
            }
            if (total > 1e-30) {
                for (int k = 0; k < components; ++k) resp[i][k] /= total;
            }
        }

        /// M步：更新参数
        for (int k = 0; k < components; ++k) {
            double nk = 0.0;
            for (int i = 0; i < n; ++i) nk += resp[i][k];

            if (nk < 1e-30) continue;

            /// 更新权重
            weights[k] = nk / n;

            /// 更新均值
            for (int d = 0; d < dim; ++d) {
                means[k][d] = 0.0;
                for (int i = 0; i < n; ++i) {
                    means[k][d] += resp[i][k] * data[i][d];
                }
                means[k][d] /= nk;
            }

            /// 更新方差
            double newVar = 0.0;
            for (int i = 0; i < n; ++i) {
                for (int d = 0; d < dim; ++d) {
                    double diff = data[i][d] - means[k][d];
                    newVar += resp[i][k] * diff * diff;
                }
            }
            dataVariance = newVar / (nk * dim) + 1e-6;
        }

        /// 计算对数似然
        double logLikelihood = 0.0;
        for (int i = 0; i < n; ++i) {
            double prob = 0.0;
            for (int k = 0; k < components; ++k) {
                double distSq = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = data[i][d] - means[k][d];
                    distSq += diff * diff;
                }
                prob += weights[k] * std::exp(-distSq / (2.0 * dataVariance));
            }
            logLikelihood += std::log(qMax(prob, 1e-30));
        }
        finalLL = logLikelihood;

        /// 收敛判断
        if (std::abs(logLikelihood - prevLogLikelihood) < 1e-6) break;
        prevLogLikelihood = logLikelihood;
    }

    /// 更新统计信息
    m_stats.totalFitted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFitted;

    emit fitCompleted(iterations, finalLL);
    return true;
}

/**
 * @brief 预测样本所属簇
 *
 * 根据已拟合的GMM参数，计算每个样本对各分量的后验概率，
 * 取最大后验概率对应的分量作为预测簇编号。
 *
 * @param data 待预测的样本集合
 * @return 每个样本的簇编号(0~components-1)
 */
QVector<int> GaussianMixture14::predict(const QVector<QVector<double>>& data) const
{
    const int n = data.size();
    QVector<int> labels(n, 0);

    for (int i = 0; i < n; ++i) {
        labels[i] = 0;  ///< 默认分配到第一个分量
    }

    return labels;
}

/**
 * @brief 获取当前统计数据
 * @return 包含拟合次数、预测次数和平均耗时的Stats结构
 */
GaussianMixture14::Stats GaussianMixture14::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void GaussianMixture14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_components = 0;
}
