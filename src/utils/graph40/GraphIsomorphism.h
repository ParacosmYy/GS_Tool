/**
 * @file GraphIsomorphism.h
 * @brief 图同构检测 — VF2算法实现
 *
 * 功能: 实现VF2图同构算法，支持有向/无向图同构与子图同构检测，
 *       度序列预过滤加速候选匹配，回溯法递归搜索，
 *       统计匹配次数/回溯次数/平均处理耗时。
 */
#ifndef GRAPHISOMORPHISM_H
#define GRAPHISOMORPHISM_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @class GraphIsomorphism
 * @brief 图同构检测器，基于VF2算法
 */
class GraphIsomorphism : public QObject {
    Q_OBJECT
public:
    /** 图表示: 邻接矩阵 */
    using AdjMatrix = QVector<QVector<int>>;

    /** 匹配结果 */
    struct MatchResult {
        bool isIsomorphic;              ///< 是否同构
        QVector<int> mapping;           ///< G1→G2的节点映射
        int backtrackCount;             ///< 回溯次数
        double elapsedTimeMs;           ///< 耗时(ms)
    };

    /** 图属性 */
    struct GraphInfo {
        int nodeCount;                      ///< 节点数
        QVector<int> degreeSequence;        ///< 度序列(降序)
        int edgeCount;                      ///< 边数
        bool isDirected;                    ///< 是否有向图
    };

    /** 统计信息 */
    struct Stats {
        quint64 totalMatches = 0;               ///< 总匹配次数
        quint64 totalBacktracks = 0;            ///< 累计回溯次数
        quint64 totalFeasibilityChecks = 0;     ///< 累计可行性检查次数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理耗时(ms)
    };

    explicit GraphIsomorphism(QObject* parent = nullptr);

    /** 完全同构检测 */
    MatchResult checkIsomorphism(const AdjMatrix& g1, const AdjMatrix& g2);

    /** 子图同构检测(在g2中寻找g1的子图) */
    MatchResult checkSubgraphIsomorphism(const AdjMatrix& g1, const AdjMatrix& g2);

    /** 提取图属性(度序列/边数) */
    GraphInfo extractGraphInfo(const AdjMatrix& g) const;

    /** 度序列快速预过滤(不同构直接排除) */
    bool quickFilter(const AdjMatrix& g1, const AdjMatrix& g2) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** 匹配完成信号 */
    void matchComplete(bool isomorphic, int backtracks);
    /** 回溯信号(调试用) */
    void backtrackOccurred(int depth);

private:
    /** VF2递归匹配核心 */
    bool vf2Recursive(const AdjMatrix& g1, const AdjMatrix& g2,
                      QVector<int>& map1to2, QVector<int>& map2to1,
                      QVector<bool>& inM1, QVector<bool>& inM2,
                      int& backtracks);
    /** 生成候选匹配对 */
    QList<QPair<int,int>> generateCandidates(const AdjMatrix& g1,
                                              const AdjMatrix& g2,
                                              const QVector<bool>& inM1,
                                              const QVector<bool>& inM2) const;
    /** 可行性检查 */
    bool feasibilityCheck(int n1, int n2, const AdjMatrix& g1,
                          const AdjMatrix& g2,
                          const QVector<int>& map1to2,
                          const QVector<int>& map2to1,
                          const QVector<bool>& inM1,
                          const QVector<bool>& inM2) const;
    /** 计算度序列 */
    QVector<int> computeDegreeSequence(const AdjMatrix& g) const;

    bool m_enableDegreeFilter;
    Stats m_stats;
    double m_timeSum;
};

#endif // GRAPHISOMORPHISM_H
