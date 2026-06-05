/**
 * @file AATree.h
 * @brief AA树(Arne Andersson Tree) — 平衡二叉搜索树
 *
 * 功能: 实现AA树数据结构，一种简化的红黑树变体。
 *       支持插入/删除/查找/中序遍历操作。
 *       统计插入/删除/查找次数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class AATree
 * @brief AA树 — 基于level的平衡二叉搜索树
 *
 * AA树使用"level"概念替代红黑树的颜色标记，
 * 仅需skew和split两个旋转操作即可维持平衡。
 * 所有操作平均O(log n)时间复杂度。
 */
class AATree : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行统计信息 */
    struct Stats {
        quint64 totalInserts = 0;    ///< 总插入次数
        quint64 totalRemoves = 0;    ///< 总删除次数
        quint64 totalSearches = 0;   ///< 总查找次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit AATree(QObject* parent = nullptr);

    /** @brief 析构函数 — 释放所有节点 */
    ~AATree();

    /**
     * @brief 插入键值
     * @param key 待插入的键值
     */
    void insert(double key);

    /**
     * @brief 删除键值
     * @param key 待删除的键值
     */
    void remove(double key);

    /**
     * @brief 查找键值是否存在
     * @param key 待查找的键值
     * @return 是否存在
     */
    bool contains(double key) const;

    /**
     * @brief 中序遍历获取所有键值(升序)
     * @return 升序排列的键值列表
     */
    QVector<double> inOrder() const;

    /** @brief 获取节点数量 */
    int size() const { return m_size; }

    /** @brief 判断树是否为空 */
    bool isEmpty() const { return m_root == nullptr; }

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 再平衡信号 @param level 触发再平衡时的节点level */
    void rebalanced(int level);

private:
    /** @brief AA树节点结构 */
    struct Node {
        double key;       ///< 键值
        int    level;     ///< 节点level(替代红黑树颜色)
        Node*  left;      ///< 左子树
        Node*  right;     ///< 右子树

        explicit Node(double k)
            : key(k), level(1), left(nullptr), right(nullptr) {}
    };

    /** @brief 右旋(skew) — 消除左向水平链接 */
    Node* skew(Node* node);

    /** @brief 左旋(split) — 消除连续右向水平链接 */
    Node* split(Node* node);

    /** @brief 递归插入 */
    Node* insertNode(Node* node, double key);

    /** @brief 递归删除 */
    Node* removeNode(Node* node, double key);

    /** @brief 查找子树最小节点 */
    Node* findMin(Node* node) const;

    /** @brief 递归降低level并修复不平衡 */
    Node* decreaseLevel(Node* node);

    /** @brief 递归中序遍历 */
    void inOrderHelper(Node* node, QVector<double>& result) const;

    /** @brief 递归释放节点 */
    void destroyTree(Node* node);

    Node*  m_root = nullptr;  ///< 根节点
    int    m_size = 0;        ///< 节点数量
    mutable Stats m_stats;    ///< 统计信息
    mutable double m_timeSum = 0.0; ///< 累计耗时
    int m_totalOps = 0;       ///< 总操作次数(用于平均耗时)
};
