/**
 * @file IntervalTree2.h
 * @brief 增强区间树 — 区间插入/查询/删除 + 重叠检测 + 增广子树最大值
 *
 * 功能: 实现基于增广二叉搜索树的区间树，每个节点存储一个区间[low, high]，
 *       并维护以该节点为根的子树中最大的high值(maxHigh)用于剪枝。
 *       支持精确重叠查询、点查询、 stabbing查询、区间删除。
 *       统计操作次数/重叠检测数/平均处理耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class IntervalTree2
 * @brief 增广区间树，支持高效重叠查询
 */
class IntervalTree2 : public QObject {
    Q_OBJECT
public:
    /** 区间表示 */
    struct Interval {
        double low;   ///< 区间下界
        double high;  ///< 区间上界
        int     id;   ///< 用户标识
    };

    /** 操作统计 */
    struct Stats {
        quint64 totalInsertions = 0;       ///< 总插入次数
        quint64 totalDeletions = 0;        ///< 总删除次数
        quint64 totalQueries = 0;          ///< 总查询次数
        quint64 totalOverlapsFound = 0;    ///< 累计找到的重叠区间数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** 构造函数 */
    explicit IntervalTree2(QObject* parent = nullptr);

    /** 析构函数 */
    ~IntervalTree2() override;

    /** @brief 插入一个区间 @param low 下界 @param high 上界 @param id 标识(默认自增) */
    void insert(double low, double high, int id = -1);

    /** @brief 删除指定id的区间 @param id 区间标识 @return 是否成功删除 */
    bool remove(int id);

    /**
     * @brief 查询与给定区间重叠的所有区间
     * @param low 查询下界
     * @param high 查询上界
     * @return 重叠的区间列表
     */
    QVector<Interval> queryOverlaps(double low, double high) const;

    /**
     * @brief 查询包含指定点的所有区间(stabbing查询)
     * @param point 查询点
     * @return 包含该点的区间列表
     */
    QVector<Interval> queryPoint(double point) const;

    /**
     * @brief 检查两个区间是否重叠
     * @param a 第一个区间
     * @param b 第二个区间
     * @return 是否重叠
     */
    static bool overlaps(const Interval& a, const Interval& b);

    /** @brief 获取树中区间总数 */
    int size() const { return m_size; }

    /** @brief 获取所有区间(中序遍历) */
    QVector<Interval> allIntervals() const;

    /** @brief 清空所有区间 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 区间插入 @param id 区间标识 @param low 下界 @param high 上界 */
    void intervalInserted(int id, double low, double high);
    /** @brief 区间删除 @param id 区间标识 */
    void intervalRemoved(int id);
    /** @brief 重叠查询完成 @param count 找到数量 */
    void queryCompleted(int count);

private:
    /** 树节点 */
    struct Node {
        Interval interval;   ///< 节点区间
        double   maxHigh;    ///< 子树最大上界
        double   minLow;     ///< 子树最小下界
        int      height;     ///< AVL平衡因子
        Node*    left;       ///< 左子树
        Node*    right;      ///< 右子树

        explicit Node(const Interval& iv)
            : interval(iv), maxHigh(iv.high), minLow(iv.low),
              height(1), left(nullptr), right(nullptr) {}
    };

    /** AVL插入 */
    Node* insertNode(Node* node, const Interval& iv);
    /** AVL删除 */
    Node* removeNode(Node* node, int id, bool& removed);
    /** 重叠查询 */
    void queryOverlapsHelper(Node* node, double low, double high,
                             QVector<Interval>& result) const;
    /** 中序遍历 */
    void inorderTraversal(Node* node, QVector<Interval>& result) const;
    /** 释放子树 */
    void deleteSubtree(Node* node);

    /** AVL旋转 */
    Node* rotateRight(Node* y);
    Node* rotateLeft(Node* x);
    /** 更新增广信息 */
    void updateAugment(Node* node);
    /** 获取平衡因子 */
    int  getBalance(Node* node) const;
    /** 获取节点高度 */
    int  getHeight(Node* node) const;

    Node* m_root;           ///< 根节点
    int   m_size;           ///< 区间总数
    int   m_nextId;         ///< 自增ID

    mutable Stats  m_stats;         ///< 统计信息
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
