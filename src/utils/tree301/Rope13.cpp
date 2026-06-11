/**
 * @file Rope13.cpp
 * @brief Rope13 实现
 *
 * 实现Rope数据结构：写时复制子树共享与路径复制实现函数式持久化文本编辑。
 */

#include "utils/tree301/Rope13.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Rope13::Rope13(QObject *parent)
    : QObject(parent) {}

Rope13::~Rope13() = default;

/* ---- Configuration ---- */

void Rope13::setLeafSize(int size) { m_leafSize = qBound(8, size, 4096); }

/* ---- Allocate a new node ---- */

int Rope13::allocNode()
{
    Node n;
    m_nodes.append(n);
    m_stats.numNodes = m_nodes.size();
    return m_nodes.size() - 1;
}

/* ---- Clone node for COW path copying ---- */

int Rope13::cloneNode(int idx)
{
    if (idx < 0 || idx >= m_nodes.size()) return -1;

    int newIdx = allocNode();
    m_nodes[newIdx] = m_nodes[idx];
    m_nodes[newIdx].refCount = 1;
    m_nodes[newIdx].version = m_currentVersion;

    // Increment child ref counts (shared subtrees)
    if (m_nodes[newIdx].leftIdx >= 0)
        m_nodes[m_nodes[newIdx].leftIdx].refCount++;
    if (m_nodes[newIdx].rightIdx >= 0)
        m_nodes[m_nodes[newIdx].rightIdx].refCount++;

    return newIdx;
}

/* ---- Build helper ---- */

int Rope13::buildHelper(const QString& text, int start, int end)
{
    int len = end - start;
    if (len <= m_leafSize) {
        int idx = allocNode();
        m_nodes[idx].isLeaf = true;
        m_nodes[idx].text = text.mid(start, len);
        m_nodes[idx].weight = len;
        m_nodes[idx].leftIdx = -1;
        m_nodes[idx].rightIdx = -1;
        return idx;
    }

    int mid = start + len / 2;
    int left = buildHelper(text, start, mid);
    int right = buildHelper(text, mid, end);

    int idx = allocNode();
    m_nodes[idx].isLeaf = false;
    m_nodes[idx].leftIdx = left;
    m_nodes[idx].rightIdx = right;
    m_nodes[idx].weight = computeWeight(left);
    return idx;
}

/* ---- Compute weight ---- */

int Rope13::computeWeight(int idx) const
{
    if (idx < 0 || idx >= m_nodes.size()) return 0;
    const auto& node = m_nodes[idx];
    if (node.isLeaf) return node.text.size();
    return computeWeight(node.leftIdx);
}

/* ---- Collect text ---- */

void Rope13::collectText(int idx, QString& out) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    const auto& node = m_nodes[idx];
    if (node.isLeaf) {
        out.append(node.text);
        return;
    }
    collectText(node.leftIdx, out);
    collectText(node.rightIdx, out);
}

/* ---- Build rope ---- */

Rope13::EditResult Rope13::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    m_currentVersion++;

    int root = buildHelper(text, 0, text.size());
    m_currentRoot = root;

    m_nodes[root].version = m_currentVersion;

    VersionInfo vi;
    vi.rootIdx = root;
    vi.length = text.size();
    vi.editCount = 0;
    m_versions.append(vi);

    EditResult result;
    result.rootIdx = root;
    result.version = m_currentVersion;
    result.length = text.size();
    result.elapsedMs = timer.elapsed();

    m_stats.totalEdits++;
    m_stats.numVersions = m_versions.size();
    m_stats.totalLength = text.size();
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEdits;

    emit editDone(m_currentVersion, text.size(), result.elapsedMs);
    return result;
}

/* ---- Split rope at position ---- */

QPair<int, int> Rope13::split(int idx, int position)
{
    if (idx < 0 || position <= 0)
        return {-1, idx};

    int weight = computeWeight(idx);
    if (position >= weight)
        return {idx, -1};

    int cloned = cloneNode(idx);
    auto& node = m_nodes[cloned];

    if (node.isLeaf) {
        // Split leaf text
        int leftIdx = allocNode();
        int rightIdx = allocNode();

        m_nodes[leftIdx].isLeaf = true;
        m_nodes[leftIdx].text = node.text.left(position);
        m_nodes[leftIdx].weight = position;

        m_nodes[rightIdx].isLeaf = true;
        m_nodes[rightIdx].text = node.text.mid(position);
        m_nodes[rightIdx].weight = node.text.size() - position;

        return {leftIdx, rightIdx};
    }

    int leftWeight = computeWeight(node.leftIdx);
    if (position < leftWeight) {
        auto [l, r] = split(node.leftIdx, position);
        node.leftIdx = r;
        node.weight = computeWeight(r);
        return {l, cloned};
    } else {
        auto [l, r] = split(node.rightIdx, position - leftWeight);
        node.rightIdx = l;
        return {cloned, r};
    }
}

