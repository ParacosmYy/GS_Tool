/**
 * @file TravelingSalesman2.cpp
 * @brief 旅行商问题2实现 — 2-opt+3-opt+LK局部搜索
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph56/TravelingSalesman2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
TravelingSalesman2::TravelingSalesman2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("TravelingSalesman2"));
}

/**
 * @brief 设置城市坐标
 *
 * 从二维坐标计算欧氏距离矩阵。
 *
 * @param coordinates 城市坐标列表 (x, y)
 */
void TravelingSalesman2::setCities(const QVector<QPair<double,double>>& coordinates)
{
    m_n = coordinates.size();
    m_dist.assign(m_n * m_n, 0.0);

    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) {
            double dx = coordinates[i].first - coordinates[j].first;
            double dy = coordinates[i].second - coordinates[j].second;
            m_dist[i * m_n + j] = qSqrt(dx * dx + dy * dy);
        }
    }
}

/**
 * @brief 设置预计算的距离矩阵
 * @param dist 距离矩阵（行优先，n*n）
 * @param n 城市数量
 */
void TravelingSalesman2::setDistanceMatrix(const QVector<double>& dist, int n)
{
    m_n = n;
    m_dist = dist;
}

/**
 * @brief 求解TSP
 *
 * 构造初始解（最近邻启发式），然后依次应用
 * 2-opt、3-opt和Or-opt局部搜索优化。
 *
 * @return 最优路径（城市索引序列）
 */
QVector<int> TravelingSalesman2::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 1) return {};

    m_stats.totalNodes = m_n;

    /* 初始解：最近邻启发式 */
    QVector<int> tour = nearestNeighborStart();

    /* 局部搜索 */
    twoOpt(tour);
    threeOpt(tour);
    orOpt(tour);

    m_bestTour = tour;
    m_bestLength = tourLength(tour);

    m_stats.totalSolves++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_n, m_bestLength, m_stats.totalImprovements);
    return m_bestTour;
}

/**
 * @brief 计算给定路径的总长度
 *
 * 包含回到起点的闭合距离。
 *
 * @param tour 路径序列
 * @return 路径总长度
 */
double TravelingSalesman2::tourLength(const QVector<int>& tour) const
{
    if (tour.isEmpty()) return 0.0;
    double len = 0.0;
    for (int i = 0; i < tour.size(); ++i) {
        int j = (i + 1) % tour.size();
        if (tour[i] >= 0 && tour[i] < m_n && tour[j] >= 0 && tour[j] < m_n) {
            len += m_dist[tour[i] * m_n + tour[j]];
        }
    }
    return len;
}

/**
 * @brief 对初始路径进行改进
 *
 * 在给定初始路径基础上执行2-opt、3-opt和Or-opt优化。
 *
 * @param initialTour 初始路径
 * @return 优化后的路径
 */
QVector<int> TravelingSalesman2::improve(const QVector<int>& initialTour)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> tour = initialTour;

    twoOpt(tour);
    threeOpt(tour);
    orOpt(tour);

    double len = tourLength(tour);
    if (len < m_bestLength || m_bestLength == 0.0) {
        m_bestTour = tour;
        m_bestLength = len;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalSolves);

    return tour;
}

/**
 * @brief 最近邻启发式构造初始解
 *
 * 从城市0开始，每次选择最近的未访问城市。
 *
 * @return 初始路径
 */
QVector<int> TravelingSalesman2::nearestNeighborStart() const
{
    QVector<int> tour;
    QVector<bool> visited(m_n, false);

    int current = 0;
    tour.append(current);
    visited[current] = true;

    for (int step = 1; step < m_n; ++step) {
        double bestDist = std::numeric_limits<double>::max();
        int bestNext = -1;

        for (int j = 0; j < m_n; ++j) {
            if (visited[j]) continue;
            double d = m_dist[current * m_n + j];
            if (d < bestDist) {
                bestDist = d;
                bestNext = j;
            }
        }

        if (bestNext >= 0) {
            tour.append(bestNext);
            visited[bestNext] = true;
            current = bestNext;
        }
    }

    return tour;
}

/**
 * @brief 2-opt局部搜索
 *
 * 反转路径中的一段子路径，如果总长度减少则接受。
 * 重复直到无法进一步改进。
 *
 * @param tour 路径（会被修改）
 * @return 改进的距离量
 */
double TravelingSalesman2::twoOpt(QVector<int>& tour)
{
    double totalImprovement = 0.0;
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < m_n - 1; ++i) {
            for (int j = i + 2; j < m_n; ++j) {
                if (i == 0 && j == m_n - 1) continue;

                int a = tour[i], b = tour[i + 1];
                int c = tour[j], d = tour[(j + 1) % m_n];

                double before = m_dist[a * m_n + b] + m_dist[c * m_n + d];
                double after = m_dist[a * m_n + c] + m_dist[b * m_n + d];

                if (after < before - 1e-10) {
                    /* 反转 i+1 到 j 之间的段 */
                    std::reverse(tour.begin() + i + 1, tour.begin() + j + 1);
                    totalImprovement += before - after;
                    improved = true;
                    m_stats.totalImprovements++;
                }
            }
        }
    }

    return totalImprovement;
}

/**
 * @brief 3-opt局部搜索
 *
 * 尝试断开三条边并重新连接，选择最优重连方式。
 * 在2-opt基础上进一步优化解质量。
 *
 * @param tour 路径（会被修改）
 * @return 改进的距离量
 */
