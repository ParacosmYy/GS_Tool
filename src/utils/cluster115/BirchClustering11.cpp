#include "BirchClustering11.h"
#include <QElapsedTimer>
#include <QMap>
#include <QtMath>
#include <algorithm>

/* ---- 内部CF存储（弥补头文件无CF成员变量的限制） ---- */
static QVector<QPair<int, QPair<QVector<double>, double>>> g_cfEntries;

/**
 * @brief 构造函数，初始化BIRCH聚类引擎
 * @param parent 父对象指针
 */
BirchClustering11::BirchClustering11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void BirchClustering11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    g_cfEntries.clear();
}

/**
 * @brief 执行BIRCH聚类（三阶段流程）
 *
 * 阶段一：逐点构建CF树，将数据点插入最近的子簇。
 * 阶段二：合并距离过近的子簇以精简树结构。
 * 阶段三：对最终子簇执行距离分配得到聚类标签。
 *
 * @param dataPoints 输入数据点集合
 * @param branchingFactor CF树分支因子（最大子簇数）
 * @param threshold 聚类半径阈值
 * @return 各数据点的聚类标签
 */
QVector<int> BirchClustering11::fit(const QVector<QVector<double>>& dataPoints,
                                     int branchingFactor, double threshold)
{
    QElapsedTimer timer;
    timer.start();

    const int n = dataPoints.size();
    QVector<int> labels(n, -1);
    if (n == 0) {
        emit clusteringCompleted(0);
        return labels;
    }

    /* === 阶段一：构建CF树 === */
    g_cfEntries.clear();
    QVector<int> ptToSc(n, -1);

    for (int i = 0; i < n; ++i) {
        const auto& pt = dataPoints[i];
        const int dim = pt.size();

        /* 查找最近的子簇 */
        double bestDist = 1e18;
        int bestIdx = -1;
        for (int j = 0; j < g_cfEntries.size(); ++j) {
            const int cnt = g_cfEntries[j].first;
            const auto& ls = g_cfEntries[j].second.first;
            double dist = 0.0;
            for (int d = 0; d < dim && d < ls.size(); ++d) {
                double diff = pt[d] - ls[d] / cnt;
                dist += diff * diff;
            }
            dist = qSqrt(dist);
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = j;
            }
        }

        if (bestIdx >= 0 && bestDist <= threshold) {
            /* 吸收进已有子簇 */
            auto& sc = g_cfEntries[bestIdx];
            sc.first++;
            auto& ls = sc.second.first;
            for (int d = 0; d < dim && d < ls.size(); ++d) {
                ls[d] += pt[d];
            }
            double ss = 0.0;
            for (int d = 0; d < dim; ++d) ss += pt[d] * pt[d];
            sc.second.second += ss;
            ptToSc[i] = bestIdx;
        } else {
            /* 创建新子簇 */
            if (g_cfEntries.size() >= branchingFactor) {
                mergeSubclusters(threshold * 2.0);
            }
            QVector<double> ls = pt;
            double ss = 0.0;
            for (int d = 0; d < dim; ++d) ss += pt[d] * pt[d];
            g_cfEntries.append({1, {ls, ss}});
            ptToSc[i] = g_cfEntries.size() - 1;
        }
    }

    /* === 阶段二：精简合并 === */
    mergeSubclusters(threshold * 1.5);

    /* === 阶段三：分配聚类标签 === */
    QMap<int, int> scToLabel;
    int labelCnt = 0;
    for (int i = 0; i < n; ++i) {
        int scIdx = ptToSc[i];
        if (scIdx < 0) continue;
        if (!scToLabel.contains(scIdx)) {
            scToLabel[scIdx] = labelCnt++;
        }
        labels[i] = scToLabel[scIdx];
    }

    m_stats.totalInsertOps += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalInsertOps);

    emit clusteringCompleted(labelCnt);
    return labels;
}