/* ---- Concatenate ---- */

int Rope13::concat(int leftIdx, int rightIdx)
{
    if (leftIdx < 0) return rightIdx;
    if (rightIdx < 0) return leftIdx;

    int idx = allocNode();
    m_nodes[idx].isLeaf = false;
    m_nodes[idx].leftIdx = leftIdx;
    m_nodes[idx].rightIdx = rightIdx;
    m_nodes[idx].weight = computeWeight(leftIdx);
    m_nodes[idx].version = m_currentVersion;
    return idx;
}

/* ---- Check if unbalanced ---- */

bool Rope13::isUnbalanced(int idx) const
{
    if (idx < 0 || idx >= m_nodes.size()) return false;
    const auto& node = m_nodes[idx];
    if (node.isLeaf) return false;

    int leftW = computeWeight(node.leftIdx);
    int rightW = computeWeight(node.rightIdx);
    int total = leftW + rightW;
    if (total == 0) return false;

    // Unbalanced if one side is < 20% of total
    double ratio = static_cast<double>(qMin(leftW, rightW)) / total;
    return ratio < 0.2;
}

/* ---- Rebalance ---- */

int Rope13::rebalance(int idx)
{
    if (!isUnbalanced(idx)) return idx;

    // Flatten and rebuild
    QString text;
    collectText(idx, text);
    return buildHelper(text, 0, text.size());
}

/* ---- Insert ---- */

Rope13::EditResult Rope13::insert(int rootIdx, int position, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    m_currentVersion++;

    auto [left, right] = split(rootIdx, position);
    int textNode = buildHelper(text, 0, text.size());
    int mid = concat(left, textNode);
    int newRoot = concat(mid, right);

    // Rebalance if needed
    newRoot = rebalance(newRoot);

    VersionInfo vi;
    vi.rootIdx = newRoot;
    vi.length = computeWeight(newRoot);
    vi.editCount = 1;
    m_versions.append(vi);

    EditResult result;
    result.rootIdx = newRoot;
    result.version = m_currentVersion;
    result.length = vi.length;
    result.elapsedMs = timer.elapsed();

    m_stats.totalEdits++;
    m_stats.numVersions = m_versions.size();
    m_stats.totalLength = vi.length;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEdits;

    emit editDone(m_currentVersion, result.length, result.elapsedMs);
    return result;
}

/* ---- Remove ---- */

Rope13::EditResult Rope13::remove(int rootIdx, int from, int to)
{
    QElapsedTimer timer;
    timer.start();

    m_currentVersion++;

    auto [left, _mid] = split(rootIdx, from);
    auto [_m2, right] = split(_mid, to - from);
    Q_UNUSED(_mid); Q_UNUSED(_m2);

    int newRoot = concat(left, right);
    newRoot = rebalance(newRoot);

    VersionInfo vi;
    vi.rootIdx = newRoot;
    vi.length = computeWeight(newRoot);
    vi.editCount = 1;
    m_versions.append(vi);

    EditResult result;
    result.rootIdx = newRoot;
    result.version = m_currentVersion;
    result.length = vi.length;
    result.elapsedMs = timer.elapsed();

    m_stats.totalEdits++;
    m_stats.numVersions = m_versions.size();
    m_stats.totalLength = vi.length;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEdits;

    emit editDone(m_currentVersion, result.length, result.elapsedMs);
    return result;
}

/* ---- toString ---- */

QString Rope13::toString(int rootIdx) const
{
    QString result;
    collectText(rootIdx, result);
    return result;
}

/* ---- at ---- */

QChar Rope13::at(int rootIdx, int index) const
{
    if (rootIdx < 0 || rootIdx >= m_nodes.size()) return QChar();
    const auto& node = m_nodes[rootIdx];

    if (node.isLeaf) {
        if (index >= 0 && index < node.text.size())
            return node.text[index];
        return QChar();
    }

    int leftW = computeWeight(node.leftIdx);
    if (index < leftW)
        return at(node.leftIdx, index);
    return at(node.rightIdx, index - leftW);
}

/* ---- substring ---- */

QString Rope13::substring(int rootIdx, int from, int to) const
{
    QString result;
    for (int i = from; i < to; ++i)
        result.append(at(rootIdx, i));
    return result;
}

/* ---- Reset ---- */

void Rope13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_versions.clear();
    m_currentRoot = -1;
    m_currentVersion = 0;
}
