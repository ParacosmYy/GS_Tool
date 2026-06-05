/**
 * @file PersistentTree2.cpp
 * @brief 持久化平衡BST实现 — 路径拷贝 + 版本化查询 + 版本GC
#include "utils/tree18/PersistentTree2.h"
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <stack>
// ═══════════════════════════════════════════════════════════
// 构造 / 析构
PersistentTree2::PersistentTree2(QObject* parent)
    : QObject(parent)
    , m_nextVersion(0)
{
    /* 初始空版本 */
    VersionInfo v0;
    v0.version = 0;
    v0.rootNode = -1;
    v0.nodeCount = 0;
    v0.treeSize = 0;
    v0.description = QStringLiteral("init");
    m_versions[0] = v0;
    m_nextVersion = 1;
}

PersistentTree2::~PersistentTree2() = default;
// ═══════════════════════════════════════════════════════════
// 修改操作
PersistentTree2::Version PersistentTree2::insert(
    Key key, Value value, const QString& description)
{
    QElapsedTimer timer;
    timer.start();
    /* 在当前最新版本根上插入 */
    Version curVer = m_nextVersion - 1;
    int rootIdx = m_versions[curVer].rootNode;
    int nodesBefore = m_nodes.size();
    int newRoot = insertImpl(rootIdx, key, value);
    /* 增加新根的引用 */
    addRef(newRoot);
    /* 释放旧根引用 */
    releaseRef(rootIdx);
    /* 创建新版本 */
    VersionInfo info;
    info.version = m_nextVersion;
    info.rootNode = newRoot;
    info.nodeCount = m_nodes.size() - nodesBefore +
                     m_versions[curVer].nodeCount;
    info.treeSize = m_versions[curVer].treeSize + 1;
    info.description = description;
    m_versions[m_nextVersion] = info;
    Version newVer = m_nextVersion++;
    m_stats.totalInsertions++;
    m_stats.totalNodesCreated += m_nodes.size() - nodesBefore;
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 totalOps = m_stats.totalInsertions + m_stats.totalDeletions +
                       m_stats.totalLookups;
    m_stats.avgProcessingTimeMs =
        (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;
    emit versionCreated(newVer, description);
    return newVer;
}

PersistentTree2::Version PersistentTree2::remove(
    Key key, const QString& description)
{
    QElapsedTimer timer;
    timer.start();
    Version curVer = m_nextVersion - 1;
    int rootIdx = m_versions[curVer].rootNode;
    /* 先检查键是否存在 */
    auto lookup = lookupImpl(rootIdx, key, 0);
    if (!lookup.found) return -1;
    int nodesBefore = m_nodes.size();
    int newRoot = removeImpl(rootIdx, key);
    addRef(newRoot);
    releaseRef(rootIdx);
    VersionInfo info;
    info.version = m_nextVersion;
    info.rootNode = newRoot;
    info.nodeCount = m_versions[curVer].nodeCount;
    info.treeSize = m_versions[curVer].treeSize - 1;
    info.description = description;
    m_versions[m_nextVersion] = info;
    Version newVer = m_nextVersion++;
    m_stats.totalDeletions++;
    m_stats.totalNodesCreated += m_nodes.size() - nodesBefore;
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 totalOps = m_stats.totalInsertions + m_stats.totalDeletions +
                       m_stats.totalLookups;
    m_stats.avgProcessingTimeMs =
        (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;
    emit versionCreated(newVer, description);
    return newVer;
}

// ═══════════════════════════════════════════════════════════
// 查询操作
PersistentTree2::LookupResult PersistentTree2::lookup(
    Key key, Version version) const
{
    QElapsedTimer timer;
    timer.start();
    if (version < 0) version = m_nextVersion - 1;
    LookupResult result;
    if (!m_versions.contains(version)) return result;
    int root = m_versions[version].rootNode;
    result = lookupImpl(root, key, 0);
    m_stats.totalLookups++;
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 totalOps = m_stats.totalInsertions + m_stats.totalDeletions +
                       m_stats.totalLookups;
    m_stats.avgProcessingTimeMs =
        (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;
    return result;
}

PersistentTree2::RangeResult PersistentTree2::rangeQuery(
    Key low, Key high, Version version) const
{
    if (version < 0) version = m_nextVersion - 1;
    RangeResult result;
    if (!m_versions.contains(version)) return result;
    int root = m_versions[version].rootNode;
    rangeQueryImpl(root, low, high, result.entries);
    result.count = result.entries.size();
    return result;
}

int PersistentTree2::size(Version version) const
{
    if (version < 0) version = m_nextVersion - 1;
    if (!m_versions.contains(version)) return 0;
    return m_versions[version].treeSize;
}

// ═══════════════════════════════════════════════════════════
// 版本管理
PersistentTree2::Version PersistentTree2::currentVersion() const
{
    return m_nextVersion - 1;
}

QVector<PersistentTree2::VersionInfo> PersistentTree2::versionHistory() const
{
    QVector<VersionInfo> history;
    for (auto it = m_versions.constBegin();
         it != m_versions.constEnd(); ++it) {
        history.append(it.value());
    }
    return history;
}

PersistentTree2::VersionInfo PersistentTree2::versionInfo(Version version) const
{
    return m_versions.value(version, VersionInfo{});
}

PersistentTree2::Version PersistentTree2::rollback(
    Version version, const QString& description)
{
    QElapsedTimer timer;
    timer.start();
    if (!m_versions.contains(version)) return -1;
    const VersionInfo& target = m_versions[version];
    VersionInfo info;
    info.version = m_nextVersion;
    info.rootNode = target.rootNode;
    info.nodeCount = target.nodeCount;
    info.treeSize = target.treeSize;
    info.description = description.isEmpty()
        ? QStringLiteral("rollback to v%1").arg(version) : description;
    addRef(info.rootNode);
    m_versions[m_nextVersion] = info;
    Version newVer = m_nextVersion++;
    m_timeSum += static_cast<double>(timer.elapsed());
    emit versionCreated(newVer, info.description);
    return newVer;
}

int PersistentTree2::garbageCollect(const QSet<Version>& keepVersions)
{
    /* 收集需要保留的版本对应的根节点 */
    QSet<int> reachableRoots;
    for (Version v : keepVersions) {
        if (m_versions.contains(v)) {
            reachableRoots.insert(m_versions[v].rootNode);
        }
    }
    /* 从可达根开始BFS标记所有可达节点 */
    QSet<int> reachable;
    std::stack<int> stack;
    for (int root : reachableRoots) {
        if (root >= 0) stack.push(root);
    }
    while (!stack.empty()) {
        int idx = stack.top();
        stack.pop();
        if (reachable.contains(idx) || idx < 0 ||
            idx >= m_nodes.size()) continue;
        reachable.insert(idx);
        if (m_nodes[idx].left >= 0) stack.push(m_nodes[idx].left);
        if (m_nodes[idx].right >= 0) stack.push(m_nodes[idx].right);
    }
    /* 标记不可达节点, 但不实际删除(避免索引偏移) */
    /* 改为: 将不可达节点标记为"dead" */
    int gcCount = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (!reachable.contains(i) && m_nodes[i].refCount >= 0) {
            m_nodes[i].refCount = -1; /* 标记为dead */
            gcCount++;
        }
    }
    /* 删除不可达版本 */
    QList<Version> toRemove;
    for (auto it = m_versions.constBegin();
         it != m_versions.constEnd(); ++it) {
        if (!keepVersions.contains(it.key())) {
            toRemove.append(it.key());
        }
    }
    for (Version v : toRemove) {
        m_versions.remove(v);
    }
    m_stats.totalNodesGCd += gcCount;
    emit nodesGarbageCollected(gcCount);
    return gcCount;
}

// ═══════════════════════════════════════════════════════════
// 统计
PersistentTree2::Stats PersistentTree2::stats() const
{
    return m_stats;
}

void PersistentTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ═══════════════════════════════════════════════════════════
// 内部实现 — 节点管理
int PersistentTree2::allocateNode(Key key, Value value, int left, int right)
{
    Node n;
    n.key = key;
    n.value = value;
    n.left = left;
    n.right = right;
    n.height = 1 + qMax(getHeight(left), getHeight(right));
    n.refCount = 0;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

int PersistentTree2::cloneNode(int nodeIdx)
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return -1;
    return allocateNode(m_nodes[nodeIdx].key, m_nodes[nodeIdx].value,
                        m_nodes[nodeIdx].left, m_nodes[nodeIdx].right);
}

void PersistentTree2::addRef(int nodeIdx)
{
    if (nodeIdx >= 0 && nodeIdx < m_nodes.size()) {
        m_nodes[nodeIdx].refCount++;
    }
    /* 同时增加子节点引用(共享子树不需要额外ref, 简化处理) */
}

void PersistentTree2::releaseRef(int nodeIdx)
{
    if (nodeIdx >= 0 && nodeIdx < m_nodes.size()) {
        m_nodes[nodeIdx].refCount--;
    }
}

// ═══════════════════════════════════════════════════════════
// 内部实现 — AVL操作
int PersistentTree2::getHeight(int nodeIdx) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return 0;
    return m_nodes[nodeIdx].height;
}

int PersistentTree2::getBalanceFactor(int nodeIdx) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return 0;
    return getHeight(m_nodes[nodeIdx].left) - getHeight(m_nodes[nodeIdx].right);
}

