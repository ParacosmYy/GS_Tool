/**
 * @file BStarTree.h
 * @brief B*树 — 高磁盘利用率平衡多路搜索树
 *
 * 功能: 实现B*树数据结构，节点分裂阈值2/3以保持至少2/3
 *       填充率，支持插入、搜索、删除和范围查询。
 *
 * 协作: DataCache(数据缓存) / SettingsManager(持久化)
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief B*树
 */
class BStarTree : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;        ///< 累计插入次数
        quint64 totalSearches = 0;       ///< 累计搜索次数
        quint64 totalSplits = 0;         ///< 累计分裂次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param order 阶数(默认3，最小3)
     * @param parent 父对象
     */
    explicit BStarTree(int order = 3, QObject* parent = nullptr);
    ~BStarTree() override;

    /**
     * @brief 插入键值对
     * @param key 整数键
     * @param value 关联数据
     */
    void insert(int key, const QByteArray& value);

    /**
     * @brief 搜索键
     * @param key 整数键
     * @return 关联数据(不存在返回空QByteArray)
     */
    QByteArray search(int key) const;

    /**
     * @brief 删除键
     * @param key 待删除的键
     */
    void remove(int key);

    /**
     * @brief 范围查询
     * @param lo 下界(含)
     * @param hi 上界(含)
     * @return 范围内的键列表(有序)
     */
    QVector<int> rangeQuery(int lo, int hi) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 节点分裂发生 */
    void splitOccurred();

private:
    /**
     * @brief B*树节点
     */
    struct Node {
        QVector<int>       keys;     ///< 键列表(有序)
        QVector<QByteArray> values;  ///< 值列表
        QVector<Node*>     children; ///< 子节点指针
        Node*              parent = nullptr; ///< 父节点
        bool               isLeaf = true;   ///< 是否为叶节点
    };

    /** @brief 在节点中查找插入位置 */
    int findPosition(Node* node, int key) const;

    /** @brief 插入到叶节点 */
    void insertToLeaf(Node* node, int key, const QByteArray& value);

    /** @brief 分裂节点 */
    void splitNode(Node* node);

    /** @brief 向兄弟借键 */
    bool borrowFromSibling(Node* node);

    /** @brief 合并节点 */
    void mergeNodes(Node* left, Node* right, int parentKeyIdx);

    /** @brief 删除递归 */
    void removeHelper(Node* node, int key);

    /** @brief 找到键的前驱 */
    int findPredecessor(Node* node, int idx) const;

    /** @brief 范围查询递归 */
    void rangeQueryHelper(Node* node, int lo, int hi,
                          QVector<int>& result) const;

    /** @brief 递归删除子树 */
    void deleteTree(Node* node);

    Node* m_root;    ///< 根节点
    int   m_order;   ///< 阶数

    mutable Stats        m_stats;
    mutable double       m_totalTimeMs = 0.0;
    mutable QElapsedTimer m_timer;
};
