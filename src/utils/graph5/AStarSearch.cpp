/**
 * @file AStarSearch.cpp
 * @brief A*图搜索算法实现 — 启发式最短路径搜索
 */

#include "utils/graph5/AStarSearch.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
AStarSearch::AStarSearch(QObject* parent)
    : QObject(parent)
    , m_bidirectional(false)
    , m_timeSum(0.0)
    , m_nodesVisitedSum(0.0)
    , m_pathLengthSum(0.0)
{
}

/** @brief 设置图结构
 *  @param nodes 节点名称列表
 *  @param edges 边列表: 每项包含起点、终点和权重
 */
void AStarSearch::setGraph(const QVector<QString>& nodes,
                           const QVector<Edge>& edges)
{
    m_adjForward.clear();
    m_adjBackward.clear();
    m_nodeExists.clear();

    /* 注册节点 */
    for (const QString& node : nodes) {
        m_nodeExists[node] = true;
    }

    /* 构建正向和反向邻接表 */
    for (const Edge& edge : edges) {
        double weight = qMax(0.0, edge.weight);
        m_nodeExists[edge.from] = true;
        m_nodeExists[edge.to] = true;

        m_adjForward[edge.from][edge.to] = weight;
        m_adjBackward[edge.to][edge.from] = weight;
    }
}

/** @brief 设置启发函数
 *  @param fn 启发函数
 */
void AStarSearch::setHeuristic(
    std::function<double(const QString&, const QString&)> fn)
{
    m_heuristic = std::move(fn);
}

/** @brief 启用/禁用双向搜索
 *  @param enabled 是否启用
 */
void AStarSearch::setBidirectional(bool enabled)
{
    m_bidirectional = enabled;
}

/** @brief 重建路径
 *  @param cameFrom 前驱映射
 *  @param current 终点
 *  @return 路径列表
 */
QVector<QString> AStarSearch::reconstructPath(
    const QHash<QString, QString>& cameFrom,
    const QString& current) const
{
    QVector<QString> path;
    QString node = current;
    while (!node.isEmpty()) {
        path.prepend(node);
        auto it = cameFrom.find(node);
        if (it != cameFrom.end()) {
            node = it.value();
        } else {
            break;
        }
    }
    return path;
}

/** @brief 单向A*搜索核心
 *  @param start 起点
 *  @param goal 终点
 *  @param forward 正向/反向
 *  @return {路径, 代价}
 */
QPair<QVector<QString>, double> AStarSearch::searchSingle(
    const QString& start, const QString& goal, bool forward) const
{
    const auto& adj = forward ? m_adjForward : m_adjBackward;

    /* 开放列表: {fScore, 节点名} */
    QVector<QPair<double, QString>> openList;
    QHash<QString, double> gScore;   ///< 实际代价
    QHash<QString, double> fScore;   ///< 估计总代价
    QHash<QString, QString> cameFrom;///< 前驱节点
    QHash<QString, bool> closed;     ///< 已关闭节点

    gScore[start] = 0.0;
    double h0 = m_heuristic ? m_heuristic(start, goal) : 0.0;
    fScore[start] = h0;
    openList.append({h0, start});

    while (!openList.isEmpty()) {
        /* 取f值最小的节点(简单排序选择) */
        int bestIdx = 0;
        for (int i = 1; i < openList.size(); ++i) {
            if (openList[i].first < openList[bestIdx].first) {
                bestIdx = i;
            }
        }
        QString current = openList[bestIdx].second;
        openList.removeAt(bestIdx);

        if (current == goal) {
            return {reconstructPath(cameFrom, current), gScore[current]};
        }

        if (closed[current]) continue;
        closed[current] = true;

        /* 扩展邻居 */
        auto neighborsIt = adj.find(current);
        if (neighborsIt == adj.end()) continue;

        for (auto nit = neighborsIt->constBegin();
             nit != neighborsIt->constEnd(); ++nit) {
            const QString& neighbor = nit.key();
            double edgeWeight = nit.value();

            if (closed[neighbor]) continue;

            double tentativeG = gScore[current] + edgeWeight;
            double prevG = gScore.value(neighbor, 1e18);

            if (tentativeG < prevG) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentativeG;
                double h = m_heuristic
                    ? m_heuristic(neighbor, goal) : 0.0;
                double f = tentativeG + h;
                fScore[neighbor] = f;
                openList.append({f, neighbor});
            }
        }
    }

    /* 不可达 */
    return {{}, 1e18};
}

/** @brief 执行A*搜索
 *  @param start 起点
 *  @param goal 终点
 *  @return 最短路径
 */
