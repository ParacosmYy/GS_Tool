/**
 * @file BPlusTree4.h
 * @brief B+树增强 — 范围查询/批量加载/延迟分裂/节点压缩
 *
 * 增强版B+树实现，支持:
 *   - 范围查询: 高效的[lower, upper]区间扫描
 *   - 批量加载: 从有序数据构建最优B+树
 *   - 延迟分裂: 节点满时不立即分裂，延迟到必要时再操作
 *   - 节点压缩: 前缀压缩减少内部节点存储开销
 * 统计插入/查询/分裂/合并次数。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <QPair>

/**
 * @brief B+树增强
 */
class BPlusTree4 : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalInsertions = 0;    ///< 总插入次数
        quint64 totalQueries = 0;       ///< 总查询次数
        quint64 totalSplits = 0;        ///< 总节点分裂次数
        quint64 totalMerges = 0;        ///< 总节点合并次数
        quint64 totalRangeQueries = 0;  ///< 总范围查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param order B+树阶数(每个节点最多order个子节点)
     * @param enablePrefixCompression 是否启用前缀压缩
     * @param parent 父对象
     */
    explicit BPlusTree4(int order = 64, bool enablePrefixCompression = true,
                        QObject* parent = nullptr);

    ~BPlusTree4();

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     */
    void insert(int key, int value);

    /**
     * @brief 查询键
     * @param key 键
     * @return 值列表(B+树允许重复键)
     */
    QVector<int> query(int key) const;

    /**
     * @brief 范围查询 [lower, upper]
     * @param lower 下界(含)
     * @param upper 上界(含)
     * @return 键值对列表
     */
    QVector<QPair<int, int>> rangeQuery(int lower, int upper) const;

    /**
     * @brief 批量加载有序数据构建最优B+树
     * @param keys 有序键列表
     * @param values 对应值列表
     */
    void bulkLoad(const QVector<int>& keys, const QVector<int>& values);

    /** @brief 删除键 */
    bool remove(int key);

    /** @brief 获取树高度 */
    int height() const;

    /** @brief 获取节点总数 */
    int nodeCount() const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 清空树 */
    void clear();

signals:
    /** 插入完成 */
    void inserted(int key, bool causedSplit);
    /** 范围查询完成 */
    void rangeQueryComplete(int resultCount, double processingTimeMs);
    /** 批量加载完成 */
    void bulkLoadComplete(int count, int height);

private:
    /** B+树节点 */
    struct Node {
        bool isLeaf;
        QVector<int> keys;
        QVector<int> values;     ///< 叶节点: 数据; 内部节点无
        QVector<Node*> children; ///< 内部节点: 子节点; 叶节点无
        Node* next;              ///< 叶节点链表指针
        Node* parent;
        bool delayedSplit;       ///< 延迟分裂标记

        explicit Node(bool leaf) : isLeaf(leaf), next(nullptr), parent(nullptr),
                                   delayedSplit(false) {}
    };

    /** 查找叶节点 */
    Node* findLeaf(int key) const;
    /** 分裂叶节点 */
    Node* splitLeaf(Node* leaf);
    /** 分裂内部节点 */
    Node* splitInternal(Node* node);
    /** 插入到父节点 */
    void insertIntoParent(Node* left, int key, Node* right);
    /** 合并或重分配 */
    void mergeOrRedistribute(Node* node);
    /** 计算高度 */
    int computeHeight(Node* node) const;
    /** 统计节点数 */
    int countNodes(Node* node) const;
    /** 递归删除 */
    void deleteTree(Node* node);
    /** 前缀压缩编码 */
    void compressKeys(Node* node);

    int m_order;
    bool m_compress;
    Node* m_root;
    Stats m_stats;
    double m_timeSum = 0.0;
    QElapsedTimer m_timing;
};
