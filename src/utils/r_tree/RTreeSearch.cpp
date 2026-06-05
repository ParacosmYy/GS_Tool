/**
 * @file RTreeSearch.cpp
 * @brief R树空间索引实现
 */

#include "utils/r_tree/RTreeSearch.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param maxEntries 每个节点最大条目数 @param parent 父对象 */
RTreeSearch::RTreeSearch(int maxEntries, QObject* parent)
    : QObject(parent)
    , m_maxEntries(maxEntries)
    , m_root(nullptr)
    , m_timeSum(0.0)
{
    m_root = createNode(true);
}

/** @brief 析构函数 */
RTreeSearch::~RTreeSearch()
{
    destroyNode(m_root);
}

/**
 * @brief 插入矩形数据
 *
 * 选择面积增长最小的子节点递归插入，
 * 在叶子节点中添加条目后向上更新MBR。
 */
void RTreeSearch::insert(double xMin, double yMin, double xMax, double yMax,
                          int data)
{
    QElapsedTimer timer;
    timer.start();

    BBox entry{xMin, yMin, xMax, yMax, data};

    /* 仅根节点且为空 */
    if (m_root->entries.isEmpty() && m_root->children.isEmpty()) {
        m_root->entries.append(entry);
        updateMBR(m_root);
    } else if (m_root->isLeaf) {
        m_root->entries.append(entry);
        updateMBR(m_root);

        /* 分裂根节点 */
        if (m_root->entries.size() > m_maxEntries) {
            Node* newRoot = createNode(false);
            newRoot->children.append(m_root);

            /* 创建兄弟节点并分裂条目 */
            Node* sibling = createNode(true);
            int mid = m_root->entries.size() / 2;
            for (int i = mid; i < m_root->entries.size(); ++i) {
                sibling->entries.append(m_root->entries[i]);
            }
            m_root->entries.resize(mid);

            updateMBR(m_root);
            updateMBR(sibling);
            newRoot->children.append(sibling);
            updateMBR(newRoot);
            m_root = newRoot;
        }
    } else {
        /* 选择最佳子节点插入 */
        Node* target = chooseSubtree(m_root, xMin, yMin, xMax, yMax);
        target->entries.append(entry);
        updateMBR(target);

        /* 节点溢出分裂 */
        if (target->entries.size() > m_maxEntries) {
            Node* sibling = createNode(true);
            int mid = target->entries.size() / 2;
            for (int i = mid; i < target->entries.size(); ++i) {
                sibling->entries.append(target->entries[i]);
            }
            target->entries.resize(mid);
            updateMBR(target);
            updateMBR(sibling);
            m_root->children.append(sibling);
            updateMBR(m_root);
        }
        updateMBR(m_root);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalInserts;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalSearches);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
}

/**
 * @brief 范围查询
 *
 * 递归遍历R树，检查节点MBR与查询矩形的相交关系，
 * 在叶子节点中检查每个条目的精确相交。
 */
QVector<int> RTreeSearch::search(double xMin, double yMin,
                                  double xMax, double yMax)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    searchNode(m_root, xMin, yMin, xMax, yMax, result);

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSearches;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalSearches);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit searchCompleted(result.size());
    return result;
}

/** @brief 重置统计信息 */
void RTreeSearch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 创建新节点 @param isLeaf 是否为叶子节点 */
RTreeSearch::Node* RTreeSearch::createNode(bool isLeaf)
{
    Node* node = new Node;
    node->xMin = 1e18;
    node->yMin = 1e18;
    node->xMax = -1e18;
    node->yMax = -1e18;
    node->isLeaf = isLeaf;
    return node;
}

/** @brief 销毁节点及子节点 */
void RTreeSearch::destroyNode(Node* node)
{
    if (!node) return;
    for (Node* child : node->children) {
        destroyNode(child);
    }
    delete node;
}

/** @brief 更新节点MBR */
void RTreeSearch::updateMBR(Node* node)
{
    node->xMin = 1e18;
    node->yMin = 1e18;
    node->xMax = -1e18;
    node->yMax = -1e18;

    for (const BBox& e : node->entries) {
        node->xMin = std::min(node->xMin, e.xMin);
        node->yMin = std::min(node->yMin, e.yMin);
        node->xMax = std::max(node->xMax, e.xMax);
        node->yMax = std::max(node->yMax, e.yMax);
    }

    for (Node* child : node->children) {
        node->xMin = std::min(node->xMin, child->xMin);
        node->yMin = std::min(node->yMin, child->yMin);
        node->xMax = std::max(node->xMax, child->xMax);
        node->yMax = std::max(node->yMax, child->yMax);
    }
}

/**
 * @brief 选择插入的子节点
 *
 * 选择面积增长最小的子节点。若增长相同则选面积最小的。
 */
RTreeSearch::Node* RTreeSearch::chooseSubtree(
    Node* node, double xMin, double yMin, double xMax, double yMax)
{
    if (node->children.isEmpty()) return node;

    Node* best = nullptr;
    double bestEnl = 1e18;
    double bestArea = 1e18;

    for (Node* child : node->children) {
        double enl = enlargement(child, xMin, yMin, xMax, yMax);
        double area = (child->xMax - child->xMin) * (child->yMax - child->yMin);

        if (enl < bestEnl || (enl == bestEnl && area < bestArea)) {
            bestEnl = enl;
            bestArea = area;
            best = child;
        }
    }

    return best ? chooseSubtree(best, xMin, yMin, xMax, yMax) : node;
}

/** @brief 递归范围查询 */
void RTreeSearch::searchNode(Node* node, double xMin, double yMin,
                              double xMax, double yMax, QVector<int>& result)
{
    if (!node) return;

    /* 检查MBR是否与查询范围相交 */
    if (node->xMax < xMin || node->xMin > xMax ||
        node->yMax < yMin || node->yMin > yMax) {
        return;
    }

    /* 叶子节点: 检查每个条目 */
    for (const BBox& e : node->entries) {
        if (!(e.xMax < xMin || e.xMin > xMax ||
              e.yMax < yMin || e.yMin > yMax)) {
            result.append(e.data);
        }
    }

    /* 内部节点: 递归搜索子节点 */
    for (Node* child : node->children) {
        searchNode(child, xMin, yMin, xMax, yMax, result);
    }
}

/**
 * @brief 计算插入给定矩形后的MBR面积增长
 * @param node 当前节点
 * @param xMin 矩形最小X
 * @param yMin 矩形最小Y
 * @param xMax 矩形最大X
 * @param yMax 矩形最大Y
 * @return 面积增长量
 */
double RTreeSearch::enlargement(const Node* node, double xMin, double yMin,
                                 double xMax, double yMax) const
{
    double oldArea = (node->xMax - node->xMin) * (node->yMax - node->yMin);
    double newXMin = std::min(node->xMin, xMin);
    double newYMin = std::min(node->yMin, yMin);
    double newXMax = std::max(node->xMax, xMax);
    double newYMax = std::max(node->yMax, yMax);
    double newArea = (newXMax - newXMin) * (newYMax - newYMin);
    return newArea - oldArea;
}
