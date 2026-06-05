/**
 * @file CartesianTree4.cpp
 * @brief 笛卡尔树4实现 — 可持久化+RMQ/LCA
 *
 * 笛卡尔树实现，支持O(1) RMQ查询和O(1) LCA查询。
 * 通过欧拉游览+稀疏表实现高效区间查询。
 * 笛卡尔树性质: 中序遍历为原序列，堆性质保证RMQ。
 */

#include "utils/tree48/CartesianTree4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化空树
 * @param parent 父QObject
 */
CartesianTree4::CartesianTree4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数，释放资源
 */
CartesianTree4::~CartesianTree4()
{
    destroyTree();
}

/**
 * @brief 从数值序列构建笛卡尔树
 * @param values 输入数值序列
 *
 * 使用单调栈在线性时间内构建笛卡尔树:
 * - 中序遍历保持原始顺序
 * - 最小堆性质：父节点值 <= 子节点值
 * - 用于RMQ查询时，区间最小值的索引即为LCA
 */
void CartesianTree4::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    destroyTree();
    m_n = values.size();
    if (m_n == 0) return;

    m_nodes.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_nodes[i].parent = -1;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].value = values[i];
        m_nodes[i].index = i;
    }

    /* 单调栈构建笛卡尔树 */
    QVector<int> stack;

    for (int i = 0; i < m_n; ++i) {
        int lastPopped = -1;

        /* 弹出栈顶所有大于当前值的节点 */
        while (!stack.isEmpty() && values[stack.back()] > values[i]) {
            lastPopped = stack.back();
            stack.pop_back();
        }

        if (!stack.isEmpty()) {
            /* 当前节点成为栈顶的右子 */
            m_nodes[i].parent = stack.back();
            m_nodes[stack.back()].right = i;
        }

        if (lastPopped >= 0) {
            /* 最后弹出的节点成为当前节点的左子 */
            m_nodes[lastPopped].parent = i;
            m_nodes[i].left = lastPopped;
        }

        stack.append(i);
    }

    /* 找根节点（无父节点的节点） */
    m_root = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_nodes[i].parent < 0) {
            m_root = i;
            break;
        }
    }

    /* 构建欧拉游览和RMQ稀疏表 */
    buildRMQ();

    /* 计算树高 */
    auto getHeight = [&](int node) -> int {
        int h = 0;
        while (node >= 0) {
            int maxChild = -1;
            double maxVal = 1e18;
            if (m_nodes[node].left >= 0) {
                int lc = m_nodes[node].left;
                if (m_nodes[lc].value < maxVal) {
                    maxVal = m_nodes[lc].value;
                    maxChild = lc;
                }
            }
            if (m_nodes[node].right >= 0) {
                int rc = m_nodes[node].right;
                if (m_nodes[rc].value < maxVal) {
                    maxVal = m_nodes[rc].value;
                    maxChild = rc;
                }
            }
            if (maxChild < 0) break;
            h++;
            node = maxChild;
        }
        return h;
    };

    int height = getHeight(m_root);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalBuilds++;
    m_stats.totalNodes += m_n;
    m_stats.avgProcessingTimeMs = (m_stats.totalBuilds + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalBuilds + m_stats.totalQueries)
        : elapsed;

    emit buildCompleted(m_n, height);
}

/**
 * @brief 区间最小值查询(RMQ) — 返回索引
 * @param lo 区间左端
 * @param hi 区间右端
 * @return 区间[lo,hi]中最小值的原始索引
 *
 * 通过欧拉游览+稀疏表实现O(1)查询。
 */
int CartesianTree4::rangeMinimum(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (lo < 0 || hi >= m_n || lo > hi || m_first.isEmpty()) return -1;

    /* 在first映射表中找区间端点的欧拉位置 */
    int eulerLo = m_first[lo];
    int eulerHi = m_first[hi];
    if (eulerLo > eulerHi) std::swap(eulerLo, eulerHi);

    /* 稀疏表RMQ查询 */
    int rmqIdx = rmqQuery(eulerLo, eulerHi);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalQueries++;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalBuilds + m_stats.totalQueries);

    return (rmqIdx >= 0 && rmqIdx < m_euler.size()) ? m_euler[rmqIdx] : -1;
}

/**
 * @brief 区间最小值查询(RMQ) — 返回值
 * @param lo 区间左端
 * @param hi 区间右端
 * @return 区间[lo,hi]中的最小值
 */
