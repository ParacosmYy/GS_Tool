/**
 * @file ScapegoatTree3.h
 * @brief 替罪羊树(增强版) — 平衡因子/重建触发/扁平化/权重维护
 *
 * 功能: 增强型替罪羊树，支持可配置平衡因子α、重建触发阈值、
 *       扁平化重建、权重维护、范围查询、秩查询(k-th)、
 *       统计插入/删除/重建/查询次数与平均处理耗时。
 */
#ifndef SCAPEGOATTREE3_H
#define SCAPEGOATTREE3_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class ScapegoatTree3
 * @brief 增强型替罪羊树，支持权重维护和秩查询
 */
class ScapegoatTree3 : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;           ///< 累计插入次数
        quint64 totalDeletes = 0;           ///< 累计删除次数
        quint64 totalRebuilds = 0;          ///< 累计重建次数
        quint64 totalQueries = 0;           ///< 累计查询次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit ScapegoatTree3(QObject* parent = nullptr);
    ~ScapegoatTree3();

    /** 设置平衡因子α(0.5~0.99)，越小越严格 */
    void setAlpha(double alpha);

    /** 插入键值 */
    void insert(double key);

    /** 删除键值 */
    void remove(double key);

    /** 查询键是否存在 */
    bool contains(double key) const;

    /** 范围查询: 返回[lo, hi]内所有键 */
    QVector<double> rangeQuery(double lo, double hi) const;

    /** 秩查询: 返回第k小元素(k从1开始) */
    double selectKth(int k) const;

    /** 查询键的排名(从1开始) */
    int rank(double key) const;

    /** 树中元素数 */
    int size() const { return m_size; }

    /** 中序遍历 */
    QVector<double> inOrder() const;

    /** 验证当前树的平衡性 */
    bool validateBalance() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** 子树重建信号 */
    void rebuilt(int subtreeSize, int depth);
    /** 失衡检测信号 */
    void imbalanceDetected(double leftRatio, double rightRatio);

private:
    /** 树节点 */
    struct Node {
        double key;         ///< 键值
        Node* left;         ///< 左子树
        Node* right;        ///< 右子树
        int subSize;        ///< 子树大小(含自身)
        int weight;         ///< 权重(考虑已删除的惰性标记)

        explicit Node(double k)
            : key(k), left(nullptr), right(nullptr), subSize(1), weight(1) {}
    };

    /** 插入并搜索替罪羊 */
    Node* insertInternal(Node* node, double key, bool& rebuilt, int depth);
    /** 删除节点 */
    Node* removeInternal(Node* node, double key);
    /** 检查α不平衡 */
    bool isUnbalanced(Node* node) const;
    /** 重建子树 */
    Node* rebuildSubtree(Node* node, int depth);
    /** 扁平化为有序数组 */
    void flatten(Node* node, QVector<Node*>& sorted);
    /** 从有序数组构建完美平衡树 */
    Node* buildBalanced(const QVector<Node*>& sorted, int lo, int hi);
    /** 更新子树大小 */
    void updateSubSize(Node* node);
    /** 递归销毁 */
    void destroyTree(Node* node);
    /** 范围查询递归 */
    void rangeCollect(Node* node, double lo, double hi,
                      QVector<double>& result) const;
    /** 验证平衡性递归 */
    bool validateNode(Node* node) const;

    Node* m_root;
    int m_size;
    double m_alpha;
    Stats m_stats;
    double m_timeSum;
};

#endif // SCAPEGOATTREE3_H
