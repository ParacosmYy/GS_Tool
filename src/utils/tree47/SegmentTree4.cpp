/**
 * @file SegmentTree4.cpp
 * @brief 线段树4实现 — 可持久化+主席树
 *
 * 可持久化线段树（主席树）实现，支持O(log n)的历史版本查询。
 * 每次更新创建新的根节点路径，共享未修改的子树。
 * 支持区间求和查询和第k大值查询。
 */

#include "utils/tree47/SegmentTree4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化空树
 * @param parent 父QObject
 */
SegmentTree4::SegmentTree4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数，释放所有节点
 */
SegmentTree4::~SegmentTree4()
{
    destroyTree();
}

/**
 * @brief 从数据数组构建初始线段树
 * @param data 初始数据
 *
 * 创建版本0作为初始快照，节点数组预分配足够空间。
 */
void SegmentTree4::build(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    destroyTree();
    m_n = data.size();
    if (m_n == 0) return;

    /* 预分配节点空间 */
    m_nodes.reserve(m_n * 4);

    /* 构建初始树 */
    int root = buildTree(0, m_n - 1, data);
    m_roots.clear();
    m_versions.clear();
    m_roots.append(root);
    m_versions.append(root);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalVersions++;
    m_stats.avgProcessingTimeMs = (m_stats.totalUpdates + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries)
        : elapsed;
}

/**
 * @brief 在指定版本基础上进行单点更新
 * @param version 基础版本号
 * @param pos 更新位置
 * @param value 新值
 * @return 新创建的版本号
 *
 * 可持久化更新: 只修改路径上的节点，未修改的子树与旧版本共享。
 */
int SegmentTree4::update(int version, int pos, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (version < 0 || version >= m_roots.size() || m_n == 0) {
        return currentVersion();
    }

    int newRoot = updateTree(m_roots[version], 0, m_n - 1, pos, value);
    m_roots.append(newRoot);
    m_versions.append(newRoot);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalUpdates++;
    m_stats.totalVersions++;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries);

    emit versionCreated(m_versions.size() - 1, pos);
    return m_versions.size() - 1;
}

/**
 * @brief 查询指定版本的区间和
 * @param version 版本号
 * @param lo 区间左端点
 * @param hi 区间右端点
 * @return 区间[lo, hi]的元素和
 */
double SegmentTree4::query(int version, int lo, int hi) const
{
    if (version < 0 || version >= m_roots.size()) return 0.0;
    return queryTree(m_roots[version], 0, m_n - 1, lo, hi);
}

/**
 * @brief 查询指定版本的第k小值（主席树查询）
 * @param version 版本号
 * @param k 排名（1-based）
 * @return 第k小值
 *
 * 仅在数据为非负整数时完全准确，实数数据作为近似查询。
 */
double SegmentTree4::queryKth(int version, int k) const
{
    if (version < 0 || version >= m_roots.size() || m_n == 0) return 0.0;

    /* 简化实现: 提取所有数据后排序查找 */
    QVector<double> allData = versionData(version);
    if (k < 1 || k > allData.size()) return 0.0;

    std::sort(allData.begin(), allData.end());
    return allData[k - 1];
}

/**
 * @brief 统计指定版本中值在[low, high]范围内的元素个数
 * @param version 版本号
 * @param low 下界
 * @param high 上界
 * @return 符合条件的元素数量
 */
int SegmentTree4::count(int version, double low, double high) const
{
    QVector<double> data = versionData(version);
    int cnt = 0;
    for (double v : data) {
        if (v >= low && v <= high) cnt++;
    }
    return cnt;
}

/**
 * @brief 获取指定版本的完整数据快照
 * @param version 版本号
 * @return 该版本的数据数组
 */
QVector<double> SegmentTree4::versionData(int version) const
{
    if (version < 0 || version >= m_roots.size()) {
        return QVector<double>();
    }

    QVector<double> data(m_n, 0.0);
    /* 通过线段树中序遍历重建数据 */
    for (int i = 0; i < m_n; ++i) {
        data[i] = queryTree(m_roots[version], 0, m_n - 1, i, i);
    }
    return data;
}

/**
 * @brief 重置所有统计信息
 */
void SegmentTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 递归构建线段树
 * @param lo 区间左端
 * @param hi 区间右端
 * @param data 原始数据
 * @return 新创建的节点索引
 */
int SegmentTree4::buildTree(int lo, int hi, const QVector<double>& data)
{
    if (lo > hi) return -1;

    if (lo == hi) {
        return newNode(data[lo], -1, -1);
    }

    int mid = lo + (hi - lo) / 2;
    int leftChild = buildTree(lo, mid, data);
    int rightChild = buildTree(mid + 1, hi, data);

    double sum = 0.0;
    if (leftChild >= 0) sum += m_nodes[leftChild].sum;
    if (rightChild >= 0) sum += m_nodes[rightChild].sum;

    return newNode(sum, leftChild, rightChild);
}

/**
 * @brief 可持久化更新
 * @param node 当前节点
 * @param lo 区间左端
 * @param hi 区间右端
 * @param pos 更新位置
 * @param value 新值
 * @return 新节点索引
 */
int SegmentTree4::updateTree(int node, int lo, int hi, int pos, double value)
{
    if (lo > hi || node < 0) return -1;

    if (lo == hi) {
        return newNode(value, -1, -1);
    }

    int mid = lo + (hi - lo) / 2;
    int newLeft = m_nodes[node].left;
    int newRight = m_nodes[node].right;

    if (pos <= mid) {
        newLeft = updateTree(m_nodes[node].left, lo, mid, pos, value);
    } else {
        newRight = updateTree(m_nodes[node].right, mid + 1, hi, pos, value);
    }

    double sum = 0.0;
    if (newLeft >= 0) sum += m_nodes[newLeft].sum;
    if (newRight >= 0) sum += m_nodes[newRight].sum;

    return newNode(sum, newLeft, newRight);
}

/**
 * @brief 区间查询
 * @param node 当前节点
 * @param lo 节点区间左端
 * @param hi 节点区间右端
 * @param ql 查询区间左端
 * @param qr 查询区间右端
 * @return 区间和
 */
double SegmentTree4::queryTree(int node, int lo, int hi, int ql, int qr) const
{
    if (node < 0 || lo > qr || hi < ql) return 0.0;
    if (ql <= lo && hi <= qr) return m_nodes[node].sum;

    int mid = lo + (hi - lo) / 2;
    double result = 0.0;

    if (ql <= mid) {
        result += queryTree(m_nodes[node].left, lo, mid, ql, qr);
    }
    if (qr > mid) {
        result += queryTree(m_nodes[node].right, mid + 1, hi, ql, qr);
    }

    return result;
}

/**
 * @brief 创建新节点
 * @param sum 节点存储的和
 * @param left 左子节点索引
 * @param right 右子节点索引
 * @return 新节点索引
 */
int SegmentTree4::newNode(double sum, int left, int right)
{
    Node n;
    n.sum = sum;
    n.left = left;
    n.right = right;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/**
 * @brief 销毁整棵树，释放资源
 */
void SegmentTree4::destroyTree()
{
    m_nodes.clear();
    m_roots.clear();
    m_versions.clear();
    m_n = 0;
}
