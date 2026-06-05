/**
 * @file SkipGraph.h
 * @brief Skip Graph跳表图数据结构
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QRandomGenerator>
#include <cmath>

/**
 * @class SkipGraph
 * @brief Skip Graph — 多层链表式分布式数据结构
 *
 * 支持O(log n)查找/插入/删除，适合分布式环境中的有序数据管理。
 * 每个节点拥有多层指针，层数由成员向量决定。
 */
class SkipGraph : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 节点结构
     */
    struct Node {
        double key;                     /**< 键值 */
        QVariant value;                 /**< 关联数据 */
        QVector<Node*> forward;         /**< 各层前向指针 */
        QVector<Node*> backward;        /**< 各层后向指针 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;        /**< 总插入数 */
        int totalRemoves = 0;        /**< 总删除数 */
        int totalSearches = 0;       /**< 总查找数 */
        int totalHits = 0;           /**< 命中次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param maxLevel 最大层数(默认16)
     * @param parent 父对象
     */
    explicit SkipGraph(int maxLevel = 16, QObject* parent = nullptr);

    ~SkipGraph();

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     */
    void insert(double key, const QVariant& value = QVariant());

    /**
     * @brief 删除键
     * @param key 键
     * @return 是否成功删除
     */
    bool remove(double key);

    /**
     * @brief 查找键
     * @param key 键
     * @return 对应值(未找到返回无效QVariant)
     */
    QVariant search(double key) const;

    /**
     * @brief 范围查询
     * @param lo 下界
     * @param hi 上界
     * @return 范围内的键值列表
     */
    QVector<QPair<double, QVariant>> rangeQuery(double lo, double hi) const;

    /** @brief 获取有序键列表 */
    QVector<double> keys() const;

    /** @brief 获取元素数量 */
    int size() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void insertCompleted(double key);
    /** @brief 删除完成信号 */
    void removeCompleted(double key, bool success);

private:
    int randomLevel() const;
    Node* findNode(double key) const;

    int m_maxLevel;             /**< 最大层数 */
    int m_level;                /**< 当前最高层 */
    int m_count;                /**< 元素数量 */
    Node* m_header;             /**< 头哨兵节点 */
    mutable Stats m_stats;      /**< 统计信息 */
    mutable double m_timeSum;   /**< 累计时间 */
};
