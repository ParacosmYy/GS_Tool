#include "GraphIsomorph4.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @class GraphIsomorph4
 * @brief 图同构检测器实现
 *
 * 基于VF2算法检测两个图是否同构。
 * 图同构是指两个图通过顶点重命名后具有完全相同的结构。
 * VF2算法通过逐步构建部分映射并检查一致性来搜索同构映射。
 *
 * 一致性检查条件:
 * 1. 前驱一致性: 新映射的边在两个图中必须一致
 * 2. 后继一致性: 候选顶点的邻居与已映射顶点的邻居一致
 *
 * VF2复杂度: 最坏O(n!)，实际中通过剪枝远快于暴力搜索。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
GraphIsomorph4::GraphIsomorph4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 检测两个图是否同构
 *
 * 使用VF2算法逐步尝试构建从g1到g2的双射映射:
 * 1. 初始化空映射
 * 2. 选择g1中的下一个未映射顶点
 * 3. 尝试将g2中的每个未映射顶点映射到它
 * 4. 检查映射一致性(边的对应关系)
 * 5. 递归直到所有顶点映射完成(同构)或回溯
 *
 * 快速排除条件: 顶点数不同、度序列不同
 *
 * @param g1 第一个图的邻接表
 * @param g2 第二个图的邻接表
 * @return true如果两个图同构
 */
bool GraphIsomorph4::isIsomorphic(const QVector<QVector<int>>& g1,
                                   const QVector<QVector<int>>& g2)
{
    QElapsedTimer timer;
    timer.start();

    m_mapping.clear();

    int n1 = g1.size();
    int n2 = g2.size();

    /* 快速排除: 顶点数不同 */
    if (n1 != n2) {
        m_stats.totalComparisons++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;
        emit comparisonCompleted(false, qMax(n1, n2));
        return false;
    }

    if (n1 == 0) {
        m_stats.totalComparisons++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;
        emit comparisonCompleted(true, 0);
        return true;
    }

    /* 快速排除: 度序列不同 */
    QVector<int> deg1(n1), deg2(n2);
    for (int i = 0; i < n1; ++i) deg1[i] = g1[i].size();
    for (int i = 0; i < n2; ++i) deg2[i] = g2[i].size();
    std::sort(deg1.begin(), deg1.end());
    std::sort(deg2.begin(), deg2.end());
    if (deg1 != deg2) {
        m_stats.totalComparisons++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;
        emit comparisonCompleted(false, n1);
        return false;
    }

    /* 构建邻接矩阵加速查询 */
    QVector<QVector<bool>> adj1(n1, QVector<bool>(n1, false));
    QVector<QVector<bool>> adj2(n2, QVector<bool>(n2, false));
    for (int i = 0; i < n1; ++i) {
        for (int j : g1[i]) adj1[i][j] = true;
    }
    for (int i = 0; i < n2; ++i) {
        for (int j : g2[i]) adj2[i][j] = true;
    }

    /* VF2映射: map1to2[g1顶点] = g2顶点, map2to1[g2顶点] = g1顶点 */
    QVector<int> map1to2(n1, -1);
    QVector<int> map2to1(n2, -1);

    /* 一致性检查函数 */
    auto isConsistent = [&](int v1, int v2) -> bool {
        int mappedNeighbors = 0;
        int mappedNeighborsEdges = 0;

        for (int i = 0; i < n1; ++i) {
            if (map1to2[i] < 0) continue;
            bool edgeInG1 = adj1[v1][i] || adj1[i][v1];
            bool edgeInG2 = adj2[v2][map1to2[i]] || adj2[map1to2[i]][v2];
            if (edgeInG1 != edgeInG2) return false;
            if (edgeInG1) mappedNeighborsEdges++;
            mappedNeighbors++;
        }

        /* 检查度数一致性 */
        int degV1 = 0, degV2 = 0;
        for (int j : g1[v1]) if (map1to2[j] < 0) degV1++;
        for (int j : g2[v2]) if (map2to1[j] < 0) degV2++;
        if (degV1 != degV2) return false;

        return true;
    };

    /* VF2递归搜索 */
    std::function<bool(int)> vf2 = [&](int depth) -> bool {
        if (depth == n1) return true; /* 所有顶点已映射 */

        /* 选择g1中下一个未映射顶点(简单策略: 第一个未映射的) */
        int v1 = -1;
        for (int i = 0; i < n1; ++i) {
            if (map1to2[i] < 0) { v1 = i; break; }
        }

        /* 尝试g2中每个未映射顶点 */
        for (int v2 = 0; v2 < n2; ++v2) {
            if (map2to1[v2] >= 0) continue;

            if (isConsistent(v1, v2)) {
                map1to2[v1] = v2;
                map2to1[v2] = v1;

                if (vf2(depth + 1)) return true;

                /* 回溯 */
                map1to2[v1] = -1;
                map2to1[v2] = -1;
            }
        }

        return false;
    };

    bool result = vf2(0);

    if (result) {
        m_mapping.resize(n1);
        for (int i = 0; i < n1; ++i) {
            m_mapping[i] = map1to2[i];
        }
        m_stats.totalIsomorphicFound++;
    }

    m_stats.totalComparisons++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;

    emit comparisonCompleted(result, n1);

    return result;
}

/**
 * @brief 获取同构映射
 *
 * 返回g1到g2的顶点映射关系。
 * 必须在isIsomorphic()返回true后使用。
 *
 * @return 映射向量，mapping[i] = j表示g1的第i个顶点映射到g2的第j个
 */
QVector<int> GraphIsomorph4::mapping() const
{
    return m_mapping;
}

/**
 * @brief 重置所有统计数据
 *
 * 将比较计数、同构计数和计时归零。
 */
void GraphIsomorph4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_mapping.clear();
}
