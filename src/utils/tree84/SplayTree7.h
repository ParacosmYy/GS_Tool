#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 伸展树(Splay Tree)
 *
 * 自调整二叉搜索树，最近访问的节点自动移至根位置。
 */
class SplayTree7 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalAccesses = 0;
        int totalSplayOps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree7(QObject* parent = nullptr);

    /** @brief 插入键并伸展至根 */
    void insert(int key);

    /** @brief 查找键并伸展 */
    bool find(int key);

    /** @brief 返回有序键值列表 */
    QVector<int> inOrderKeys() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void splayPerformed(int key, int rotations);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct SplayNode* m_root = nullptr;
};
