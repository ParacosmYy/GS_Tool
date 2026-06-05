/**
 * @file MaxBipartiteMatch.cpp
 * @brief 匈牙利算法最大权二部图匹配实现
 */

#include "utils/graph23/MaxBipartiteMatch.h"

#include <QtMath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
MaxBipartiteMatch::MaxBipartiteMatch(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 求解最大权匹配
 * @param costMatrix n×n代价矩阵
 * @return (匹配对列表, 总权重)
 */
QPair<QVector<QPair<int, int>>, double>
MaxBipartiteMatch::solveMaxWeight(const QVector<QVector<double>>& costMatrix)
{
    m_timer.start();

    int n = costMatrix.size();
    if (n == 0) return {{}, 0.0};

    /* 转换为最小化问题: max -> min by negation */
    double maxVal = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < static_cast<int>(costMatrix[i].size()); ++j) {
            maxVal = qMax(maxVal, costMatrix[i][j]);
        }
    }

    QVector<QVector<double>> mat(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < static_cast<int>(costMatrix[i].size()); ++j) {
            mat[i][j] = maxVal - costMatrix[i][j];
        }
        /* 补齐到n×n */
        for (int j = static_cast<int>(costMatrix[i].size()); j < n; ++j) {
            mat[i][j] = maxVal;
        }
    }

    /* 运行匈牙利算法 */
    QVector<int> matchR = hungarianCore(mat);

    /* 构建匹配结果 */
    QVector<QPair<int, int>> matches;
    double totalWeight = 0.0;

    for (int j = 0; j < n; ++j) {
        if (matchR[j] >= 0 && matchR[j] < n) {
            matches.append({matchR[j], j});
            if (j < static_cast<int>(costMatrix[matchR[j]].size())) {
                totalWeight += costMatrix[matchR[j]][j];
            }
        }
    }

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalSolves;
    m_stats.totalWeight += totalWeight;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(matches.size(), totalWeight);
    return {matches, totalWeight};
}

/**
 * @brief 求解最小权匹配
 * @param costMatrix n×n代价矩阵
 * @return (匹配对列表, 总代价)
 */
QPair<QVector<QPair<int, int>>, double>
MaxBipartiteMatch::solveMinWeight(const QVector<QVector<double>>& costMatrix)
{
    m_timer.start();

    int n = costMatrix.size();
    if (n == 0) return {{}, 0.0};

    QVector<QVector<double>> mat(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < static_cast<int>(costMatrix[i].size()); ++j) {
            mat[i][j] = costMatrix[i][j];
        }
    }

    QVector<int> matchR = hungarianCore(mat);

    QVector<QPair<int, int>> matches;
    double totalCost = 0.0;

    for (int j = 0; j < n; ++j) {
        if (matchR[j] >= 0 && matchR[j] < n) {
            matches.append({matchR[j], j});
            if (j < static_cast<int>(costMatrix[matchR[j]].size())) {
                totalCost += costMatrix[matchR[j]][j];
            }
        }
    }

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(matches.size(), totalCost);
    return {matches, totalCost};
}

/**
 * @brief 验证是否为完美匹配
 */
bool MaxBipartiteMatch::isPerfectMatch(
    const QVector<QVector<double>>& costMatrix,
    const QVector<QPair<int, int>>& matches) const
{
    int n = costMatrix.size();
    if (matches.size() != n) return false;

    QSet<int> leftUsed, rightUsed;
    for (const auto& p : matches) {
        if (leftUsed.contains(p.first) || rightUsed.contains(p.second))
            return false;
        leftUsed.insert(p.first);
        rightUsed.insert(p.second);
    }
    return true;
}

/**
 * @brief 获取对偶变量
 */
QPair<QVector<double>, QVector<double>> MaxBipartiteMatch::labels() const
{
    return {m_uLabels, m_vLabels};
}

/** @brief 重置统计 */
void MaxBipartiteMatch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_uLabels.clear();
    m_vLabels.clear();
}

/**
 * @brief 匈牙利算法核心 — 标号法 O(n^3)
 * 使用potential(标号)数组 + 增广路径搜索
 */
QVector<int> MaxBipartiteMatch::hungarianCore(QVector<QVector<double>>& mat)
{
    int n = mat.size();
    if (n == 0) return {};

    QVector<double> uLabel(n + 1, 0.0);  /* 左侧标号 */
    QVector<double> vLabel(n + 1, 0.0);  /* 右侧标号 */
    QVector<int> matchL(n + 1, 0);       /* 左->右匹配 */
    QVector<int> matchR(n + 1, 0);       /* 右->左匹配 */

    /* 初始化v标号为各列最小值 */
    for (int j = 1; j <= n; ++j) {
        vLabel[j] = std::numeric_limits<double>::max();
        for (int i = 1; i <= n; ++i) {
            if (i - 1 < static_cast<int>(mat.size()) &&
                j - 1 < static_cast<int>(mat[i - 1].size())) {
                vLabel[j] = qMin(vLabel[j], mat[i - 1][j - 1]);
            }
        }
        if (vLabel[j] == std::numeric_limits<double>::max())
            vLabel[j] = 0.0;
    }

    /* 逐个匹配左侧顶点 */
    for (int u = 1; u <= n; ++u) {
        matchL[0] = u; /* 虚拟起点 */
        int freeVertex = 0;

        QVector<double> slack(n + 1,
            std::numeric_limits<double>::max());
        QVector<int> slackFrom(n + 1, 0);
        QVector<bool> visitedR(n + 1, false);

        do {
            /* BFS寻找增广路径 */
            QVector<bool> visitedL(n + 1, false);
            visitedL[freeVertex] = true;
            double delta = std::numeric_limits<double>::max();
            int nextFree = 0;

            for (int j = 1; j <= n; ++j) {
                if (!visitedR[j]) {
                    double cost = (u - 1 < static_cast<int>(mat.size()) &&
                        j - 1 < static_cast<int>(mat[u - 1].size()))
                        ? mat[u - 1][j - 1] : 0.0;

                    double reduced = cost - uLabel[u] - vLabel[j];
                    if (reduced < slack[j]) {
                        slack[j] = reduced;
                        slackFrom[j] = u;
                    }

                    if (slack[j] < delta) {
                        delta = slack[j];
                        nextFree = j;
                    }
                }
            }

            /* 更新标号 */
            for (int i = 0; i <= n; ++i) {
                if (visitedL[i]) uLabel[i] += delta;
                if (visitedR[i]) vLabel[i] -= delta;
                else slack[i] -= delta;
            }

            visitedR[nextFree] = true;
            ++m_stats.totalAugmentingPaths;

            /* 如果找到未匹配点，增广成功 */
            if (matchR[nextFree] == 0) {
                freeVertex = nextFree;
            } else {
                freeVertex = matchR[nextFree];
                visitedL[freeVertex] = true;
            }

        } while (matchR[freeVertex] != 0);

        /* 回溯更新匹配 */
        while (freeVertex != 0) {
            int prev = slackFrom[freeVertex];
            matchR[freeVertex] = prev;
            int tmp = matchL[prev];
            matchL[prev] = freeVertex;
            freeVertex = tmp;
        }
    }

    /* 保存标号 */
    m_uLabels = QVector<double>(uLabel.begin() + 1, uLabel.end());
    m_vLabels = QVector<double>(vLabel.begin() + 1, vLabel.end());

    /* 构建结果: matchR[j] = i 表示右j匹配左i */
    QVector<int> result(n, -1);
    for (int j = 1; j <= n; ++j) {
        result[j - 1] = matchR[j] - 1;
    }

    return result;
}
