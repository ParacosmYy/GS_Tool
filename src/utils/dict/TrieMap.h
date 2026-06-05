/**
 * @file TrieMap.h
 * @brief Trie字典树映射 — 高效前缀查找与键值存储
 *
 * 功能: 基于Trie(前缀树)数据结构实现字符串键到double值的
 *       映射，支持前缀查询、插入、删除操作。
 *
 * 协作: DataPatternDetector(模式检测) / FrequencyCounterWidget(频率统计)
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QElapsedTimer>

/**
 * @brief Trie字典树映射
 */
class TrieMap : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;        ///< 累计插入次数
        quint64 totalLookups = 0;        ///< 累计查找次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit TrieMap(QObject* parent = nullptr);
    ~TrieMap() override;

    /**
     * @brief 插入键值对
     * @param key 字符串键
     * @param value 关联值
     */
    void insert(const QString& key, double value);

    /**
     * @brief 查找键对应的值
     * @param key 字符串键
     * @return 关联值(不存在返回NaN)
     */
    double lookup(const QString& key) const;

    /**
     * @brief 查询指定前缀的所有键
     * @param prefix 前缀字符串
     * @return 匹配的键列表
     */
    QVector<QString> keysWithPrefix(const QString& prefix) const;

    /**
     * @brief 删除键
     * @param key 待删除的键
     */
    void remove(const QString& key);

    /** @brief 当前节点总数 */
    int nodeCount() const { return m_nodeCount; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 结构变更 @param nodeCount 当前节点数 */
    void structureChanged(int nodeCount);

private:
    /**
     * @brief Trie节点
     */
    struct Node {
        QMap<QChar, Node*> children;  ///< 子节点
        double  value = 0.0;          ///< 存储值
        bool    isEnd = false;        ///< 是否为键终止节点
    };

    /** @brief 递归收集前缀下的所有键 */
    void collectKeys(Node* node, const QString& prefix,
                     QVector<QString>& result) const;

    /** @brief 递归删除子树 */
    void deleteSubtree(Node* node);

    /** @brief 递归删除节点(修剪空分支) */
    bool removeHelper(Node* node, const QString& key, int depth);

    Node*  m_root;       ///< 根节点
    int    m_nodeCount;  ///< 节点总数

    mutable Stats        m_stats;
    mutable double       m_totalTimeMs = 0.0;
    mutable QElapsedTimer m_timer;
};
