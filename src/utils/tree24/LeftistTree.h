/**
 * @file LeftistTree.h
 * @brief 左偏堆 — 合并操作/Skew合并变体/NPL增强/可合并优先队列
 *
 * 功能: 实现左偏堆(Leftist Heap)优先队列，支持高效合并操作，
 *       包含skew-merge变体，NPL(零路径长度)增强，以及
 *       标准优先队列操作(插入/删除/查找)。
 *
 * 协作: DataTrigger(触发器) / EventTimeline(事件排序)
 */
#ifndef LEFTISTTREE_H
#define LEFTISTTREE_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 左偏堆(可合并优先队列)
 */
class LeftistTree : public QObject {
    Q_OBJECT

public:
    /** @brief 合并策略 */
    enum class MergeStrategy {
        Standard,       ///< 标准左偏合并(按NPL交换子树)
        Skew            ///< Skew堆变体(总是交换子树)
    };
    Q_ENUM(MergeStrategy)

    /** @brief 统计 */
    struct Stats {
        quint64 totalMerges = 0;            ///< 累计合并次数
        quint64 totalInsertions = 0;        ///< 累计插入次数
        quint64 totalDeletions = 0;         ///< 累计删除次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        int     peakSize = 0;               ///< 峰值堆大小
    };

    explicit LeftistTree(QObject* parent = nullptr);

    ~LeftistTree();

    /** @brief 设置合并策略 @param strategy 策略 */
    void setMergeStrategy(MergeStrategy strategy);

    /** @brief 插入元素 @param priority 优先级 @param data 关联数据 */
    void insert(double priority, double data = 0.0);

    /** @brief 删除并返回最小优先级元素 @return (优先级, 数据), 堆空返回(-1, 0) */
    QPair<double, double> deleteMin();

    /** @brief 查看最小优先级元素 @return (优先级, 数据) */
    QPair<double, double> findMin() const;

    /** @brief 合并另一个左偏堆到当前堆 @param other 另一个堆(合并后变空) */
    void merge(LeftistTree& other);

    /** @brief 批量插入 @param items (优先级, 数据)列表 */
    void insertBatch(const QList<QPair<double, double>>& items);

    /** @brief 堆是否为空 @return 是否为空 */
    bool isEmpty() const;

    /** @brief 堆大小 @return 元素数量 */
    int size() const;

    /** @brief 获取所有元素(按优先级排序) @return 排序后列表 */
    QList<QPair<double, double>> toSortedList() const;

    /** @brief 清空堆 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 合并完成 @param resultingSize 合并后大小 */
    void mergeComplete(int resultingSize);

    /** @brief 元素删除 @param priority 被删除的优先级 */
    void elementDeleted(double priority);

private:
    struct Node {
        double priority;        ///< 优先级
        double data;            ///< 关联数据
        int npl;                ///< 零路径长度(NPL)
        Node* left;             ///< 左子树
        Node* right;            ///< 右子树

        Node(double p, double d)
            : priority(p), data(d), npl(1), left(nullptr), right(nullptr) {}
    };

    Node* mergeNodes(Node* a, Node* b);
    Node* skewMerge(Node* a, Node* b);
    int npl(Node* node) const;
    void updateNpl(Node* node);
    void collectInorder(Node* node, QList<QPair<double, double>>& list) const;
    void destroyTree(Node* node);
    int countNodes(Node* node) const;

    Node* m_root;                   ///< 堆根节点
    int m_size;                     ///< 元素数量
    MergeStrategy m_strategy;       ///< 合并策略

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

#endif // LEFTISTTREE_H
