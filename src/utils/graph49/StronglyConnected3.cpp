/**
 * @file StronglyConnected3.cpp
 * @brief 强连通分量增强实现 — Tarjan/Kosaraju/2-SAT/缩图
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现：
 * - Tarjan SCC 算法（单次 DFS）
 * - Kosaraju SCC 算法（两次 DFS）
 * - 2-SAT 求解器（基于蕴含图 + SCC）
 * - 缩图（DAG）构建
 */

#include "utils/graph49/StronglyConnected3.h"

#include <QElapsedTimer>
#include <algorithm>
#include <stack>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
StronglyConnected3::StronglyConnected3(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("StronglyConnected3"));
}

/**
 * @brief 重置统计信息
 */
void StronglyConnected3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设置图的邻接表
 *
 * @param n 顶点数（0 ~ n-1 编号）
 * @param edges 有向边列表 (from, to)
 */
void StronglyConnected3::setGraph(int n, const QVector<QPair<int, int>> &edges)
{
    m_n = n;
    m_adj.assign(n, QVector<int>());
    m_radj.assign(n, QVector<int>());

    for (const auto &e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n) {
            m_adj[e.first].append(e.second);
            m_radj[e.second].append(e.first);
        }
    }
}

/**
 * @brief Tarjan 算法求解强连通分量
 *
 * 经典 Tarjan 算法，使用 DFS 序号和 low 值识别 SCC。
 * 时间复杂度 O(V + E)。
 *
 * @param adj 邻接表
 * @param n 顶点数
 * @return 每个顶点所属的分量编号（同一分量编号相同）
 */
static QVector<int> tarjanImpl(const QVector<QVector<int>> &adj, int n)
{
    QVector<int> index(n, -1);
    QVector<int> low(n, 0);
    QVector<bool> onStack(n, false);
    QVector<int> component(n, -1);
    std::stack<int> stk;
    int curIndex = 0;
    int compId = 0;

    // 递归 DFS 使用显式栈避免栈溢出
    // 使用迭代形式的 Tarjan
    struct Frame {
        int u;
        int neighborIdx;
        bool processed;
    };
    std::stack<Frame> callStack;

    for (int start = 0; start < n; ++start) {
        if (index[start] != -1) continue;

        callStack.push({start, 0, false});

        while (!callStack.empty()) {
            Frame &frame = callStack.top();
            int u = frame.u;

            if (!frame.processed) {
                // 首次访问该节点
                index[u] = curIndex;
                low[u] = curIndex;
                curIndex++;
                onStack[u] = true;
                stk.push(u);
                frame.processed = true;
            }

            // 遍历邻居
            bool recursed = false;
            while (frame.neighborIdx < adj[u].size()) {
                int v = adj[u][frame.neighborIdx];
                frame.neighborIdx++;

                if (index[v] == -1) {
                    // 未访问过的邻居，递归
                    callStack.push({v, 0, false});
                    recursed = true;
                    break;
                } else if (onStack[v]) {
                    low[u] = qMin(low[u], index[v]);
                }
            }

            if (recursed) continue;

            // 所有邻居已处理，回溯
            if (frame.neighborIdx >= static_cast<int>(adj[u].size())) {
                // 更新父节点的 low 值
                if (callStack.size() > 1) {
                    Frame parentFrame = callStack.top();
                    callStack.pop();
                    Frame &parent = callStack.top();
                    low[parent.u] = qMin(low[parent.u], low[parentFrame.u]);
                    callStack.push(parentFrame); // 临时放回以获取引用
                    callStack.pop(); // 移除
                    // 重新放入 parent（不含子帧）
                    // 实际上我们需要在回溯时更新 low
                }

                // 检查是否为 SCC 根节点
                if (low[u] == index[u]) {
                    int w;
                    do {
                        w = stk.top();
                        stk.pop();
                        onStack[w] = false;
                        component[w] = compId;
                    } while (w != u);
                    compId++;
                }

                callStack.pop();
            }
        }
    }

    return component;
}

/**
 * @brief Tarjan 算法求解强连通分量（包装方法）
 * @return 每个顶点的分量编号
 */
QVector<int> StronglyConnected3::tarjanSCC()
{
    QElapsedTimer timer;
    timer.start();

    m_component = tarjanImpl(m_adj, m_n);

    m_stats.totalComputations++;
    m_stats.totalVerticesProcessed += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    int numComp = numComponents();
    emit computationComplete(numComp);
    return m_component;
}

/**
 * @brief Kosaraju 算法求解强连通分量
 *
 * 两次 DFS：
 * 1. 在原图上 DFS，记录后序（完成时间）
 * 2. 在反图上按逆后序 DFS，每次 DFS 到达的顶点构成一个 SCC
 *
 * @return 每个顶点的分量编号
 */
