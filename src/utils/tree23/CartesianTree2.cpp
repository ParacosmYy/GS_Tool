/**
 * @file CartesianTree2.cpp
 * @brief 笛卡尔树实现 — 线性时间构建/RMQ via LCA/中序遍历
 */

#include "utils/tree23/CartesianTree2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
CartesianTree2::CartesianTree2(QObject* parent)
    : QObject(parent)
    , m_root(-1)
    , m_isMinHeap(true)
    , m_lcaCacheValid(false)
    , m_timeSum(0.0)
{
}

/** @brief 从序列线性时间构建笛卡尔树(最小堆) @param sequence 输入序列 */
void CartesianTree2::buildMin(const QVector<double>& sequence)
{
    buildImpl(sequence, true);
}

/** @brief 从序列线性时间构建笛卡尔树(最大堆) @param sequence 输入序列 */
void CartesianTree2::buildMax(const QVector<double>& sequence)
{
    buildImpl(sequence, false);
}

/** @brief RMQ查询: 区间最小值 @param left 左边界 @param right 右边界 @return 最小值位置和值 */
CartesianTree2::RmqResult CartesianTree2::rangeMinQuery(int left, int right) const
{
    RmqResult result;
    if (m_nodes.isEmpty() || left < 0 || right >= m_nodes.size()
        || left > right) {
        return result;
    }

    /* 笛卡尔树(最小堆)中, [left,right]的RMQ = LCA(left, right) */
    int lcaIdx = lca(left, right);
    result.minIndex = lcaIdx;
    result.minValue = m_nodes[lcaIdx].priority;

    return result;
}

/** @brief RMQ查询: 区间最大值 @param left 左边界 @param right 右边界 @return 最大值位置和值 */
CartesianTree2::RmqResult CartesianTree2::rangeMaxQuery(int left, int right) const
{
    RmqResult result;
    if (m_nodes.isEmpty() || left < 0 || right >= m_nodes.size()
        || left > right) {
        return result;
    }

    /* 最大堆笛卡尔树: LCA给出最大值 */
    int lcaIdx = lca(left, right);
    result.minIndex = lcaIdx;
    result.minValue = m_nodes[lcaIdx].priority;

    return result;
}

/** @brief 计算两节点最近公共祖先 @param u 节点u @param v 节点v @return LCA节点索引 */
int CartesianTree2::lca(int u, int v) const
{
    if (u < 0 || u >= m_nodes.size() || v < 0 || v >= m_nodes.size()) {
        return -1;
    }
    if (u == v) return u;

    /* 构建欧拉环游缓存(如果无效) */
    if (!m_lcaCacheValid) {
        CartesianTree2* self = const_cast<CartesianTree2*>(this);
        self->m_euler.clear();
        self->m_eulerDepth.clear();
        self->m_firstOccur.resize(m_nodes.size(), -1);
        self->eulerHelper(m_root, self->m_euler, self->m_eulerDepth);
        self->m_lcaCacheValid = true;
    }

    int fu = m_firstOccur[u];
    int fv = m_firstOccur[v];
    if (fu < 0 || fv < 0) return -1;

    /* 在欧拉环游的[fu, fv]区间中找深度最小的节点 */
    int lo = qMin(fu, fv);
    int hi = qMax(fu, fv);
    int minDepth = m_eulerDepth[lo];
    int lcaIdx = m_euler[lo];

    for (int i = lo + 1; i <= hi; ++i) {
        if (m_eulerDepth[i] < minDepth) {
            minDepth = m_eulerDepth[i];
            lcaIdx = m_euler[i];
        }
    }

    return lcaIdx;
}

/** @brief 获取节点信息 @param idx 节点索引 @return 节点 */
const CartesianTree2::Node& CartesianTree2::node(int idx) const
{
    return m_nodes[idx];
}

/** @brief 计算树深度 @return 最大深度 */
int CartesianTree2::computeDepth() const
{
    if (m_root < 0) return 0;
    return depthHelper(m_root);
}

/** @brief 中序遍历 @param start 起始节点 @return 中序序列索引 */
QVector<int> CartesianTree2::inorderTraversal(int start) const
{
    QVector<int> result;
    if (start < 0 || start >= m_nodes.size()) {
        /* 默认从根开始 */
        if (m_root >= 0) inorderHelper(m_root, result);
    } else {
        inorderHelper(start, result);
    }
    return result;
}

/** @brief 重置统计 */
void CartesianTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 构建实现(线性时间) @param sequence 输入序列 @param isMinHeap true=最小堆
 *
 * 使用单调栈实现O(n)构建:
 * 维护右脊(right spine)上的节点栈,
 * 新节点沿脊向上找到合适位置,建立父子关系。
 */
