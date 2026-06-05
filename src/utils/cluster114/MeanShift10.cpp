#include "MeanShift10.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化均值漂移聚类引擎
 * @param parent 父对象指针
 */
MeanShift10::MeanShift10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void MeanShift10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行均值漂移聚类
 *
 * 对每个数据点执行均值漂移迭代直到收敛到密度极大值点（模式），
 * 然后将距离小于带宽的收敛模式合并为同一簇。
 *
 * @param dataPoints 输入数据点集合 (n×d)
 * @param bandwidth 核带宽参数（若<=0则自动估计）
 * @return 各数据点的聚类标签
 */
QVector<int> MeanShift10::fit(const QVector<QVector<double>>& dataPoints, double bandwidth)
{
    QElapsedTimer timer;
    timer.start();

    const int n = dataPoints.size();
    QVector<int> labels(n, -1);
    if (n == 0) {
        emit clusteringCompleted(0);
        return labels;
    }

    /* 自动估计带宽 */
    double bw = bandwidth;
    if (bw <= 0.0) bw = estimateBandwidth(dataPoints);
    if (bw < 1e-15) bw = 1.0;

    /* 1. 对每个点执行均值漂移 */
    QVector<QVector<double>> modes(n);
    for (int i = 0; i < n; ++i)
        modes[i] = shiftPoint(dataPoints[i], dataPoints, bw, 100);

    /* 2. 合并距离小于带宽的相近模式 */
    QVector<int> modeLabels(n, -1);
    QVector<QVector<double>> uniqueModes;
    double mergeThreshold = bw * 0.5;

    for (int i = 0; i < n; ++i) {
        bool merged = false;
        for (int m = 0; m < uniqueModes.size(); ++m) {
            double dist = 0.0;
            int dims = qMin(modes[i].size(), uniqueModes[m].size());
            for (int d = 0; d < dims; ++d) {
                double diff = modes[i][d] - uniqueModes[m][d];
                dist += diff * diff;
            }
            dist = qSqrt(dist);
            if (dist < mergeThreshold) {
                modeLabels[i] = m;
                merged = true;
                break;
            }
        }
        if (!merged) {
            modeLabels[i] = uniqueModes.size();
            uniqueModes.append(modes[i]);
        }
    }

    labels = modeLabels;

    m_stats.totalShiftOps += n;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalShiftOps > 0
                                                 ? m_stats.totalShiftOps : 1);

    emit clusteringCompleted(uniqueModes.size());
    return labels;
}

/**
 * @brief 单点均值漂移迭代至收敛
 *
 * 使用核函数加权计算邻域内数据点的加权均值，
 * 迭代移动当前点直到位移小于收敛阈值或达到最大迭代次数。
 *
 * @param point 起始点坐标
 * @param dataPoints 所有数据点集合
 * @param bandwidth 核带宽参数
 * @param maxIterations 最大迭代次数
 * @return 收敛后的密度极大值点坐标
 */
QVector<double> MeanShift10::shiftPoint(const QVector<double>& point,
                                        const QVector<QVector<double>>& dataPoints,
                                        double bandwidth, int maxIterations)
{
    const int n = dataPoints.size();
    if (n == 0 || point.isEmpty()) return point;

    const int dims = point.size();
    double bw2 = bandwidth * bandwidth;
    if (bw2 < 1e-30) bw2 = 1.0;
    const double convergenceThreshold = 1e-5 * bandwidth;

    QVector<double> current = point;

    for (int iter = 0; iter < maxIterations; ++iter) {
        QVector<double> numerator(dims, 0.0);
        double denominator = 0.0;

        for (int i = 0; i < n; ++i) {
            /* 计算当前点到数据点i的距离平方 */
            double sqDist = 0.0;
            int dMax = qMin(dims, dataPoints[i].size());
            for (int d = 0; d < dMax; ++d) {
                double diff = current[d] - dataPoints[i][d];
                sqDist += diff * diff;
            }

            /* 高斯核权重 */
            double weight = qExp(-sqDist / (2.0 * bw2));
            if (weight < 1e-10) continue;

            for (int d = 0; d < dMax; ++d)
                numerator[d] += weight * dataPoints[i][d];
            denominator += weight;
        }

        if (denominator < 1e-15) break;

        /* 计算新位置 */
        QVector<double> shifted(dims, 0.0);
        for (int d = 0; d < dims; ++d)
            shifted[d] = numerator[d] / denominator;

        /* 检查收敛 */
        double shift = 0.0;
        for (int d = 0; d < dims; ++d) {
            double diff = shifted[d] - current[d];
            shift += diff * diff;
        }
        shift = qSqrt(shift);

        current = shifted;
        if (shift < convergenceThreshold) break;
    }

    return current;
}

/**
 * @brief 自动估计最佳带宽（Silverman法则）
 *
 * 带宽 h = A * n^{-1/(d+4)} * sigma
 * 其中 A = (4/(d+2))^{1/(d+4)}, sigma为标准差,
 * n为样本数, d为维度。
 *
 * @param dataPoints 数据点集合
 * @return 建议的带宽值
 */
double MeanShift10::estimateBandwidth(const QVector<QVector<double>>& dataPoints)
{
    const int n = dataPoints.size();
    if (n == 0) return 1.0;

    int dims = 0;
    for (int i = 0; i < n; ++i)
        dims = qMax(dims, dataPoints[i].size());
    if (dims == 0) return 1.0;

    /* 计算每个维度的均值 */
    QVector<double> mean(dims, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int d = 0; d < qMin(dims, dataPoints[i].size()); ++d)
            mean[d] += dataPoints[i][d];
    }
    for (int d = 0; d < dims; ++d)
        mean[d] /= n;

    /* 计算各维度标准差并取平均 */
    double avgStd = 0.0;
    for (int d = 0; d < dims; ++d) {
        double variance = 0.0;
        for (int i = 0; i < n; ++i) {
            double val = (d < dataPoints[i].size()) ? dataPoints[i][d] : 0.0;
            double diff = val - mean[d];
            variance += diff * diff;
        }
        variance /= n;
        avgStd += qSqrt(variance);
    }
    avgStd /= dims;

    if (avgStd < 1e-15) avgStd = 1.0;

    /* Silverman法则: h = A * n^{-1/(d+4)} * sigma */
    double exponent = 1.0 / (dims + 4.0);
    double A = qPow(4.0 / (dims + 2.0), exponent);
    double h = A * qPow(static_cast<double>(n), -exponent) * avgStd;

    return qMax(h, 1e-10);
}

/**
 * @brief 设置核函数类型
 *
 * 设置均值漂移使用的核函数，影响shiftPoint中的权重计算。
 * 注意：当前shiftPoint固定使用高斯核，此方法预留扩展接口。
 *
 * @param kernelType 核类型名称 (gaussian/epanechnikov/uniform)
 */
void MeanShift10::setKernel(const QString& kernelType)
{
    Q_UNUSED(kernelType)
    /* 预留：当前实现固定使用高斯核。
     * 后续可扩展支持Epanechnikov核和均匀核，
     * 通过成员变量m_kernelType在shiftPoint中分支处理。 */
}
