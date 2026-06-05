/**
 * @file ImplicitTreap.h
 * @brief 隐式Treap — 支持区间操作的可分裂/合并数组
 *
 * 功能: 基于隐式Treap(笛卡尔树)实现动态数组，支持O(log n)的
 *       按位置插入、删除、访问和区间反转操作。
 *
 * 协作: DataTransformer(数据变换) / SlidingWindowStats(滑动窗口)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <QRandomGenerator>

/**
 * @brief 隐式Treap(按位置索引的平衡树)
 */
class ImplicitTreap : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;      ///< 累计操作次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit ImplicitTreap(QObject* parent = nullptr);
    ~ImplicitTreap() override;

    /**
     * @brief 尾部追加元素
     * @param value 元素值
     */
    void pushBack(double value);

    /**
     * @brief 在指定位置插入元素
     * @param pos 位置索引(0-based)
     * @param value 元素值
     */
    void insert(int pos, double value);

    /**
     * @brief 删除指定位置元素
     * @param pos 位置索引(0-based)
     */
    void remove(int pos);

    /**
     * @brief 访问指定位置元素
     * @param pos 位置索引(0-based)
     * @return 元素值
     */
    double at(int pos) const;

    /**
     * @brief 反转区间[l, r]
     * @param l 左端点(含)
     * @param r 右端点(含)
     */
    void reverse(int l, int r);

    /** @brief 当前元素数量 */
    int size() const { return m_size; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 结构变更信号 */
    void structureChanged();

private:
    /**
     * @brief Treap节点
     */
    struct Node {
        double    value;           ///< 存储值
        int       priority;        ///< 随机优先级
        int       size = 1;        ///< 子树大小
        bool      rev = false;     ///< 反转懒标记
        Node*     left = nullptr;  ///< 左子节点
        Node*     right = nullptr; ///< 右子节点

        explicit Node(double v) : value(v), priority(QRandomGenerator::global()->generate()) {}
    };

    /** @brief 更新子树大小 */
    static void updateSize(Node* node);

    /** @brief 下推懒标记 */
    static void pushDown(Node* node);

    /** @brief 按位置分裂: 前k个在left, 其余在right */
    void split(Node* node, int k, Node*& left, Node*& right) const;

    /** @brief 合并两棵子树(左树所有位置<右树) */
    Node* merge(Node* left, Node* right) const;

    /** @brief 递归删除子树 */
    void deleteTree(Node* node);

    /** @brief 按位置查找节点 */
    Node* findByPos(Node* node, int pos) const;

    Node* m_root;  ///< 根节点
    int   m_size;  ///< 元素总数

    mutable Stats        m_stats;
    mutable double       m_totalTimeMs = 0.0;
    mutable QElapsedTimer m_timer;
};