void CartesianTree2::buildImpl(const QVector<double>& sequence, bool isMinHeap)
{
    QElapsedTimer timer;
    timer.start();

    int n = sequence.size();
    m_isMinHeap = isMinHeap;
    m_lcaCacheValid = false;

    m_nodes.resize(n);
    for (int i = 0; i < n; ++i) {
        m_nodes[i].key = i;
        m_nodes[i].priority = sequence[i];
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].parent = -1;
    }

    /* 单调栈: 维护右脊上的节点 */
    QStack<int> stack;

    for (int i = 0; i < n; ++i) {
        int last = -1;

        /* 弹出栈顶直到找到满足堆序的位置 */
        while (!stack.isEmpty()) {
            int top = stack.top();
            bool ok = isMinHeap
                      ? (m_nodes[top].priority <= m_nodes[i].priority)
                      : (m_nodes[top].priority >= m_nodes[i].priority);
            if (ok) break;
            last = stack.top();
            stack.pop();
        }

        if (!stack.isEmpty()) {
            /* 新节点成为栈顶的右子 */
            int parent = stack.top();
            m_nodes[i].parent = parent;
            m_nodes[i].left = m_nodes[parent].right;
            if (m_nodes[parent].right >= 0) {
                m_nodes[m_nodes[parent].right].parent = i;
            }
            m_nodes[parent].right = i;
        } else {
            /* 新节点成为根,弹出节点成为其左子 */
            m_nodes[i].left = last;
            if (last >= 0) {
                m_nodes[last].parent = i;
            }
            m_root = i;
        }

        stack.push(i);
    }

    /* 如果根不是0,则根是栈中剩余最底部的元素 */
    /* 找根: 沿着parent链走到顶 */
    if (!m_nodes.isEmpty()) {
        m_root = 0;
        while (m_nodes[m_root].parent >= 0) {
            m_root = m_nodes[m_root].parent;
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalBuilds;
    m_stats.totalNodesProcessed += static_cast<quint64>(n);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalBuilds);
    int depth = computeDepth();
    if (depth > m_stats.maxTreeDepth) m_stats.maxTreeDepth = depth;

    emit buildComplete(n, depth);
}

/** @brief 递归计算深度 @param idx 节点索引 @return 子树深度 */
int CartesianTree2::depthHelper(int idx) const
{
    if (idx < 0) return 0;
    int leftDepth = depthHelper(m_nodes[idx].left);
    int rightDepth = depthHelper(m_nodes[idx].right);
    return 1 + qMax(leftDepth, rightDepth);
}

/** @brief 递归中序遍历 @param idx 节点索引 @param result 输出序列 */
void CartesianTree2::inorderHelper(int idx, QVector<int>& result) const
{
    if (idx < 0) return;
    inorderHelper(m_nodes[idx].left, result);
    result.append(idx);
    inorderHelper(m_nodes[idx].right, result);
}

/** @brief 构建欧拉环游序列 @return 欧拉环游索引列表 */
QVector<int> CartesianTree2::eulerTour() const
{
    QVector<int> euler;
    QVector<int> depthList;
    eulerHelper(m_root, euler, depthList);
    return euler;
}

/** @brief 欧拉环游递归辅助 @param idx 当前节点 @param euler 欧拉序列 @param depthList 深度序列 */
void CartesianTree2::eulerHelper(int idx, QVector<int>& euler,
                                  QVector<int>& depthList) const
{
    if (idx < 0) return;

    /* 首次出现 */
    if (idx < m_firstOccur.size()) {
        m_firstOccur[idx] = euler.size();
    }

    int depth = (m_nodes[idx].parent < 0) ? 0
                : depthList.isEmpty() ? 1
                : depthList.last() + 1;

    euler.append(idx);
    depthList.append(depth - (m_nodes[idx].parent < 0 ? 0 : depthList.isEmpty() ? 0 : 0));

    /* 正确计算深度 */
    int curDepth = 0;
    int tmp = idx;
    while (m_nodes[tmp].parent >= 0) {
        ++curDepth;
        tmp = m_nodes[tmp].parent;
    }
    depthList[depthList.size() - 1] = curDepth;

    /* 递归遍历左子树 */
    if (m_nodes[idx].left >= 0) {
        eulerHelper(m_nodes[idx].left, euler, depthList);
        euler.append(idx);
        depthList.append(curDepth);
    }

    /* 递归遍历右子树 */
    if (m_nodes[idx].right >= 0) {
        eulerHelper(m_nodes[idx].right, euler, depthList);
        euler.append(idx);
        depthList.append(curDepth);
    }
}