double TravelingSalesman2::threeOpt(QVector<int>& tour)
{
    double totalImprovement = 0.0;
    bool improved = true;
    int maxIter = 10;

    while (improved && maxIter-- > 0) {
        improved = false;
        for (int i = 0; i < m_n - 3; ++i) {
            for (int j = i + 2; j < m_n - 1; ++j) {
                for (int k = j + 2; k < m_n; ++k) {
                    int a = tour[i], b = tour[i + 1];
                    int c = tour[j], d = tour[j + 1];
                    int e = tour[k], f = tour[(k + 1) % m_n];

                    double d0 = m_dist[a * m_n + b] + m_dist[c * m_n + d] +
                                m_dist[e * m_n + f];

                    /* 尝试多种重连方式 */
                    double d1 = m_dist[a * m_n + c] + m_dist[b * m_n + e] +
                                m_dist[d * m_n + f];
                    double d2 = m_dist[a * m_n + d] + m_dist[e * m_n + b] +
                                m_dist[c * m_n + f];
                    double d3 = m_dist[a * m_n + e] + m_dist[d * m_n + b] +
                                m_dist[c * m_n + f];

                    double bestD = qMin({d1, d2, d3});
                    if (bestD < d0 - 1e-10) {
                        if (bestD == d1) {
                            std::reverse(tour.begin() + i + 1, tour.begin() + j + 1);
                            std::reverse(tour.begin() + j + 1, tour.begin() + k + 1);
                        } else if (bestD == d2) {
                            QVector<int> newTour;
                            for (int x = 0; x <= i; ++x) newTour.append(tour[x]);
                            for (int x = j + 1; x <= k; ++x) newTour.append(tour[x]);
                            for (int x = i + 1; x <= j; ++x) newTour.append(tour[x]);
                            for (int x = k + 1; x < m_n; ++x) newTour.append(tour[x]);
                            tour = newTour;
                        } else {
                            std::reverse(tour.begin() + i + 1, tour.begin() + k + 1);
                        }

                        totalImprovement += d0 - bestD;
                        improved = true;
                        m_stats.totalImprovements++;
                        break;
                    }
                }
                if (improved) break;
            }
            if (improved) break;
        }
    }

    return totalImprovement;
}

/**
 * @brief Or-opt局部搜索
 *
 * 将1-3个连续城市的子段从当前位置移除，
 * 尝试插入到路径的其他位置。
 *
 * @param tour 路径（会被修改）
 * @return 改进的距离量
 */
double TravelingSalesman2::orOpt(QVector<int>& tour)
{
    double totalImprovement = 0.0;
    bool improved = true;

    while (improved) {
        improved = false;
        for (int segLen = 3; segLen >= 1; --segLen) {
            for (int i = 0; i < m_n; ++i) {
                if (i + segLen > m_n) continue;

                int prev = (i - 1 + m_n) % m_n;
                int next = (i + segLen) % m_n;

                /* 移除段i..i+segLen-1的开销 */
                double removeCost = m_dist[tour[prev] * m_n + tour[i]] +
                    m_dist[tour[i + segLen - 1] * m_n + tour[next]];
                double removeGain = m_dist[tour[prev] * m_n + tour[next]];

                /* 尝试插入到位置j */
                for (int j = 0; j < m_n; ++j) {
                    if (j >= i && j <= i + segLen) continue;
                    int jNext = (j + 1) % m_n;

                    double insertCost = m_dist[tour[j] * m_n + tour[i]] +
                        m_dist[tour[i + segLen - 1] * m_n + tour[jNext]];
                    double insertGain = m_dist[tour[j] * m_n + tour[jNext]];

                    double delta = (removeGain - removeCost) + (insertGain - insertCost);
                    if (delta > 1e-10) {
                        /* 执行Or-opt移动 */
                        QVector<int> seg;
                        for (int s = 0; s < segLen; ++s) seg.append(tour[i + s]);

                        QVector<int> newTour;
                        for (int x = 0; x < m_n; ++x) {
                            if (x >= i && x < i + segLen) continue;
                            newTour.append(tour[x]);
                        }

                        int insertPos = (j > i) ? j - segLen + 1 : j + 1;
                        insertPos = qBound(0, insertPos, newTour.size());
                        for (int s = 0; s < segLen; ++s) {
                            newTour.insert(newTour.begin() + insertPos + s, seg[s]);
                        }

                        tour = newTour;
                        totalImprovement += delta;
                        improved = true;
                        m_stats.totalImprovements++;
                        break;
                    }
                }
                if (improved) break;
            }
            if (improved) break;
        }
    }

    return totalImprovement;
}

/**
 * @brief 2-opt单次交换
 *
 * 执行一次2-opt交换操作。
 *
 * @param tour 路径
 * @param i 交换位置1
 * @param j 交换位置2
 * @return 是否改进
 */
bool TravelingSalesman2::twoOptSwap(QVector<int>& tour, int i, int j)
{
    int a = tour[i], b = tour[(i + 1) % m_n];
    int c = tour[j], d = tour[(j + 1) % m_n];

    double before = m_dist[a * m_n + b] + m_dist[c * m_n + d];
    double after = m_dist[a * m_n + c] + m_dist[b * m_n + d];

    if (after < before - 1e-10) {
        std::reverse(tour.begin() + i + 1, tour.begin() + j + 1);
        return true;
    }
    return false;
}

/**
 * @brief 重置所有统计数据
 */
void TravelingSalesman2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
