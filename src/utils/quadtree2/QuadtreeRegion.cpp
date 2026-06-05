/**
 * @file QuadtreeRegion.cpp
 * @brief 区域四叉树实现
 */

#include "utils/quadtree2/QuadtreeRegion.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
QuadtreeRegion::QuadtreeRegion(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_nodeCount(0)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 */
QuadtreeRegion::~QuadtreeRegion()
{
    destroyNode(m_root);
}

/**
 * @brief 从网格构建区域四叉树
 *
 * 递归将网格分割为四个象限。当区域内值的方差小于阈值，
 * 或者区域尺寸为1x1时，创建叶子节点。
 * 否则继续递归分割。
 */
void QuadtreeRegion::build(const QVector<QVector<double>>& grid,
                            double threshold)
{
    QElapsedTimer timer;
    timer.start();

    destroyNode(m_root);
    m_root = nullptr;
    m_nodeCount = 0;

    int rows = grid.size();
    int cols = (rows > 0) ? grid[0].size() : 0;

    if (rows > 0 && cols > 0) {
        m_root = buildNode(grid, 0, 0, cols, rows, threshold);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalBuilds;
    double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit buildCompleted(m_nodeCount);
}

/**
 * @brief 查询指定位置的值
 *
 * 从根节点开始，根据坐标定位到包含该点的叶子节点，
 * 返回其代表值。
 */
double QuadtreeRegion::queryValue(int x, int y)
{
    QElapsedTimer timer;
    timer.start();

    double val = 0.0;
    if (m_root) {
        val = queryNode(m_root, x, y);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return val;
}

/** @brief 重置统计信息 */
void QuadtreeRegion::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 递归构建四叉树节点
 *
 * 计算区域的方差和均值。若方差小于阈值或区域为最小尺寸，
 * 创建叶子节点。否则将区域四分并递归构建子节点。
 */
QuadtreeRegion::QNode* QuadtreeRegion::buildNode(
    const QVector<QVector<double>>& grid,
    int x, int y, int w, int h, double threshold)
{
    QNode* node = new QNode;
    node->x = x;
    node->y = y;
    node->width = w;
    node->height = h;
    node->isLeaf = true;
    for (int i = 0; i < 4; ++i) {
        node->children[i] = nullptr;
    }

    ++m_nodeCount;

    /* 计算区域方差和均值 */
    double mean = 0.0;
    double variance = regionVariance(grid, x, y, w, h, mean);

    /* 叶子条件: 区域为1x1，或方差低于阈值 */
    if ((w <= 1 && h <= 1) || variance <= threshold) {
        node->value = mean;
        node->isLeaf = true;
        return node;
    }

    /* 分割为四个子区域: NW, NE, SW, SE */
    node->isLeaf = false;
    int hw = w / 2;
    int hh = h / 2;
    int rw = w - hw; /* 右半宽 */
    int rh = h - hh; /* 下半高 */

    /* NW: (x, y, hw, hh) */
    node->children[0] = buildNode(grid, x, y, hw, hh, threshold);
    /* NE: (x+hw, y, rw, hh) */
    node->children[1] = buildNode(grid, x + hw, y, rw, hh, threshold);
    /* SW: (x, y+hh, hw, rh) */
    node->children[2] = buildNode(grid, x, y + hh, hw, rh, threshold);
    /* SE: (x+hw, y+hh, rw, rh) */
    node->children[3] = buildNode(grid, x + hw, y + hh, rw, rh, threshold);

    return node;
}

/**
 * @brief 递归查询节点值
 *
 * 若当前节点为叶子节点则直接返回代表值。
 * 否则根据坐标定位到对应的子节点继续查询。
 */
double QuadtreeRegion::queryNode(QNode* node, int x, int y)
{
    if (!node) return 0.0;

    /* 叶子节点直接返回代表值 */
    if (node->isLeaf) {
        return node->value;
    }

    /* 确定坐标属于哪个子象限 */
    int hw = node->width / 2;
    int hh = node->height / 2;
    int idx = 0;
    if (x >= node->x + hw) idx += 1;  /* 东 */
    if (y >= node->y + hh) idx += 2;  /* 南 */

    return queryNode(node->children[idx], x, y);
}

/**
 * @brief 计算区域的均值和方差
 * @param grid 数据网格
 * @param x 起始列
 * @param y 起始行
 * @param w 宽度
 * @param h 高度
 * @param mean 输出均值
 * @return 方差
 */
double QuadtreeRegion::regionVariance(
    const QVector<QVector<double>>& grid,
    int x, int y, int w, int h, double& mean) const
{
    double sum = 0.0;
    double sumSq = 0.0;
    int count = 0;

    int yEnd = std::min(y + h, static_cast<int>(grid.size()));
    for (int row = y; row < yEnd; ++row) {
        int xEnd = std::min(x + w, static_cast<int>(grid[row].size()));
        for (int col = x; col < xEnd; ++col) {
            double val = grid[row][col];
            sum += val;
            sumSq += val * val;
            ++count;
        }
    }

    if (count == 0) {
        mean = 0.0;
        return 0.0;
    }

    mean = sum / count;
    double variance = (sumSq / count) - (mean * mean);
    return (variance > 0.0) ? variance : 0.0;
}

/** @brief 递归销毁节点 */
void QuadtreeRegion::destroyNode(QNode* node)
{
    if (!node) return;
    for (int i = 0; i < 4; ++i) {
        destroyNode(node->children[i]);
    }
    delete node;
}
