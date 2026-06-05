/**
 * @file OctreeVolume.cpp
 * @brief 八叉树三维空间索引实现
 */

#include "utils/octree/OctreeVolume.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param maxDepth 最大递归深度 @param parent 父对象 */
OctreeVolume::OctreeVolume(int maxDepth, QObject* parent)
    : QObject(parent)
    , m_maxDepth(maxDepth)
    , m_root(nullptr)
    , m_timeSum(0.0)
{
    /* 初始根节点覆盖较大空间 */
    m_root = createNode(0.0, 0.0, 0.0, 1e6);
}

/** @brief 析构函数 */
OctreeVolume::~OctreeVolume()
{
    destroyNode(m_root);
}

/**
 * @brief 插入三维数据点
 *
 * 从根节点递归定位到包含该点的叶子节点。
 * 若叶子节点已满(超过maxPointsPerNode)且未达到最大深度，
 * 则分裂为八个子节点并重新分配点。
 */
bool OctreeVolume::insert(double x, double y, double z, int data)
{
    QElapsedTimer timer;
    timer.start();

    bool ok = false;
    if (m_root) {
        /* 先检查点是否在根节点范围内 */
        double hs = m_root->halfSize;
        if (x < m_root->cx - hs || x > m_root->cx + hs ||
            y < m_root->cy - hs || y > m_root->cy + hs ||
            z < m_root->cz - hs || z > m_root->cz + hs) {
            /* 扩展根节点范围 */
            double newHalf = std::max({std::abs(x), std::abs(y), std::abs(z)}) * 2.0;
            newHalf = std::max(newHalf, m_root->halfSize * 2.0);
            destroyNode(m_root);
            m_root = createNode(0.0, 0.0, 0.0, newHalf);
        }
        ok = insertNode(m_root, x, y, z, data, 0);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalInserts;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return ok;
}

/**
 * @brief 三维范围查询
 *
 * 递归遍历八叉树，检查每个子节点与查询AABB的相交关系。
 * 相交的叶子节点中的所有点逐一检查是否在查询范围内。
 */
QVector<int> OctreeVolume::rangeQuery(
    double xMin, double yMin, double zMin,
    double xMax, double yMax, double zMax)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (m_root) {
        queryNode(m_root, xMin, yMin, zMin, xMax, yMax, zMax, result);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit queryCompleted(result.size());
    return result;
}

/** @brief 重置统计信息 */
void OctreeVolume::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 创建八叉树节点 @param cx 中心X @param cy 中心Y @param cz 中心Z @param halfSize 半尺寸 */
OctreeVolume::OctNode* OctreeVolume::createNode(
    double cx, double cy, double cz, double halfSize)
{
    OctNode* node = new OctNode;
    node->cx = cx;
    node->cy = cy;
    node->cz = cz;
    node->halfSize = halfSize;
    node->isLeaf = true;
    for (int i = 0; i < 8; ++i) {
        node->children[i] = nullptr;
    }
    return node;
}

/**
 * @brief 递归插入点
 *
 * 定位到叶子节点后添加点。若溢出则分裂。
 */
bool OctreeVolume::insertNode(OctNode* node, double x, double y,
                               double z, int data, int depth)
{
    if (!node) return false;

    if (node->isLeaf) {
        node->points.append({x, y, z, data});

        /* 超过容量且未达到最大深度则分裂 */
        if (static_cast<int>(node->points.size()) > s_maxPointsPerNode &&
            depth < m_maxDepth) {
            subdivide(node);

            /* 重新插入所有点到子节点 */
            QVector<Point3D> oldPoints;
            oldPoints.swap(node->points);

            for (const Point3D& pt : oldPoints) {
                int octant = 0;
                if (pt.x >= node->cx) octant |= 1;
                if (pt.y >= node->cy) octant |= 2;
                if (pt.z >= node->cz) octant |= 4;
                insertNode(node->children[octant], pt.x, pt.y,
                           pt.z, pt.data, depth + 1);
            }
        }
        return true;
    }

    /* 内部节点: 定位到对应子节点 */
    int octant = 0;
    if (x >= node->cx) octant |= 1;
    if (y >= node->cy) octant |= 2;
    if (z >= node->cz) octant |= 4;
    return insertNode(node->children[octant], x, y, z, data, depth + 1);
}

/**
 * @brief 分裂节点为八个子节点
 *
 * 根据中心坐标将空间分为八个象限。
 */
void OctreeVolume::subdivide(OctNode* node)
{
    double qs = node->halfSize * 0.5; /* 子节点半尺寸 */

    for (int i = 0; i < 8; ++i) {
        double ox = (i & 1) ? qs : -qs;
        double oy = (i & 2) ? qs : -qs;
        double oz = (i & 4) ? qs : -qs;
        node->children[i] = createNode(
            node->cx + ox, node->cy + oy, node->cz + oz, qs);
    }
    node->isLeaf = false;
}

/**
 * @brief 递归范围查询
 *
 * 检查节点AABB与查询AABB是否相交。
 * 不相交则剪枝，叶子节点则逐一检查点。
 */
void OctreeVolume::queryNode(OctNode* node,
                              double xMin, double yMin, double zMin,
                              double xMax, double yMax, double zMax,
                              QVector<int>& result)
{
    if (!node) return;

    /* AABB相交测试 */
    double hs = node->halfSize;
    if (node->cx + hs < xMin || node->cx - hs > xMax ||
        node->cy + hs < yMin || node->cy - hs > yMax ||
        node->cz + hs < zMin || node->cz - hs > zMax) {
        return; /* 不相交，剪枝 */
    }

    if (node->isLeaf) {
        /* 检查每个点是否在查询范围内 */
        for (const Point3D& pt : node->points) {
            if (pt.x >= xMin && pt.x <= xMax &&
                pt.y >= yMin && pt.y <= yMax &&
                pt.z >= zMin && pt.z <= zMax) {
                result.append(pt.data);
            }
        }
        return;
    }

    /* 递归搜索子节点 */
    for (int i = 0; i < 8; ++i) {
        queryNode(node->children[i], xMin, yMin, zMin,
                  xMax, yMax, zMax, result);
    }
}

/** @brief 递归销毁节点 */
void OctreeVolume::destroyNode(OctNode* node)
{
    if (!node) return;
    for (int i = 0; i < 8; ++i) {
        destroyNode(node->children[i]);
    }
    delete node;
}
