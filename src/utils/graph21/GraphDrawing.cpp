/**
 * @file GraphDrawing.cpp
 * @brief 力导向图布局实现,基于Fruchterman-Reingold算法
 */

#include "GraphDrawing.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <random>

GraphDrawing::GraphDrawing(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<GraphDrawing::NodePosition> GraphDrawing::computeLayout(
    const QVector<Edge>& edges, int nodeCount,
    double width, double height, int iterations)
{
    QElapsedTimer timer;
    timer.start();

    QVector<NodePosition> nodes(nodeCount);

    if (nodeCount == 0) {
        m_timeSum += timer.elapsed();
        return nodes;
    }

    /* 随机初始化节点位置 */
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> xDist(0.0, width);
    std::uniform_real_distribution<double> yDist(0.0, height);

    for (int i = 0; i < nodeCount; ++i) {
        nodes[i].id = i;
        nodes[i].x = xDist(rng);
        nodes[i].y = yDist(rng);
        nodes[i].dispX = 0.0;
        nodes[i].dispY = 0.0;
    }

    double area = width * height;
    double k = idealEdgeLength(nodeCount, area);
    double initialTemp = width / 4.0;
    double temperature = initialTemp;

    /* 构建邻接集合用于斥力计算 */
    for (int iter = 0; iter < iterations; ++iter) {
        /* 重置位移 */
        for (auto& n : nodes) {
            n.dispX = 0.0;
            n.dispY = 0.0;
        }

        /* 计算斥力: 所有节点对之间 */
        applyRepulsive(nodes, k);

        /* 计算引力: 沿边方向 */
        applyAttractive(nodes, edges, k);

        /* 中心引力: 防止图漂移 */
        applyGravity(nodes, width / 2.0, height / 2.0, 0.1);

        /* 根据温度限制位移并更新位置 */
        for (auto& n : nodes) {
            double dispMag = std::sqrt(n.dispX * n.dispX + n.dispY * n.dispY);
            if (dispMag > 1e-10) {
                double limitedDisp = std::min(dispMag, temperature);
                n.x += (n.dispX / dispMag) * limitedDisp;
                n.y += (n.dispY / dispMag) * limitedDisp;
            }

            /* 限制在画布内 */
            double margin = 10.0;
            n.x = qBound(margin, n.x, width - margin);
            n.y = qBound(margin, n.y, height - margin);
        }

        /* 退火 */
        temperature = coolTemperature(temperature, initialTemp, iter, iterations);

        /* 计算当前能量用于进度报告 */
        double energy = computeEnergy(nodes, edges, k);
        emit iterationProgress(iter, energy);
    }

    double finalEnergy = computeEnergy(nodes, edges, k);
    m_stats.totalLayouts++;
    m_stats.totalIterations += iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalLayouts;

    emit layoutCompleted(nodeCount, finalEnergy);
    return nodes;
}

QVector<GraphDrawing::NodePosition> GraphDrawing::refineLayout(
    const QVector<NodePosition>& currentPositions,
    const QVector<Edge>& edges,
    double width, double height,
    int iterations)
{
    QElapsedTimer timer;
    timer.start();

    QVector<NodePosition> nodes = currentPositions;
    int nodeCount = nodes.size();
    if (nodeCount == 0) {
        m_timeSum += timer.elapsed();
        return nodes;
    }

    double area = width * height;
    double k = idealEdgeLength(nodeCount, area);
    double initialTemp = width / 20.0;
    double temperature = initialTemp;

    for (int iter = 0; iter < iterations; ++iter) {
        for (auto& n : nodes) {
            n.dispX = 0.0;
            n.dispY = 0.0;
        }

        applyRepulsive(nodes, k);
        applyAttractive(nodes, edges, k);
        applyGravity(nodes, width / 2.0, height / 2.0, 0.05);

        for (auto& n : nodes) {
            double dispMag = std::sqrt(n.dispX * n.dispX + n.dispY * n.dispY);
            if (dispMag > 1e-10) {
                double limitedDisp = std::min(dispMag, temperature);
                n.x += (n.dispX / dispMag) * limitedDisp;
                n.y += (n.dispY / dispMag) * limitedDisp;
            }
            n.x = qBound(10.0, n.x, width - 10.0);
            n.y = qBound(10.0, n.y, height - 10.0);
        }

        temperature = coolTemperature(temperature, initialTemp, iter, iterations);
    }

    m_stats.totalLayouts++;
    m_stats.totalIterations += iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalLayouts;

    return nodes;
}

double GraphDrawing::computeEnergy(const QVector<NodePosition>& positions,
                                   const QVector<Edge>& edges,
                                   double idealDist) const
{
    int n = positions.size();
    double energy = 0.0;

    /* 斥力能量: 节点对之间 */
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dx = positions[i].x - positions[j].x;
            double dy = positions[i].y - positions[j].y;
            double dist = std::sqrt(dx * dx + dy * dy);
            dist = qMax(dist, 1.0);
            double repForce = (idealDist * idealDist) / dist;
            energy += repForce * repForce;
        }
    }

    /* 引力能量: 沿边方向 */
    for (const auto& e : edges) {
        if (e.source < 0 || e.source >= n || e.target < 0 || e.target >= n)
            continue;
        double dx = positions[e.source].x - positions[e.target].x;
        double dy = positions[e.source].y - positions[e.target].y;
        double dist = std::sqrt(dx * dx + dy * dy);
        dist = qMax(dist, 1.0);
        double attForce = (dist * dist) / idealDist * e.weight;
        energy += attForce * attForce;
    }

    return energy;
}

