/**
 * @file BPlusTree3.h
 * @brief B+树(批量加载+前缀压缩) — 优化磁盘IO的B+树变体
 * 批量加载从有序数据构建最优树，前缀压缩减少内部节点键存储
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QByteArray>

/**
 * @brief B+树(批量加载+前缀压缩)引擎
 */
class BPlusTree3 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;               ///< 累计插入次数
        int totalBulkLoads = 0;                ///< 累计批量加载次数
        int totalSearches = 0;                 ///< 累计搜索次数
        int totalNodesSplit = 0;               ///< 累计节点分裂次数
        int totalBytesSaved = 0;               ///< 前缀压缩节省字节数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param order B+树阶数(每个节点最多order个子节点)
     * @param parent 父对象
     */
    explicit BPlusTree3(int order = 4, QObject* parent = nullptr);

    /**
     * @brief 析构函数 — 释放所有节点
     */
    ~BPlusTree3();

    /** @brief 插入键值对 @param key 键 @param value 值 @return 重复键返回false */
    bool insert(int key, const QByteArray& value);
    /** @brief 批量加载(要求输入已按键排序) @return 加载的记录数 */
    int bulkLoad(const QList<QPair<int, QByteArray>>& pairs);
    /** @brief 精确查找 @return (是否找到, 值) */
    QPair<bool, QByteArray> find(int key) const;
    /** @brief 范围查询 @param low 下界(含) @param high 上界(含) */
    QList<QPair<int, QByteArray>> rangeQuery(int low, int high) const;
    /** @brief 删除键 @return 是否成功 */
    bool remove(int key);
    /** @brief 更新已有键的值 @return 键不存在返回false */
    bool update(int key, const QByteArray& newValue);

    /** @brief 获取树高度 */
    int height() const;
    /** @brief 获取总记录数 */
    int size() const { return m_size; }
    /** @brief 获取阶数 */
    int order() const { return m_order; }
    /** @brief 清空整棵树 */
    void clear();
    /** @brief 获取所有叶子节点的键(有序) */
    QList<int> allKeys() const;
    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 批量加载完成 @param recordCount 记录数 @param treeHeight 树高度 */
    void bulkLoadCompleted(int recordCount, int treeHeight);
    /** @brief 节点分裂 @param level 分裂发生的层级 */
    void nodeSplit(int level);

private:
    /** @brief B+树节点 */
    struct Node {
        bool isLeaf = false;                   ///< 是否叶子节点
        QVector<int> keys;                     ///< 键(前缀压缩后)
        QVector<QByteArray> values;            ///< 值(仅叶子)
        QVector<Node*> children;               ///< 子节点(仅内部)
        Node* next = nullptr;                  ///< 叶子链表下一个
        Node* parent = nullptr;                ///< 父节点
        QByteArray prefix;                     ///< 公共前缀
        QVector<int> suffixOffsets;            ///< 后缀偏移量
    };

    /** @brief 创建新节点 */
    Node* createNode(bool isLeaf);
    /** @brief 递归删除节点 */
    void destroyNode(Node* node);
    /** @brief 在叶子节点中插入 */
    void insertIntoLeaf(Node* leaf, int key, const QByteArray& value);
    /** @brief 分裂叶子节点 */
    void splitLeaf(Node* leaf);
    /** @brief 分裂内部节点 */
    void splitInternal(Node* internal);
    /** @brief 将分裂后的中间键插入父节点 */
    void insertIntoParent(Node* left, int key, Node* right);
    /** @brief 对节点内的键做前缀压缩 */
    void compressPrefix(Node* node);
    /** @brief 解压前缀获取完整键 */
    int decompressKey(const Node* node, int index) const;
    /** @brief 查找键应在的叶子节点 */
    Node* findLeaf(int key) const;
    /** @brief 递归计算树高度 */
    int computeHeight(const Node* node) const;

    int m_order;                               ///< B+树阶数
    int m_size = 0;                            ///< 记录总数
    Node* m_root = nullptr;                    ///< 根节点

    Stats m_stats;
    double m_timeSum = 0.0;                     ///< 处理时间累加器
};