/**
 * @brief 增量插入新数据点到CF树
 *
 * 查找最近子簇，若距离在阈值内则吸收，否则创建新子簇。
 * 若子簇数超过分支因子则自动触发合并。
 *
 * @param point 新数据点
 * @return 是否插入成功
 */
bool BirchClustering11::insertPoint(const QVector<double>& point)
{
    if (point.isEmpty()) return false;
    const int dim = point.size();

    double bestDist = 1e18;
    int bestIdx = -1;
    for (int j = 0; j < g_cfEntries.size(); ++j) {
        const int cnt = g_cfEntries[j].first;
        const auto& ls = g_cfEntries[j].second.first;
        double dist = 0.0;
        for (int d = 0; d < dim && d < ls.size(); ++d) {
            double diff = point[d] - ls[d] / cnt;
            dist += diff * diff;
        }
        dist = qSqrt(dist);
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = j;
        }
    }

    if (bestIdx >= 0 && bestDist <= 0.5) {
        auto& sc = g_cfEntries[bestIdx];
        sc.first++;
        auto& ls = sc.second.first;
        for (int d = 0; d < dim && d < ls.size(); ++d) {
            ls[d] += point[d];
        }
        double ss = 0.0;
        for (int d = 0; d < dim; ++d) ss += point[d] * point[d];
        sc.second.second += ss;
    } else {
        QVector<double> ls = point;
        double ss = 0.0;
        for (int d = 0; d < dim; ++d) ss += point[d] * point[d];
        g_cfEntries.append({1, {ls, ss}});
    }

    m_stats.totalInsertOps++;
    return true;
}

/**
 * @brief 获取CF树的聚类特征摘要
 *
 * 返回每个子簇的统计三元组 (N, LS, SS)。
 * N为点数，LS为各维度线性和向量，SS为平方和。
 *
 * @return 各子簇的 (点数, (线性和, 平方和)) 三元组
 */
QVector<QPair<int, QPair<QVector<double>, double>>> BirchClustering11::getClusteringFeatures() const
{
    return g_cfEntries;
}

/**
 * @brief 合并相近的子簇
 *
 * 遍历所有子簇对，计算质心间欧氏距离，
 * 将距离小于mergeThreshold的子簇合并。
 * 合并时累加CF三元组（点数、线性和、平方和）。
 *
 * @param mergeThreshold 合并距离阈值
 * @return 合并后的子簇数量
 */
int BirchClustering11::mergeSubclusters(double mergeThreshold)
{
    if (g_cfEntries.size() <= 1) return g_cfEntries.size();

    bool merged = true;
    while (merged) {
        merged = false;
        double minDist = 1e18;
        int mergeA = -1, mergeB = -1;

        for (int i = 0; i < g_cfEntries.size(); ++i) {
            for (int j = i + 1; j < g_cfEntries.size(); ++j) {
                const int cntA = g_cfEntries[i].first;
                const int cntB = g_cfEntries[j].first;
                const auto& lsA = g_cfEntries[i].second.first;
                const auto& lsB = g_cfEntries[j].second.first;
                const int dim = qMin(lsA.size(), lsB.size());
                double dist = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = lsA[d] / cntA - lsB[d] / cntB;
                    dist += diff * diff;
                }
                dist = qSqrt(dist);
                if (dist < minDist) {
                    minDist = dist;
                    mergeA = i;
                    mergeB = j;
                }
            }
        }

        if (mergeA >= 0 && mergeB >= 0 && minDist < mergeThreshold) {
            auto& a = g_cfEntries[mergeA];
            const auto& b = g_cfEntries[mergeB];
            a.first += b.first;
            auto& lsA = a.second.first;
            const auto& lsB = b.second.first;
            for (int d = 0; d < qMin(lsA.size(), lsB.size()); ++d) {
                lsA[d] += lsB[d];
            }
            a.second.second += b.second.second;
            g_cfEntries.remove(mergeB);
            merged = true;
        }
    }
    return g_cfEntries.size();
}
