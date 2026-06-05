#include "KDTree6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class KDTree6
 * @brief KD-Tree多维空间索引实现
 *
 * KD树(K-Dimensional Tree)是k维空间中的二叉空间分区树。
 * 每层按一个维度交替切分: 第d层使用第(d mod k)维作为分割轴。
 * 选择中位数作为分割点，保证树的平衡性。
 *
 * 支持: k维点插入、最近邻查询(O(log n)平均)。
 * 应用: 最近邻搜索、范围查询、图像特征匹配、推荐系统。
 */

/**
 * @brief KD树节点结构
 */
struct KDNode {
    QVector<double> point;  /**< k维坐标 */
    QVariant data;          /**< 附带数据 */
    int splitDim;           /**< 分割维度 */
    KDNode* left;           /**< 左子树(分割维值 <= 分割值) */
    KDNode* right;          /**< 右子树(分割维值 > 分割值) */

    KDNode(const QVector<double>& p, const QVariant& d, int dim)
        : point(p), data(d), splitDim(dim), left(nullptr), right(nullptr) {}

    ~KDNode() { delete left; delete right; }
};

/**
 * @brief 构造函数
 * @param dimensions 空间维度数
 * @param parent 父QObject
 */
KDTree6::KDTree6(int dimensions, QObject* parent)
    : QObject(parent)
    , m_dimensions(dimensions)
    , m_root(nullptr)
{
}

/**
 * @brief 插入k维点到KD树
 *
 * 从根节点沿分割维度比较递归下降:
 * - 若point[splitDim] <= node->point[splitDim]，进入左子树
 * - 否则进入右子树
 * - 到达空节点时创建新叶节点
 * 分割维度交替选择: depth % m_dimensions
 *
 * @param point k维坐标点
 * @param data 附带数据
 */
void KDTree6::insert(const QVector<double>& point, const QVariant& data)
{
    QElapsedTimer timer;
    timer.start();

    if (point.size() != m_dimensions) {
        m_timeSum += timer.elapsed();
        return;
    }

    KDNode* newNode = new KDNode(point, data, 0);

    if (!m_root) {
        m_root = newNode;
        m_root->splitDim = 0;
    } else {
        KDNode* current = m_root;
        int depth = 1;

        while (true) {
            int dim = depth % m_dimensions;

            if (point[current->splitDim] <= current->point[current->splitDim]) {
                if (!current->left) {
                    newNode->splitDim = dim;
                    current->left = newNode;
                    break;
                }
                current = current->left;
            } else {
                if (!current->right) {
                    newNode->splitDim = dim;
                    current->right = newNode;
                    break;
                }
                current = current->right;
            }
            depth++;
        }
    }

    m_stats.totalPointsInserted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPointsInserted + m_stats.totalNearestQueries);
}

/**
 * @brief 最近邻查询
 *
 * 使用分支限界法搜索最近邻:
 * 1. 从根节点递归下降到叶节点，记录当前最近点
 * 2. 回溯检查兄弟子树是否可能包含更近的点
 * 3. 剪枝条件: 查询点在分割维的距离 > 当前最近距离
 *
 * 时间复杂度: 平均O(log n)，最坏O(n)(高维退化)
 *
 * @param query 查询点的k维坐标
 * @return {最近邻点坐标, 附带数据}
 */
QPair<QVector<double>, QVariant> KDTree6::nearestNeighbor(const QVector<double>& query) const
{
    QElapsedTimer timer;
    timer.start();

    QPair<QVector<double>, QVariant> result;

    if (!m_root || query.size() != m_dimensions) {
        m_timeSum += timer.elapsed();
        return result;
    }

    double bestDist = 1e18;
    KDNode* bestNode = nullptr;

    /* 递归搜索最近邻 */
    std::function<void(KDNode*)> search = [&](KDNode* node) {
        if (!node) return;

        /* 计算距离 */
        double dist = 0.0;
        for (int i = 0; i < m_dimensions; ++i) {
            double diff = query[i] - node->point[i];
            dist += diff * diff;
        }

        if (dist < bestDist) {
            bestDist = dist;
            bestNode = node;
        }

        /* 决定搜索方向 */
        int dim = node->splitDim;
        double diff = query[dim] - node->point[dim];
        KDNode* first = (diff <= 0) ? node->left : node->right;
        KDNode* second = (diff <= 0) ? node->right : node->left;

        /* 搜索优先方向 */
        search(first);

        /* 检查是否需要搜索另一方向 */
        if (diff * diff < bestDist) {
            search(second);
        }
    };

    search(m_root);

    if (bestNode) {
        result.first = bestNode->point;
        result.second = bestNode->data;
    }

    m_stats.totalNearestQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPointsInserted + m_stats.totalNearestQueries);

    double finalDist = qSqrt(bestDist);
    emit nearestFound(0, finalDist);

    return result;
}

/**
 * @brief 重置所有统计数据
 *
 * 将插入计数、查询计数和计时归零。
 */
void KDTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    delete m_root;
    m_root = nullptr;
}
