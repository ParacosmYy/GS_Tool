/**
 * @file RadixTree.h
 * @brief 基数树(Patricia Trie) — 字符串键压缩前缀树
 *
 * 功能: 插入/删除/查找/前缀搜索，路径压缩节省空间，
 *       统计操作次数/耗时，插入信号。
 */
#ifndef RADIXTREE_H
#define RADIXTREE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

class RadixTree : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalRemoves = 0;
        quint64 totalLookups = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit RadixTree(QObject* parent = nullptr);
    ~RadixTree();

    /** @brief 插入键值对 @param key 字符串键 @param value 整数值 */
    void insert(const QString& key, int value);

    /** @brief 删除键 @param key 要删除的键 @return 是否删除成功 */
    bool remove(const QString& key);

    /** @brief 精确查找 @param key 查找键 @param value 输出值 @return 是否找到 */
    bool lookup(const QString& key, int& value);

    /** @brief 前缀搜索 @param prefix 前缀 @return 所有匹配的键值对 */
    QVector<QPair<QString, int>> startsWith(const QString& prefix);

    /** @brief 当前树中的键数量 */
    int size() const { return m_size; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 条目插入信号 @param key 插入的键 */
    void entryInserted(QString key);

private:
    /** 基数树节点 */
    struct Node {
        QString edgeLabel;          ///< 从父节点到此节点的边标签
        int value = 0;              ///< 存储值(仅终止节点有效)
        bool isTerminal = false;    ///< 是否为终止节点
        QMap<QChar, Node*> children;///< 子节点
        Node* parent = nullptr;     ///< 父节点

        Node() = default;
    };

    void collectAll(Node* node, const QString& prefix,
                    QVector<QPair<QString, int>>& results);
    void clearNode(Node* node);

    Node m_root;       ///< 根节点
    int m_size;        ///< 键数量
    Stats m_stats;
    double m_timeSum;
};

#endif // RADIXTREE_H