double GraphDrawing::idealEdgeLength(int nodeCount, double area) const
{
    if (nodeCount <= 1) return 1.0;
    return std::sqrt(area / nodeCount);
}

void GraphDrawing::applyRepulsive(QVector<NodePosition>& nodes,
                                  double k) const
{
    int n = nodes.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dx = nodes[i].x - nodes[j].x;
            double dy = nodes[i].y - nodes[j].y;
            double dist = std::sqrt(dx * dx + dy * dy);
            dist = qMax(dist, 0.01);

            /* Fruchterman-Reingold斥力: f_r = k^2 / d */
            double force = (k * k) / dist;

            double fx = (dx / dist) * force;
            double fy = (dy / dist) * force;

            nodes[i].dispX += fx;
            nodes[i].dispY += fy;
            nodes[j].dispX -= fx;
            nodes[j].dispY -= fy;
        }
    }
}

void GraphDrawing::applyAttractive(QVector<NodePosition>& nodes,
                                   const QVector<Edge>& edges,
                                   double k) const
{
    for (const auto& e : edges) {
        if (e.source < 0 || e.source >= nodes.size() ||
            e.target < 0 || e.target >= nodes.size())
            continue;

        double dx = nodes[e.source].x - nodes[e.target].x;
        double dy = nodes[e.source].y - nodes[e.target].y;
        double dist = std::sqrt(dx * dx + dy * dy);
        dist = qMax(dist, 0.01);

        /* Fruchterman-Reingold引力: f_a = d^2 / k */
        double force = (dist * dist) / k * e.weight;

        double fx = (dx / dist) * force;
        double fy = (dy / dist) * force;

        nodes[e.source].dispX -= fx;
        nodes[e.source].dispY -= fy;
        nodes[e.target].dispX += fx;
        nodes[e.target].dispY += fy;
    }
}

void GraphDrawing::applyGravity(QVector<NodePosition>& nodes,
                                double cx, double cy,
                                double strength) const
{
    for (auto& n : nodes) {
        double dx = cx - n.x;
        double dy = cy - n.y;
        n.dispX += dx * strength;
        n.dispY += dy * strength;
    }
}

double GraphDrawing::coolTemperature(double temp, double initialTemp,
                                     int iteration, int maxIter) const
{
    /* 线性退火 */
    double ratio = static_cast<double>(maxIter - iteration) / maxIter;
    return initialTemp * ratio;
}

GraphDrawing::Stats GraphDrawing::stats() const { return m_stats; }

void GraphDrawing::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
