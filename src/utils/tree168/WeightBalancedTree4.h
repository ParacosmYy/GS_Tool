/**
 * @file WeightBalancedTree4.h
 * @brief 重量平衡树(单/双旋转+基于排名的再平衡) — Weight-Balanced Tree with Single/Double Rotations and Rank-Based Rebalancing
 *
 * 功能: 实现重量平衡树(Weight-Balanced Tree / BB[α]树)，通过节点权重(子树大小)
 *       检测不平衡并执行单旋转或双旋转恢复平衡，支持插入/删除/排名查询。
 *
 * 协作: ScapegoatTree4(替罪羊树) / RedBlackTree(红黑树) / AVLTree(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 重量平衡树(BB[alpha]树)
 */
class WeightBalancedTree4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalRotations = 0;        ///< 累计旋转次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
    };

    explicit WeightBalancedTree4(QObject* parent = nullptr);
    ~WeightBalancedTree4() override;

    /** @brief 设置平衡因子alpha(0.5~1.0) */
    void setAlpha(double alpha);

    /** @brief 插入元素 */
    bool insert(double key);

    /** @brief 删除元素 */
    bool remove(double key);

    /** @brief 查找元素是否存在 */
    bool contains(double key) const;

    /** @brief 查找key的排名 */
    int rank(double key) const;

    /** @brief 查找第k小元素 */
    double kth(int k) const;

    /** @brief 中序遍历 */
    QVector<double> inorder() const;

    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void rotationPerformed(int type);

private:
    /** @brief 树节点 */
    struct Node {
        double key;
        Node* left = nullptr;
        Node* right = nullptr;
        int weight = 1; ///< 子树重量(节点数)

        explicit Node(double k) : key(k) {}
    };

    /** @brief 旋转类型 */
    enum RotType { SingleRight, SingleLeft, DoubleRightLeft, DoubleLeftRight };

    /** @brief 获取节点重量 */
    static int wt(Node* node);

    /** @brief 更新重量 */
    static void updateWeight(Node* node);

    /** @brief 检查并再平衡 */
    Node* rebalance(Node* node);

    /** @brief 单右旋转 */
    Node* rotateRight(Node* node);

    /** @brief 单左旋转 */
    Node* rotateLeft(Node* node);

    /** @brief 插入递归 */
    Node* insertRec(Node* node, double key, bool& inserted);

    /** @brief 删除递归 */
    Node* removeRec(Node* node, double key, bool& removed);

    /** @brief 查找递归 */
    bool containsRec(Node* node, double key) const;

    /** @brief 排名递归 */
    int rankRec(Node* node, double key) const;

    /** @brief 第k小递归 */
    double kthRec(Node* node, int k) const;

    /** @brief 中序递归 */
    void inorderRec(Node* node, QVector<double>& result) const;

    /** @brief 递归清除 */
    void clearRec(Node* node);

    /** @brief 计算树高 */
    static int height(Node* node);

    double m_alpha = 0.288; ///< Adams平衡参数
    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;
};
