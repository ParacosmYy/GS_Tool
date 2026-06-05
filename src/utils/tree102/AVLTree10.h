#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief AVL平衡二叉搜索树
 *
 * 自平衡二叉搜索树实现，保证插入/删除/查找均为O(log n)，
 * 支持旋转操作维持平衡因子。
 */
class AVLTree10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalInserts = 0;        ///< 插入操作总数
        int totalRemoves = 0;        ///< 删除操作总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit AVLTree10(QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(double key, int data);
    /** @brief 删除指定键 */
    void remove(double key);
    /** @brief 查询键是否存在 */
    bool contains(double key) const;
    /** @brief 获取树高度 */
    int height() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成，返回键值 */
    void inserted(double key);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
