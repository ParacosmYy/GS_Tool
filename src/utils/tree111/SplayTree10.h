#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 伸展树(Splay Tree)实现
 *
 * 自调整二叉搜索树，每次访问操作后将节点旋转至根，
 * 具有优异的局部性性能，适用于缓存、垃圾回收和频繁访问场景。
 */
class SplayTree10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit SplayTree10(QObject* parent = nullptr);

    /** @brief 插入键值对，插入后自动将节点伸展至根 */
    void insert(int key, const QVariant& value);

    /** @brief 查找指定键，找到后伸展至根并返回值 */
    QVariant search(int key);

    /** @brief 删除指定键，删除后合并左右子树 */
    bool remove(int key);

    /** @brief 中序遍历返回所有键值对，用于调试和序列化 */
    QVector<QPair<int, QVariant>> inOrderTraversal() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 操作完成信号，返回操作类型和当前树大小 */
    void operationCompleted(const QString& operationType, int treeSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
