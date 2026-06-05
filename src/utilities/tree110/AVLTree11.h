#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AVL平衡二叉搜索树实现 (11种操作)
 *
 * 提供自平衡BST，保证O(log n)的查找/插入/删除操作，支持范围查询和中序遍历。
 */
class AVLTree11 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalOperations = 0;        ///< 总操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int treeHeight = 0;             ///< 当前树高度
    };

    explicit AVLTree11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 插入键值对
     * @param key 排序键
     * @param value 关联值
     */
    void insert(double key, const QVariant& value);

    /**
     * @brief 删除指定键
     * @param key 待删除的键
     * @return 是否删除成功
     */
    bool remove(double key);

    /**
     * @brief 查找指定键的值
     * @param key 搜索键
     * @return 对应的值，无效表示未找到
     */
    QVariant search(double key) const;

    /**
     * @brief 范围查询 [minKey, maxKey]
     * @param minKey 范围下界
     * @param maxKey 范围上界
     * @return 范围内的键值对列表
     */
    QVector<QPair<double, QVariant>> rangeQuery(double minKey, double maxKey) const;

    /**
     * @brief 获取中序遍历结果
     * @return 有序键值对列表
     */
    QVector<QPair<double, QVariant>> inorderTraversal() const;

signals:
    /// 操作完成信号
    void operationCompleted(const QString& operation, int treeHeight);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct Node;
    Node* m_root = nullptr;
    int m_size = 0;
};
