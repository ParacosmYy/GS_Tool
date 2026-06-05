/**
 * @file VanEmdeBoas.h
 * @brief van Emde Boas树,支持O(log log U)前驱/后继操作
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief van Emde Boas树
 *
 * 以O(log log U)时间复杂度支持动态集合上的插入、删除、
 * 前驱、后继、最小值、最大值和成员查询。
 * 适用于整数键的快速有序集合操作,如网络路由表、
 * 调度器和序列分析中的区间查询。
 */
class VanEmdeBoas : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;        ///< 总插入次数
        int totalDeletions = 0;         ///< 总删除次数
        int totalQueries = 0;           ///< 总查询次数(前驱/后继/成员)
        double avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param universeSize 全域大小(必须为2的幂,如65536)
     * @param parent 父对象
     */
    explicit VanEmdeBoas(int universeSize, QObject* parent = nullptr);

    ~VanEmdeBoas();

    /**
     * @brief 插入一个元素
     * @param x 要插入的值(必须在[0, universeSize)范围内)
     */
    void insert(int x);

    /**
     * @brief 删除一个元素
     * @param x 要删除的值
     */
    void remove(int x);

    /**
     * @brief 查询成员关系
     * @param x 查询值
     * @return 是否存在于集合中
     */
    bool contains(int x) const;

    /**
     * @brief 查询前驱(小于x的最大元素)
     * @param x 查询值
     * @return 前驱值,不存在返回-1
     */
    int predecessor(int x) const;

    /**
     * @brief 查询后继(大于x的最小元素)
     * @param x 查询值
     * @return 后继值,不存在返回-1
     */
    int successor(int x) const;

    /**
     * @brief 获取集合中的最小值
     * @return 最小值,空集返回-1
     */
    int minimum() const;

    /**
     * @brief 获取集合中的最大值
     * @return 最大值,空集返回-1
     */
    int maximum() const;

    /**
     * @brief 获取当前集合大小
     * @return 元素个数
     */
    int size() const;

    /**
     * @brief 判断集合是否为空
     * @return 是否为空
     */
    bool isEmpty() const;

    /**
     * @brief 获取所有元素(按升序排列)
     * @return 有序元素列表
     */
    QVector<int> toSortedVector() const;

    /**
     * @brief 获取全域大小
     * @return 全域大小
     */
    int universeSize() const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void elementInserted(int value);

    /** @brief 删除完成信号 */
    void elementRemoved(int value);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief vEB树内部节点 */
    struct Node {
        int min = -1;           ///< 子树最小值(不存储在簇中)
        int max = -1;           ///< 子树最大值
        int u;                  ///< 该节点代表的全域大小
        Node* summary = nullptr;///< 摘要结构
        Node** cluster = nullptr;///< 簇数组
    };

    Node* m_root;           ///< 树根
    int m_universeSize;     ///< 全域大小
    int m_count;            ///< 元素计数

    int upperSqrt(int u) const;
    int lowerSqrt(int u) const;
    int high(int x, int u) const;
    int low(int x, int u) const;
    int index(int high, int low, int u) const;

    Node* createNode(int u);
    void destroyNode(Node* node);
    void insertRec(Node*& node, int x);
    void removeRec(Node*& node, int x);
    bool containsRec(Node* node, int x) const;
    int predecessorRec(Node* node, int x) const;
    int successorRec(Node* node, int x) const;
    int minimumRec(Node* node) const;
    int maximumRec(Node* node) const;
    void collectSorted(Node* node, int offset, QVector<int>& result) const;
};
