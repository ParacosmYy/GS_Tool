/**
 * @file AvlTree2.h
 * @brief 增强AVL树 — 支持秩查询和范围计数的平衡二叉搜索树
 *
 * 在标准AVL自平衡树基础上, 每个节点维护子树大小(subtreeSize)，
 * 支持 O(log n) 的 k-th 查询、秩查询(rank)和范围计数(rangeCount)。
 * 适用于串口数据的中位数计算、百分位数查询、数据分布分析等场景。
 */
#pragma once

#include <QObject>
#include <QVariant>
#include <QVector>
#include <QElapsedTimer>

/**
 * @class AvlTree2
 * @brief 增强AVL树 — 带子树大小的平衡BST, 支持秩查询
 *
 * 典型用法:
 * @code
 *   AvlTree2 tree;
 *   tree.insert(3.14, "pi");
 *   tree.insert(2.72, "e");
 *   QVariant val = tree.find(3.14);
 *   double median = tree.kth(1);
 *   int count = tree.rangeCount(2.0, 4.0);
 * @endcode
 */
class AvlTree2 : public QObject {
    Q_OBJECT

public:
    /** @brief 树操作统计结构 */
    struct Stats {
        quint64 totalInserts         = 0;    ///< 插入操作总次数
        quint64 totalRemoves         = 0;    ///< 删除操作总次数
        quint64 totalSearches        = 0;    ///< 搜索操作总次数
        double  avgProcessingTimeMs  = 0.0;  ///< 平均操作耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit AvlTree2(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~AvlTree2() override;

    // ── 核心操作 ──

    /**
     * @brief 插入键值对
     *
     * 若key已存在则更新value, 不增加节点数。
     * 插入后自底向上维护平衡和subtreeSize。
     * @param key   键(需可比较)
     * @param value 关联值
     */
    void insert(double key, const QVariant& value);

    /**
     * @brief 删除指定键
     * @param key 待删除的键
     * @return true = 成功删除, false = 键不存在
     */
    bool remove(double key);

    /**
     * @brief 查找键对应的值
     * @param key 查找键
     * @return 关联值; 不存在返回无效QVariant
     */
    QVariant find(double key);

    // ── 增强查询(基于subtreeSize) ──

    /**
     * @brief 查询第k小的键(0-indexed)
     *
     * 利用subtreeSize在O(log n)内定位第k个节点。
     * @param k 排名索引(0 = 最小键)
     * @return 第k小的键; k越界返回NaN
     */
    double kth(int k);

    /**
     * @brief 查询键的排名(小于key的键数量)
     *
     * 统计左子树路径上的subtreeSize之和。
     * @param key 查询键
     * @return 严格小于key的键数量
     */
    int rank(double key);

    /**
     * @brief 范围计数: 统计[lo, hi]区间内的键数量
     * @param lo 下界(包含)
     * @param hi 上界(包含)
     * @return [lo, hi]区间内的键数量
     */
    int rangeCount(double lo, double hi);

    /**
     * @brief 中序遍历返回所有键(升序)
     * @return 升序排列的键列表
     */
    QVector<double> inOrder();

    /** @brief 节点总数 @return 树中的节点数 */
    int size() const;

    /** @brief 树是否为空 @return true = 空 */
    bool isEmpty() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 节点插入完成信号 @param key 插入的键 */
    void nodeInserted(double key);
    /** @brief 节点删除完成信号 @param key 删除的键 */
    void nodeRemoved(double key);

private:
    /** @brief AVL树节点(带子树大小) */
    struct Node {
        double     key;           ///< 键
        QVariant   value;         ///< 关联值
        Node*      left;          ///< 左子节点
        Node*      right;         ///< 右子节点
        int        height;        ///< 节点高度
        int        subtreeSize;   ///< 以此节点为根的子树节点总数

        explicit Node(double k, const QVariant& v)
            : key(k), value(v), left(nullptr), right(nullptr)
            , height(1), subtreeSize(1) {}
    };

    /** @brief 递归销毁子树 */
    void destroy(Node* node);

    /** @brief 递归插入 */
    Node* insertNode(Node* node, double key, const QVariant& value, bool& inserted);

    /** @brief 递归删除 */
    Node* removeNode(Node* node, double key, bool& removed);

    /** @brief 递归查找 */
    Node* findNode(Node* node, double key) const;

    /** @brief 旋转辅助 */
    Node* rotateRight(Node* y);
    Node* rotateLeft(Node* x);
    Node* balance(Node* node);

    /** @brief 更新节点高度和subtreeSize */
    void updateNode(Node* node);

    /** @brief 获取平衡因子 */
    int balanceFactor(Node* node) const;

    /** @brief 获取节点高度 */
    int heightOf(Node* node) const;

    /** @brief 获取子树大小 */
    int sizeOf(Node* node) const;

    /** @brief 中序遍历递归 */
    void inOrderHelper(Node* node, QVector<double>& result) const;

    /** @brief 更新统计平均耗时 */
    void updateAvgTime(double elapsed);

    Node* m_root;            ///< 树根节点
    Stats m_stats;           ///< 统计数据
    QElapsedTimer m_timer;   ///< 耗时计时器
};
