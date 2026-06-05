/**
 * @file AVLTree2.h
 * @brief 增强AVL树 — 带子树规模的顺序统计操作
 *
 * 功能: 实现AVL平衡二叉搜索树，每个节点维护子树规模信息，
 *       支持按秩查询、排名查询、中序遍历范围查询等顺序统计操作，
 *       适用于时间戳索引、滑动窗口中位数、优先级队列等场景。
 *
 * 协作: DataWindowManager(窗口管理) / TrendPredictor(趋势预测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 增强AVL树 — 顺序统计操作
 *
 * 典型用法:
 * @code
 *   AVLTree2 tree;
 *   tree.insert(5.0);
 *   tree.insert(3.0);
 *   double median = tree.kthElement(1);
 *   int rank = tree.rank(4.0);
 * @endcode
 */
class AVLTree2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计数据 */
    struct Stats {
        int totalInsertions = 0;                ///< 累计插入次数
        int totalDeletions = 0;                 ///< 累计删除次数
        int totalQueries = 0;                   ///< 累计查询次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
        int totalRotations = 0;                 ///< 累计旋转次数
    };

    explicit AVLTree2(QObject* parent = nullptr);
    ~AVLTree2() override;

    /** @brief 禁止拷贝 */
    AVLTree2(const AVLTree2&) = delete;
    AVLTree2& operator=(const AVLTree2&) = delete;

    /**
     * @brief 插入值
     * @param value 待插入值
     */
    void insert(double value);

    /**
     * @brief 删除值
     * @param value 待删除值
     * @return 是否成功删除
     */
    bool remove(double value);

    /**
     * @brief 查询值是否存在
     * @param value 查询值
     * @return 是否存在
     */
    bool contains(double value) const;

    /**
     * @brief 查询值的排名(从0开始)
     * @param value 查询值
     * @return 排名(小于value的元素个数)
     */
    int rank(double value) const;

    /**
     * @brief 按秩查询第k小的值
     * @param k 秩(从0开始)
     * @return 第k小的值; 越界返回NaN
     */
    double kthElement(int k) const;

    /**
     * @brief 查询中位数
     * @return 中位数值
     */
    double median() const;

    /**
     * @brief 范围查询: 统计区间[a,b]内的元素个数
     * @param a 区间左端
     * @param b 区间右端
     * @return 元素个数
     */
    int rangeCount(double a, double b) const;

    /**
     * @brief 范围查询: 获取区间[a,b]内的所有值(升序)
     * @param a 区间左端
     * @param b 区间右端
     * @return 值列表
     */
    QVector<double> rangeQuery(double a, double b) const;

    /**
     * @brief 前驱: 小于value的最大值
     * @param value 查询值
     @return 前驱值; 不存在返回NaN
     */
    double predecessor(double value) const;

    /**
     * @brief 后继: 大于value的最小值
     * @param value 查询值
     * @return 后继值; 不存在返回NaN
     */
    double successor(double value) const;

    /** @brief 获取元素总数 @return 元素数 */
    int size() const;

    /** @brief 获取树高度 @return 高度 */
    int height() const;

    /** @brief 清空树 */
    void clear();

    /** @brief 获取统计 @return 统计数据 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 元素插入 @param value 插入值 @param newSize 新大小 */
    void elementInserted(double value, int newSize);

    /** @brief 元素删除 @param value 删除值 @param success 是否成功 */
    void elementRemoved(double value, bool success);

private:
    /** @brief AVL节点 */
    struct Node {
        double value = 0.0;            ///< 节点值
        int height = 1;                ///< 节点高度
        int subtreeSize = 1;           ///< 子树规模(含自身)
        Node* left = nullptr;          ///< 左子节点
        Node* right = nullptr;         ///< 右子节点
    };

    /** @brief 获取节点高度 */
    static int nodeHeight(const Node* node);

    /** @brief 获取子树规模 */
    static int nodeSize(const Node* node);

    /** @brief 更新节点高度和规模 */
    static void updateNode(Node* node);

    /** @brief 平衡因子 */
    static int balanceFactor(const Node* node);

    /** @brief 右旋 */
    Node* rotateRight(Node* y);

    /** @brief 左旋 */
    Node* rotateLeft(Node* x);

    /** @brief 平衡化 */
    Node* balance(Node* node);

    /** @brief 递归插入 */
    Node* insertNode(Node* node, double value);

    /** @brief 递归删除 */
    Node* removeNode(Node* node, double value, bool& removed);

    /** @brief 找最小节点 */
    static Node* findMin(Node* node);

    /** @brief 递归查询排名 */
    int rankQuery(const Node* node, double value) const;

    /** @brief 递归按秩查询 */
    double kthQuery(const Node* node, int k) const;

    /** @brief 递归范围计数 */
    int rangeCountQuery(const Node* node, double a, double b) const;

    /** @brief 递归范围查询 */
    void rangeCollect(const Node* node, double a, double b,
                      QVector<double>& result) const;

    /** @brief 递归销毁 */
    static void destroyTree(Node* node);

    Node* m_root = nullptr;            ///< 根节点
    Stats m_stats;                     ///< 统计数据
    double m_timeSum = 0.0;           ///< 时间累加器
};
