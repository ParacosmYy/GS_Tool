#include "IntervalTree6.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @class IntervalTree6
 * @brief 区间树(Interval Tree)实现
 *
 * 区间树是一种以区间[lo, hi]为元素的红黑树变体。
 * 每个节点存储一个区间及其子树中最大的右端点(maxHi)。
 * 支持O(log n + k)的区间重叠查询，k为结果数。
 *
 * 应用场景: 时间段冲突检测、基因组区间查询、调度冲突判断等。
 */

/**
 * @brief 区间树节点结构
 */
struct IntervalNode {
    int lo;                        /**< 区间左端点 */
    int hi;                        /**< 区间右端点 */
    int maxHi;                     /**< 子树中最大的右端点 */
    QVariant data;                 /**< 附带数据 */
    IntervalNode* left = nullptr;  /**< 左子树 */
    IntervalNode* right = nullptr; /**< 右子树 */

    IntervalNode(int l, int h, const QVariant& d)
        : lo(l), hi(h), maxHi(h), data(d) {}
};

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
IntervalTree6::IntervalTree6(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
{
}

/**
 * @brief 插入区间[lo, hi]附带数据
 *
 * 按照区间左端点作为排序键，递归插入到二叉搜索树中。
 * 插入后更新路径上所有节点的maxHi值。
 * 使用中值策略近似平衡(非严格平衡红黑树)。
 *
 * @param lo 区间左端点
 * @param hi 区间右端点
 * @param data 附带的关联数据
 */
void IntervalTree6::insert(int lo, int hi, const QVariant& data)
{
    QElapsedTimer timer;
    timer.start();

    IntervalNode* newNode = new IntervalNode(lo, hi, data);

    if (!m_root) {
        m_root = newNode;
    } else {
        IntervalNode* current = m_root;
        while (true) {
            /* 更新maxHi */
            if (hi > current->maxHi) {
                current->maxHi = hi;
            }

            if (lo < current->lo) {
                if (!current->left) {
                    current->left = newNode;
                    break;
                }
                current = current->left;
            } else {
                if (!current->right) {
                    current->right = newNode;
                    break;
                }
                current = current->right;
            }
        }
    }

    m_stats.totalIntervalsInserted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalIntervalsInserted + m_stats.totalQueries);
}

/**
 * @brief 查询与指定点重叠的所有区间
 *
 * 利用maxHi剪枝的递归查询:
 * - 如果左子树存在且其maxHi >= point，则左子树可能包含重叠区间
 * - 如果当前节点的lo <= point <= hi，则当前区间重叠
 * - 如果当前节点的lo <= point，则右子树也可能包含重叠区间
 *
 * @param point 查询点
 * @return 与点重叠的所有区间列表，每项包含{({lo, hi}, data)}
 */
QVector<QPair<QPair<int, int>, QVariant>> IntervalTree6::queryPoint(int point) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<int, int>, QVariant>> results;

    if (!m_root) {
        m_timeSum += timer.elapsed();
        return results;
    }

    /* 迭代式深度优先搜索 */
    QVector<IntervalNode*> stack;
    stack.append(m_root);

    while (!stack.isEmpty()) {
        IntervalNode* node = stack.takeLast();
        if (!node) continue;

        /* 检查左子树是否有重叠可能 */
        if (node->left && node->left->maxHi >= point) {
            stack.append(node->left);
        }

        /* 检查当前节点是否重叠 */
        if (point >= node->lo && point <= node->hi) {
            results.append({{node->lo, node->hi}, node->data});
        }

        /* 检查右子树(当point >= node->lo时) */
        if (node->right && point >= node->lo) {
            stack.append(node->right);
        }
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalIntervalsInserted + m_stats.totalQueries);

    emit queryCompleted(point, results.size());

    return results;
}

/**
 * @brief 重置所有统计数据
 *
 * 将区间插入计数、查询计数和计时归零。
 */
void IntervalTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
