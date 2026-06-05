#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AVL平衡二叉搜索树工具类
 *
 * 提供AVL树的插入、删除和查找操作，所有操作保证
 * O(log n)时间复杂度，节点通过自旋保持平衡。
 */
class AVLTree9 : public QObject {
    Q_OBJECT
public:
    /// 操作统计信息
    struct Stats {
        int totalOperations = 0;    ///< 总操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit AVLTree9(QObject* parent = nullptr);

    /** @brief 插入一个键值对(key为排序键，value为关联数据) */
    void insert(double key, int value);

    /** @brief 删除指定键的节点 */
    void remove(double key);

    /** @brief 查找指定键是否存在 */
    bool contains(double key);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成信号，返回插入的键 */
    void inserted(double key);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    /// AVL树节点结构
    struct Node {
        double key = 0.0;
        int value = 0;
        int height = 1;
        Node* left = nullptr;
        Node* right = nullptr;
    };
    Node* m_root = nullptr;
};
