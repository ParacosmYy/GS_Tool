/**
 * @file BTreeMap.h
 * @brief B树映射容器 — 批量加载/范围查询/有序遍历
 *
 * 实现B树(B-tree)数据结构的键值映射容器:
 *   - 支持插入、删除、查找操作
 *   - 批量加载(Bulk Loading)用于预排序数据
 *   - 范围查询返回指定区间内所有键值对
 *   - 有序遍历(中序遍历)
 *
 * 协作: DataIndex(数据索引) / DataCache(缓存结构)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QString>

/**
 * @class BTreeMap
 * @brief B树映射容器模板实例化(double键/QString值)
 *
 * B树的每个节点包含多个键和子节点指针，保持平衡。
 * 查找/插入/删除时间复杂度均为O(log n)。
 */
class BTreeMap : public QObject
{
    Q_OBJECT

public:
    /** @brief 键值对类型 */
    using KeyValuePair = QPair<double, QString>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInsertions = 0;       ///< 累计插入次数
        quint64 totalDeletions = 0;        ///< 累计删除次数
        quint64 totalLookups = 0;          ///< 累计查找次数
        quint64 totalRangeQueries = 0;     ///< 累计范围查询次数
        double  avgProcessingTimeMs = 0.0; ///< 平均操作耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param order B树阶数(最小3, 默认5)
     * @param parent 父对象
     */
    explicit BTreeMap(int order = 5, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~BTreeMap();

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     * @return 是否插入成功(键已存在返回false)
     */
    bool insert(double key, const QString& value);

    /**
     * @brief 删除键
     * @param key 要删除的键
     * @return 是否删除成功
     */
    bool remove(double key);

    /**
     * @brief 查找键对应的值
     * @param key 键
     * @param value 输出值
     * @return 是否找到
     */
    bool find(double key, QString& value) const;

    /**
     * @brief 批量加载预排序数据
     * @param sortedPairs 已排序的键值对列表
     */
    void bulkLoad(const QVector<KeyValuePair>& sortedPairs);

    /**
     * @brief 范围查询
     * @param low 下界(包含)
     * @param high 上界(包含)
     * @return 范围内的键值对列表
     */
    QList<KeyValuePair> rangeQuery(double low, double high) const;

    /**
     * @brief 有序遍历所有键值对
     * @return 按键升序排列的键值对
     */
    QList<KeyValuePair> inOrderTraversal() const;

    /**
     * @brief 获取树中元素数量
     */
    int size() const;

    /**
     * @brief 树是否为空
     */
    bool isEmpty() const;

    /**
     * @brief 获取树的高度
     */
    int height() const;

    /** @brief 清空树 */
    void clear();

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入完成 @param key 键 @param size 当前大小 */
    void inserted(double key, int size);

    /** @brief 删除完成 @param key 键 */
    void removed(double key);

    /** @brief 树重新平衡 @param height 新高度 */
    void rebalanced(int height);

private:
    /** @brief B树节点结构 */
    struct BTreeNode {
        QVector<double> keys;              ///< 键数组
        QVector<QString> values;           ///< 值数组
        QVector<BTreeNode*> children;      ///< 子节点指针数组
        bool isLeaf = true;                ///< 是否叶节点
        int keyCount = 0;                  ///< 当前键数量

        /** @brief 节点构造 @param leaf 是否叶节点 */
        explicit BTreeNode(bool leaf) : isLeaf(leaf) {}
    };

    /** @brief 分裂子节点 */
    void splitChild(BTreeNode* parent, int index, BTreeNode* child);

    /** @brief 插入到非满节点 */
    void insertNonFull(BTreeNode* node, double key, const QString& value);

    /** @brief 递归查找 */
    bool findHelper(BTreeNode* node, double key, QString& value) const;

    /** @brief 递归范围查询 */
    void rangeQueryHelper(BTreeNode* node, double low, double high,
                          QList<KeyValuePair>& result) const;

    /** @brief 递归中序遍历 */
    void inOrderHelper(BTreeNode* node,
                       QList<KeyValuePair>& result) const;

    /** @brief 递归删除 */
    bool removeHelper(BTreeNode* node, double key);

    /** @brief 找到键在节点中的位置 */
    int findKeyIndex(BTreeNode* node, double key) const;

    /** @brief 递归计算高度 */
    int heightHelper(BTreeNode* node) const;

    /** @brief 递归销毁 */
    void destroyNode(BTreeNode* node);

    /** @brief 递归构建批量加载 */
    BTreeNode* buildBulkLoad(const QVector<KeyValuePair>& pairs,
                             int start, int end, bool isLeaf);

    int m_order;                ///< B树阶数
    int m_size = 0;             ///< 元素计数
    BTreeNode* m_root = nullptr;///< 根节点

    mutable Stats m_stats;         ///< 操作统计
    mutable double m_timeSum = 0.0;///< 累计耗时
};