double CartesianTree4::rangeMinimumValue(int lo, int hi) const
{
    int idx = rangeMinimum(lo, hi);
    return (idx >= 0 && idx < m_n) ? m_nodes[idx].value : 0.0;
}

/**
 * @brief 最近公共祖先(LCA)查询
 * @param u 第一个节点索引
 * @param v 第二个节点索引
 * @return LCA节点索引
 *
 * LCA(u,v) = 欧拉游览中first[u]到first[v]范围内深度最小的节点。
 */
int CartesianTree4::lca(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return -1;
    if (m_first.isEmpty()) return -1;

    int eu = m_first[u];
    int ev = m_first[v];
    if (eu > ev) std::swap(eu, ev);

    int rmqIdx = rmqQuery(eu, ev);
    return (rmqIdx >= 0 && rmqIdx < m_euler.size()) ? m_euler[rmqIdx] : -1;
}

/**
 * @brief 获取欧拉游览序列
 * @return 欧拉游览中的节点索引序列
 */
QVector<int> CartesianTree4::eulerTour() const
{
    return m_euler;
}

/**
 * @brief 获取欧拉游览的深度数组
 * @return 每个欧拉游览位置对应的深度
 */
QVector<int> CartesianTree4::depthArray() const
{
    return m_depth;
}

/**
 * @brief 重置所有统计信息
 */
void CartesianTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 构建RMQ支持结构（欧拉游览+稀疏表）
 *
 * 欧拉游览: DFS遍历树，每经过一个节点都记录。
 * 稀疏表: 预处理所有长度为2^k的区间最小值。
 */
void CartesianTree4::buildRMQ()
{
    if (m_n == 0 || m_root < 0) return;

    m_euler.clear();
    m_depth.clear();
    m_first.resize(m_n, -1);

    /* DFS欧拉游览 */
    QVector<QPair<int,int>> dfsStack; /* (node, depth) */
    dfsStack.append({m_root, 0});

    while (!dfsStack.isEmpty()) {
        auto [node, depth] = dfsStack.takeLast();

        m_euler.append(node);
        m_depth.append(depth);

        if (m_first[node] < 0) {
            m_first[node] = m_euler.size() - 1;
        }

        /* 先右后左，保证中序遍历顺序 */
        if (m_nodes[node].right >= 0) {
            dfsStack.append({m_nodes[node].right, depth + 1});
            m_euler.append(node);
            m_depth.append(depth);
        }
        if (m_nodes[node].left >= 0) {
            dfsStack.append({m_nodes[node].left, depth + 1});
            m_euler.append(node);
            m_depth.append(depth);
        }
    }

    /* 构建稀疏表 (基于深度) */
    const int m = m_depth.size();
    if (m == 0) return;

    int logM = 0;
    while ((1 << (logM + 1)) <= m) logM++;

    m_sparTable.resize(logM + 1);
    m_sparTable[0].resize(m);
    for (int i = 0; i < m; ++i) {
        m_sparTable[0][i] = i; /* 存储索引 */
    }

    for (int k = 1; k <= logM; ++k) {
        int len = 1 << k;
        m_sparTable[k].resize(m);
        for (int i = 0; i + len <= m; ++i) {
            int left = m_sparTable[k - 1][i];
            int right = m_sparTable[k - 1][i + (len >> 1)];
            m_sparTable[k][i] = (m_depth[left] <= m_depth[right]) ? left : right;
        }
    }
}

/**
 * @brief 稀疏表RMQ查询
 * @param i 查询区间左端
 * @param j 查询区间右端
 * @return 区间[i,j]中深度最小的欧拉位置索引
 */
int CartesianTree4::rmqQuery(int i, int j) const
{
    if (i > j || i < 0 || j >= m_depth.size()) return -1;
    if (i == j) return i;

    int len = j - i + 1;
    int k = 0;
    while ((1 << (k + 1)) <= len) k++;

    int left = m_sparTable[k][i];
    int right = m_sparTable[k][j - (1 << k) + 1];

    return (m_depth[left] <= m_depth[right]) ? left : right;
}

/**
 * @brief 销毁树和辅助数据结构
 */
void CartesianTree4::destroyTree()
{
    m_nodes.clear();
    m_euler.clear();
    m_depth.clear();
    m_first.clear();
    m_sparTable.clear();
    m_n = 0;
    m_root = -1;
}