int PersistentTree2::updateHeight(int nodeIdx)
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return 0;
    m_nodes[nodeIdx].height = 1 + qMax(
        getHeight(m_nodes[nodeIdx].left),
        getHeight(m_nodes[nodeIdx].right));
    return m_nodes[nodeIdx].height;
}

int PersistentTree2::rotateLeft(int nodeIdx)
{
    /* 路径拷贝: 创建新节点 */
    int newLeft = cloneNode(nodeIdx);
    int newRoot = cloneNode(m_nodes[nodeIdx].right);
    m_nodes[newLeft].right = m_nodes[newRoot].left;
    m_nodes[newRoot].left = newLeft;
    updateHeight(newLeft);
    updateHeight(newRoot);
    return newRoot;
}

int PersistentTree2::rotateRight(int nodeIdx)
{
    int newRight = cloneNode(nodeIdx);
    int newRoot = cloneNode(m_nodes[nodeIdx].left);
    m_nodes[newRight].left = m_nodes[newRoot].right;
    m_nodes[newRoot].right = newRight;
    updateHeight(newRight);
    updateHeight(newRoot);
    return newRoot;
}

int PersistentTree2::balance(int nodeIdx)
{
    updateHeight(nodeIdx);
    int bf = getBalanceFactor(nodeIdx);
    if (bf > 1) {
        if (getBalanceFactor(m_nodes[nodeIdx].left) < 0) {
            /* LR: 先左旋左子 */
            int newLeft = rotateLeft(m_nodes[nodeIdx].left);
            m_nodes[nodeIdx].left = newLeft;
        }
        return rotateRight(nodeIdx);
    }
    if (bf < -1) {
        if (getBalanceFactor(m_nodes[nodeIdx].right) > 0) {
            /* RL: 先右旋右子 */
            int newRight = rotateRight(m_nodes[nodeIdx].right);
            m_nodes[nodeIdx].right = newRight;
        }
        return rotateLeft(nodeIdx);
    }
    return nodeIdx;
}