QVector<int> StronglyConnected3::kosarajuSCC()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> component(m_n, -1);
    QVector<bool> visited(m_n, false);
    QVector<int> order;

    // 第一次 DFS：在原图上计算后序
    for (int i = 0; i < m_n; ++i) {
        if (visited[i]) continue;
        std::stack<int> stk;
        stk.push(i);
        while (!stk.empty()) {
            int u = stk.top();
            if (!visited[u]) {
                visited[u] = true;
                for (int v : m_adj[u]) {
                    if (!visited[v]) stk.push(v);
                }
            } else {
                stk.pop();
                // 加入后序列表（仅在首次 pop 时）
                if (!order.contains(u)) {
                    order.append(u);
                }
            }
        }
    }

    // 第二次 DFS：在反图上按逆后序遍历
    std::reverse(order.begin(), order.end());
    visited.assign(m_n, false);
    int compId = 0;

    for (int start : order) {
        if (visited[start]) continue;
        std::stack<int> stk;
        stk.push(start);
        while (!stk.empty()) {
            int u = stk.top();
            stk.pop();
            if (visited[u]) continue;
            visited[u] = true;
            component[u] = compId;
            for (int v : m_radj[u]) {
                if (!visited[v]) stk.push(v);
            }
        }
        compId++;
    }

    m_component = component;

    m_stats.totalComputations++;
    m_stats.totalVerticesProcessed += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationComplete(compId);
    return m_component;
}

/**
 * @brief 2-SAT 求解器
 *
 * 将每个布尔变量 x 拆为两个节点：x (真) 和 ~x (假)。
 * 每个子句 (a OR b) 转化为两条蕴含边：~a -> b 和 ~b -> a。
 * 在蕴含图上求 SCC，若 x 和 ~x 在同一分量中则不可满足。
 *
 * 变量编号规则：变量 i 的正节点为 2*i，负节点为 2*i+1。
 *
 * @param clauses 子句列表，每个子句为 (literal1, literal2)
 *                literal 编码：正值表示变量取真，负值表示取假
 * @param numVars 变量数量
 * @return true 存在满足的赋值方案，false 不可满足
 */
bool StronglyConnected3::solve2SAT(const QVector<QPair<int, int>> &clauses, int numVars)
{
    QElapsedTimer timer;
    timer.start();

    const int totalNodes = 2 * numVars;
    QVector<QVector<int>> implGraph(totalNodes);
    QVector<QVector<int>> rImplGraph(totalNodes);

    // 构建蕴含图
    for (const auto &clause : clauses) {
        int a = clause.first;
        int b = clause.second;

        // 将 literal 转换为节点编号
        // 正 literal x -> 节点 2*(x-1)，负 literal -x -> 节点 2*(-x-1)+1
        auto toNode = [numVars](int lit) -> int {
            if (lit > 0) return 2 * (lit - 1);
            return 2 * (-lit - 1) + 1;
        };

        auto negate = [](int node) -> int {
            return node ^ 1;
        };

        int na = toNode(a);
        int nb = toNode(b);

        // ~a -> b
        implGraph[negate(na)].append(nb);
        rImplGraph[nb].append(negate(na));

        // ~b -> a
        implGraph[negate(nb)].append(na);
        rImplGraph[na].append(negate(nb));
    }

    // 在蕴含图上设置图并运行 Kosaraju
    m_n = totalNodes;
    m_adj = implGraph;
    m_radj = rImplGraph;

    QVector<int> comp = kosarajuSCC();

    // 检查是否存在 x 和 ~x 在同一分量
    for (int i = 0; i < numVars; ++i) {
        if (comp[2 * i] == comp[2 * i + 1]) {
            return false; // 不可满足
        }
    }

    m_timeSum += timer.elapsed();
    return true;
}

/**
 * @brief 构建缩图（将每个 SCC 收缩为一个超级节点）
 *
 * 缩图是一个 DAG，边方向与原图一致。
 *
 * @return 缩图的边列表 (fromComponent, toComponent)
 */
QVector<QPair<int, int>> StronglyConnected3::condensationGraph() const
{
    if (m_component.isEmpty()) return {};

    const int numComp = numComponents();
    QSet<QPair<int, int>> edgeSet;

    for (int u = 0; u < m_n; ++u) {
        for (int v : m_adj[u]) {
            if (m_component[u] != m_component[v]) {
                edgeSet.insert({m_component[u], m_component[v]});
            }
        }
    }

    return edgeSet.values().toVector();
}

/**
 * @brief 获取强连通分量数量
 * @return 分量数
 */
int StronglyConnected3::numComponents() const
{
    if (m_component.isEmpty()) return 0;
    int maxComp = 0;
    for (int c : m_component) {
        maxComp = qMax(maxComp, c);
    }
    return maxComp + 1;
}
