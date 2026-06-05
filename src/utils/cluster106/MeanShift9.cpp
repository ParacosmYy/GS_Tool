#include "MeanShift9.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Mean Shift聚类引擎
 * @param parent 父对象指针
 */
MeanShift9::MeanShift9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void MeanShift9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 自动估计最佳带宽
 *
 * 使用Silverman规则：bandwidth = 1.06 * std * n^{-1/(d+4)}
 *
 * @param data 输入数据点
 * @return 建议的带宽值
 */
double MeanShift9::estimateBandwidth(const QVector<QVector<double>>& data) const
{
    const int n = data.size();
    if (n == 0) return 1.0;

    const int dims = data[0].size();

    /* 计算各维度标准差 */
    QVector<double> mean(dims, 0.0);
    for (const auto& pt : data) {
        for (int j = 0; j < dims && j < pt.size(); ++j) {
            mean[j] += pt[j];
        }
    }
    for (int j = 0; j < dims; ++j) mean[j] /= n;

    double totalStd = 0.0;
    for (const auto& pt : data) {
        for (int j = 0; j < dims && j < pt.size(); ++j) {
            double diff = pt[j] - mean[j];
            totalStd += diff * diff;
        }
    }
    totalStd = qSqrt(totalStd / (n * dims));

    return 1.06 * totalStd * qPow(n, -1.0 / (dims + 4));
}

/**
 * @brief 高斯核函数
 * @param distance 距离值
 * @param bandwidth 带宽
 * @return 核权重
 */
static double gaussianKernel(double distance, double bandwidth)
{
    return qExp(-0.5 * distance * distance / (bandwidth * bandwidth));
}

/**
 * @brief 执行Mean Shift聚类
 *
 * 对每个数据点执行均值漂移迭代：
 * x_{t+1} = Σ K(x-x_i) * x_i / Σ K(x-x_i)
 * 直到收敛，收敛到同一模态的点归为同一簇。
 *
 * @param data 输入数据点
 * @param bandwidth 核带宽参数
 * @return 每个样本的簇标签
 */
QVector<int> MeanShift9::fit(const QVector<QVector<double>>& data, double bandwidth)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) {
        emit clusteringCompleted(0);
        return {};
    }

    const int dims = data[0].size();
    m_bandwidth = qMax(1e-10, bandwidth);

    /* 步骤1：对每个点执行均值漂移 */
    QVector<QVector<double>> modes = data; /* 初始化为数据点本身 */
    const int maxIter = 100;
    const double convergenceThreshold = m_bandwidth * 0.01;

    for (int i = 0; i < n; ++i) {
        for (int iter = 0; iter < maxIter; ++iter) {
            QVector<double> newMode(dims, 0.0);
            double totalWeight = 0.0;

            for (int j = 0; j < n; ++j) {
                double dist = 0.0;
                for (int d = 0; d < dims && d < data[j].size(); ++d) {
                    double diff = modes[i][d] - data[j][d];
                    dist += diff * diff;
                }
                dist = qSqrt(dist);

                double weight = gaussianKernel(dist, m_bandwidth);
                totalWeight += weight;
                for (int d = 0; d < dims && d < data[j].size(); ++d) {
                    newMode[d] += weight * data[j][d];
                }
            }

            if (totalWeight > 0) {
                for (int d = 0; d < dims; ++d) {
                    newMode[d] /= totalWeight;
                }
            }

            /* 收敛检查 */
            double shift = 0.0;
            for (int d = 0; d < dims; ++d) {
                double diff = newMode[d] - modes[i][d];
                shift += diff * diff;
            }
            shift = qSqrt(shift);

            modes[i] = newMode;
            if (shift < convergenceThreshold) break;
        }
    }

    /* 步骤2：合并相近的模态（聚类） */
    QVector<int> labels(n, -1);
    m_centers.clear();
    double mergeThreshold = m_bandwidth * 0.5;

    for (int i = 0; i < n; ++i) {
        if (labels[i] >= 0) continue;

        /* 找到所有与此模态相近的点 */
        int newLabel = m_centers.size();
        m_centers.append(modes[i]);
        labels[i] = newLabel;

        for (int j = i + 1; j < n; ++j) {
            if (labels[j] >= 0) continue;
            double dist = 0.0;
            for (int d = 0; d < dims && d < modes[i].size(); ++d) {
                double diff = modes[i][d] - modes[j][d];
                dist += diff * diff;
            }
            if (qSqrt(dist) < mergeThreshold) {
                labels[j] = newLabel;
            }
        }
    }

    m_stats.clustersFound = m_centers.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusteringRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusteringRuns;

    emit clusteringCompleted(m_centers.size());
    return labels;
}

/**
 * @brief 预测新样本所属簇
 * @param sample 输入样本
 * @return 簇标签
 */
int MeanShift9::predict(const QVector<double>& sample) const
{
    if (m_centers.isEmpty()) return -1;

    double bestDist = 1e18;
    int bestLabel = 0;
    for (int c = 0; c < m_centers.size(); ++c) {
        double d = 0.0;
        for (int j = 0; j < sample.size() && j < m_centers[c].size(); ++j) {
            double diff = sample[j] - m_centers[c][j];
            d += diff * diff;
        }
        if (d < bestDist) {
            bestDist = d;
            bestLabel = c;
        }
    }
    return bestLabel;
}