QVector<QString> AStarSearch::search(const QString& start,
                                     const QString& goal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QString> path;
    double cost = 0.0;

    if (!m_nodeExists.value(start, false) ||
        !m_nodeExists.value(goal, false)) {
        return path;
    }

    if (start == goal) {
        path = {start};
        cost = 0.0;
    } else if (m_bidirectional) {
        /* 双向A*: 正向和反向同时搜索 */
        const auto& adjF = m_adjForward;
        const auto& adjB = m_adjBackward;

        QHash<QString, double> gF, gB;     ///< 正/反向g值
        QHash<QString, QString> fromF, fromB; ///< 正/向前驱
        QHash<QString, bool> closedF, closedB;
        QVector<QPair<double, QString>> openF, openB;

        gF[start] = 0.0;
        double h0 = m_heuristic ? m_heuristic(start, goal) : 0.0;
        openF.append({h0, start});

        gB[goal] = 0.0;
        double h0r = m_heuristic ? m_heuristic(goal, start) : 0.0;
        openB.append({h0r, goal});

        double bestMeeting = 1e18;
        QString meetFwd, meetBwd;

        while (!openF.isEmpty() || !openB.isEmpty()) {
            /* 正向扩展一步 */
            if (!openF.isEmpty()) {
                int bi = 0;
                for (int i = 1; i < openF.size(); ++i) {
                    if (openF[i].first < openF[bi].first) bi = i;
                }
                QString cur = openF[bi].second;
                openF.removeAt(bi);

                if (!closedF[cur]) {
                    closedF[cur] = true;
                    /* 检查是否与反向搜索相遇 */
                    if (closedB.contains(cur)) {
                        double totalG = gF[cur] + gB[cur];
                        if (totalG < bestMeeting) {
                            bestMeeting = totalG;
                            meetFwd = cur;
                        }
                    }
                    auto nit = adjF.find(cur);
                    if (nit != adjF.end()) {
                        for (auto it = nit->constBegin();
                             it != nit->constEnd(); ++it) {
                            double tg = gF[cur] + it.value();
                            if (tg < gF.value(it.key(), 1e18)) {
                                gF[it.key()] = tg;
                                fromF[it.key()] = cur;
                                double h = m_heuristic
                                    ? m_heuristic(it.key(), goal) : 0.0;
                                openF.append({tg + h, it.key()});
                            }
                        }
                    }
                }
            }

            /* 反向扩展一步 */
            if (!openB.isEmpty()) {
                int bi = 0;
                for (int i = 1; i < openB.size(); ++i) {
                    if (openB[i].first < openB[bi].first) bi = i;
                }
                QString cur = openB[bi].second;
                openB.removeAt(bi);

                if (!closedB[cur]) {
                    closedB[cur] = true;
                    if (closedF.contains(cur)) {
                        double totalG = gF[cur] + gB[cur];
                        if (totalG < bestMeeting) {
                            bestMeeting = totalG;
                            meetFwd = cur;
                        }
                    }
                    auto nit = adjB.find(cur);
                    if (nit != adjB.end()) {
                        for (auto it = nit->constBegin();
                             it != nit->constEnd(); ++it) {
                            double tg = gB[cur] + it.value();
                            if (tg < gB.value(it.key(), 1e18)) {
                                gB[it.key()] = tg;
                                fromB[it.key()] = cur;
                                double h = m_heuristic
                                    ? m_heuristic(it.key(), start) : 0.0;
                                openB.append({tg + h, it.key()});
                            }
                        }
                    }
                }
            }

            /* 终止条件: 两端open列表最小f值之和 >= bestMeeting */
            if (bestMeeting < 1e18) {
                bool canStop = true;
                if (!openF.isEmpty()) {
                    double minF = openF[0].first;
                    for (const auto& p : openF) {
                        if (p.first < minF) minF = p.first;
                    }
                    if (!openB.isEmpty()) {
                        double minB = openB[0].first;
                        for (const auto& p : openB) {
                            if (p.first < minB) minB = p.first;
                        }
                        if (minF + minB < bestMeeting) canStop = false;
                    }
                }
                if (canStop) break;
            }
        }

        if (bestMeeting < 1e18 && !meetFwd.isEmpty()) {
            /* 合并正反向路径 */
            QVector<QString> fwdPath = reconstructPath(fromF, meetFwd);
            QVector<QString> bwdPath = reconstructPath(fromB, meetFwd);
            /* 反向路径逆序(不含相遇点) */
            for (int i = 1; i < bwdPath.size(); ++i) {
                fwdPath.append(bwdPath[bwdPath.size() - 1 - i]);
            }
            path = fwdPath;
            cost = bestMeeting;
        }
    } else {
        auto result = searchSingle(start, goal, true);
        path = result.first;
        cost = result.second;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSearches;
    m_pathLengthSum += static_cast<double>(path.size());
    m_stats.avgPathLength = m_pathLengthSum
        / static_cast<double>(m_stats.totalSearches);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    emit searchCompleted(path, cost);
    return path;
}

/** @brief 重置统计信息 */
void AStarSearch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodesVisitedSum = 0.0;
    m_pathLengthSum = 0.0;
}
