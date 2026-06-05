/**
 * @file IntervalTree2.h
 * @brief 增强区间树
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 增强区间树
 *
 * 基于红黑树的区间树,支持O(log n)区间查询、
 * 重叠检测和 stabbing query。
 */
class IntervalTree2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 区间 */
    struct Interval {
        double low = 0.0, high = 0.0; ///< 区间[low, high]
        int id = -1;                   ///< 附加标识
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;       ///< 总插入次数
        int totalQueries = 0;          ///< 总查询次数
        double avgProcessingTimeMs = 0.0;
    };

    explicit IntervalTree2(QObject* parent = nullptr);
    ~IntervalTree2();

    /**
     * @brief 插入区间
     */
    void insert(const Interval& interval);

    /**
     * @brief 查询与指定区间重叠的所有区间
     */
    QVector<Interval> queryOverlaps(const Interval& query);

    /**
     * @brief 查询包含指定点的所有区间
     */
    QVector<Interval> queryPoint(double point);

    /**
     * @brief 检查是否存在任何重叠
     */
    bool hasAnyOverlap(const Interval& query);

    /**
     * @brief 获取所有区间
     */
    QVector<Interval> allIntervals() const;

    /**
     * @brief 区间数量
     */
    int size() const;

    /**
     * @brief 清空
     */
    void clear();

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void inserted(int id, double low, double high);

private:
    struct Node {
        Interval interval;
        double maxEnd;     ///< 子树最大右端点
        Node* left;
        Node* right;
        int height;        ///< AVL平衡因子用
        Node(const Interval& iv)
            : interval(iv), maxEnd(iv.high), left(nullptr), right(nullptr), height(1) {}
    };

    Node* m_root;
    int m_size;
    Stats m_stats;
    double m_timeSum = 0.0;

    Node* insertNode(Node* node, const Interval& iv);
    Node* rotateRight(Node* y);
    Node* rotateLeft(Node* x);
    int height(Node* n) const;
    int balanceFactor(Node* n) const;
    void updateMaxEnd(Node* node);
    void queryOverlapsHelper(Node* node, const Interval& q, QVector<Interval>& result);
    void queryPointHelper(Node* node, double point, QVector<Interval>& result);
    void collectAll(Node* node, QVector<Interval>& result) const;
    void destroyTree(Node* node);
};