// ═══════════════════════════════════════════════════════════
// 内部实现 — 插入
int PersistentTree2::insertImpl(int nodeIdx, Key key, Value value)
{
    if (nodeIdx < 0) {
        /* 空位置: 创建新节点 */
        return allocateNode(key, value, -1, -1);
    }
    /* 路径拷贝: 克隆当前节点 */
    int newNode = cloneNode(nodeIdx);
    if (key < m_nodes[newNode].key) {
        m_nodes[newNode].left = insertImpl(m_nodes[newNode].left, key, value);
    } else if (key > m_nodes[newNode].key) {
        m_nodes[newNode].right = insertImpl(m_nodes[newNode].right, key, value);
    } else {
        /* 键已存在: 更新值 */
        m_nodes[newNode].value = value;
    }
    return balance(newNode);
}

// ═══════════════════════════════════════════════════════════
// 内部实现 — 删除
int PersistentTree2::removeImpl(int nodeIdx, Key key)
{
    if (nodeIdx < 0) return -1;
    int newNode = cloneNode(nodeIdx);
    if (key < m_nodes[newNode].key) {
        m_nodes[newNode].left = removeImpl(m_nodes[newNode].left, key);
    } else if (key > m_nodes[newNode].key) {
        m_nodes[newNode].right = removeImpl(m_nodes[newNode].right, key);
    } else {
        /* 找到要删除的节点 */
        if (m_nodes[newNode].left < 0) return m_nodes[newNode].right;
        if (m_nodes[newNode].right < 0) return m_nodes[newNode].left;
        /* 两子都存在: 找右子树最小值替代 */
        int minNode = m_nodes[newNode].right;
        while (m_nodes[minNode].left >= 0) {
            minNode = m_nodes[minNode].left;
        }
        m_nodes[newNode].key = m_nodes[minNode].key;
        m_nodes[newNode].value = m_nodes[minNode].value;
        m_nodes[newNode].right = removeImpl(
            m_nodes[newNode].right, m_nodes[minNode].key);
    }
    return balance(newNode);
}

// ═══════════════════════════════════════════════════════════
// 内部实现 — 查询
PersistentTree2::LookupResult PersistentTree2::lookupImpl(
    int nodeIdx, Key key, int depth) const
{
    LookupResult result;
    result.depth = depth;
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) {
        result.found = false;
        return result;
    }
    const Node& node = m_nodes[nodeIdx];
    if (key < node.key) {
        return lookupImpl(node.left, key, depth + 1);
    } else if (key > node.key) {
        return lookupImpl(node.right, key, depth + 1);
    } else {
        result.found = true;
        result.key = key;
        result.value = node.value;
        return result;
    }
}

void PersistentTree2::rangeQueryImpl(
    int nodeIdx, Key low, Key high,
    QVector<QPair<Key, Value>>& result) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;
    const Node& node = m_nodes[nodeIdx];
    /* 中序遍历, 剪枝 */
    if (node.key > low) {
        rangeQueryImpl(node.left, low, high, result);
    }
    if (node.key >= low && node.key <= high) {
        result.append({node.key, node.value});
    }
    if (node.key < high) {
        rangeQueryImpl(node.right, low, high, result);
    }
}

