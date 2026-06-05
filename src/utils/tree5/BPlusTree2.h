/**
 * @file BPlusTree2.h
 * @brief B+树 — 范围查询与有序键值存储
 *
 * 功能: 实现B+树数据结构，支持插入、删除、精确查找和范围查询。
 *       所有值存储在叶节点，内部节点仅存键和子指针，
 *       叶节点通过链表连接以支持高效范围扫描。
 *       适用于时间序列索引、日志检索、数据窗口查询。
 *
 * 协作: DataSegmentAnalyzer(分段索引) / EventTimeline(事件索引)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QVariant>
#include <QElapsedTimer>

/**
 * @brief B+树节点
 */
struct BPlusNode {
    bool isLeaf;                              ///< 是否叶节点
    QVector<double> keys;                     ///< 键列表
    QVector<BPlusNode*> children;             ///< 子节点指针(内部节点)
    QVector<QVariant> values;                 ///< 值列表(叶节点)
    BPlusNode* next;                          ///< 下一叶节点(叶节点链表)
    BPlusNode* parent;                        ///< 父节点指针

    explicit BPlusNode(bool leaf)
        : isLeaf(leaf), next(nullptr), parent(nullptr) {}
};

/**
 * @brief B+树 — 范围查询与有序键值存储
 */
class BPlusTree2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;           ///< 累计插入次数
        quint64 totalRemoves = 0;           ///< 累计删除次数
        quint64 totalSearches = 0;          ///< 累计搜索次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param order B+树阶数(每个节点最多order个子节点, 默认4)
     * @param parent 父对象
     */
    explicit BPlusTree2(int order = 4, QObject* parent = nullptr);

    /** @brief 析构函数 — 递归释放所有节点 */
    ~BPlusTree2();

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     */
    void insert(double key, const QVariant& value);

    /**
     * @brief 删除指定键
     * @param key 键
     * @return 是否成功删除
     */
    bool remove(double key);

    /**
     * @brief 精确查找
     * @param key 键
     * @return 对应值；未找到返回无效QVariant
     */
    QVariant find(double key) const;

    /**
     * @brief 范围查询 [lo, hi]
     * @param lo 下界(包含)
     * @param hi 上界(包含)
     * @return 范围内的键值对列表(按键升序)
     */
    QVector<QPair<double, QVariant>> rangeQuery(double lo, double hi) const;

    /** @brief 获取树的总元素数 */
    int count() const { return m_count; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 插入完成信号 @param key 插入的键 */
    void elementInserted(double key);

    /** @brief 删除完成信号 @param key 删除的键 @param success 是否成功 */
    void elementRemoved(double key, bool success);

private:
    /**
     * @brief 查找键所属的叶节点
     * @param key 目标键
     * @return 叶节点指针
     */
    BPlusNode* findLeaf(double key) const;

    /**
     * @brief 分裂叶节点
     * @param node 待分裂节点
     */
    void splitLeaf(BPlusNode* node);

    /**
     * @brief 分裂内部节点
     * @param node 待分裂节点
     */
    void splitInternal(BPlusNode* node);

    /**
     * @brief 在父节点中插入分隔键
     * @param parent 父节点
     * @param key 分隔键
     * @param right 右子节点
     */
    void insertIntoParent(BPlusNode* parent, double key, BPlusNode* right);

    /**
     * @brief 递归删除节点树
     * @param node 根节点
     */
    void deleteTree(BPlusNode* node);

    /**
     * @brief 更新计时统计
     */
    void updateStats() const;

    int       m_order;         ///< B+树阶数
    int       m_count;         ///< 元素总数
    BPlusNode* m_root;         ///< 根节点

    mutable QElapsedTimer m_timer;   ///< 计时器
    mutable double  m_timeSum;       ///< 累计耗时
    mutable quint64 m_totalOps;      ///< 总操作数
    mutable Stats   m_stats;         ///< 统计信息
};
