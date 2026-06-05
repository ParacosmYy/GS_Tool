/**
 * @file AVLTree3.h
 * @brief AVL树增强 — 顺序统计/排名查询/区间统计/旋转优化
 *
 * 功能: 实现AVL平衡二叉搜索树的增强版本，支持顺序统计查询(第k小)、
 *       排名查询(元素排名)、区间统计(范围求和)、旋转优化。
 *
 * 协作: DataClassifier(分类) / StateTracker(状态跟踪)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief AVL树增强 — 顺序统计/排名查询/区间统计
 */
class AVLTree3 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalInsertions = 0;        ///< 累计插入次数
        quint64 totalDeletions = 0;         ///< 累计删除次数
        quint64 totalQueries = 0;           ///< 累计查询次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        quint64 totalRotations = 0;         ///< 累计旋转次数
    };

    explicit AVLTree3(QObject* parent = nullptr);
    ~AVLTree3() override;

    /** @brief 插入元素 @param key 键 @param value 值 */
    void insert(double key, double value = 0.0);

    /** @brief 删除元素 @param key 键 @return 是否成功删除 */
    bool remove(double key);

    /** @brief 查找元素 @param key 键 @return 是否存在 */
    bool contains(double key) const;

    /** @brief 查询第k小元素(1-indexed) @param k 排名 @return 键值 */
    QPair<double, double> kthElement(int k) const;

    /** @brief 查询元素排名(1-indexed) @param key 键 @return 排名(0=不存在) */
    int rank(double key) const;

    /** @brief 区间统计[l,r]内元素值之和 @param left 左边界 @param right 右边界 @return 区间和 */
    double rangeSum(double left, double right) const;

    /** @brief 区间统计[l,r]内元素数 @param left 左边界 @param right 右边界 @return 元素数 */
    int rangeCount(double left, double right) const;

    /** @brief 获取元素总数 @return 元素数 */
    int size() const;

    /** @brief 获取树高度 @return 高度 */
    int height() const;

    /** @brief 中序遍历 @return (键,值)列表 */
    QList<QPair<double, double>> inorderTraversal() const;

    /** @brief 清空树 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 元素插入 @param key 键 */
    void elementInserted(double key);

    /** @brief 元素删除 @param key 键 */
    void elementRemoved(double key);

    /** @brief 旋转操作 @param type 旋转类型(LL/RR/LR/RL) */
    void rotationPerformed(const QString& type);

private:
    /** @brief 树节点 */
    struct Node {
        double key = 0.0;       ///< 键
        double value = 0.0;     ///< 值
        int height = 1;         ///< 子树高度
        int subtreeSize = 1;    ///< 子树节点数
        double subtreeSum = 0.0;///< 子树值之和
        Node* left = nullptr;   ///< 左子节点
        Node* right = nullptr;  ///< 右子节点
    };

    Node* insertNode(Node* node, double key, double value);
    Node* removeNode(Node* node, double key, bool& removed);
    bool containsNode(Node* node, double key) const;
    Node* kthNode(Node* node, int k) const;
    int rankOf(Node* node, double key) const;
    double rangeSumOf(Node* node, double left, double right) const;
    int rangeCountOf(Node* node, double left, double right) const;
    void inorder(Node* node, QList<QPair<double, double>>& result) const;
    void destroyTree(Node* node);

    Node* rotateLeft(Node* node);
    Node* rotateRight(Node* node);
    Node* balance(Node* node);
    void updateNode(Node* node);
    Node* findMin(Node* node) const;

    int getHeight(Node* node) const;
    int getSize(Node* node) const;
    double getSum(Node* node) const;
    int getBalanceFactor(Node* node) const;

    Node* m_root = nullptr;  ///< 根节点
    int m_size = 0;          ///< 元素总数

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;
};
