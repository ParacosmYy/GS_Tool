/**
 * @file Rope13.h
 * @brief Rope数据结构(写时复制子树共享与路径复制实现函数式持久化文本编辑) — Rope with Copy-on-Write Subtree Sharing and Path Copying for Functional Persistent Text Editing with Full Version History
 *
 * 功能: 实现Rope数据结构(rope data structure)，采用写时复制子树共享(copy-on-write subtree sharing)
 *       与路径复制(path copying)实现函数式持久化文本编辑(functional persistent text editing with full version history)。
 *
 * 协作: BTree11(B树) / Trie(字典树) / PersistentVector(持久化向量)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

class Rope13 : public QObject {
    Q_OBJECT

public:
    /** @brief Rope node (immutable with COW) */
    struct Node {
        bool isLeaf = true;
        QString text;                // leaf content
        int weight = 0;              // left subtree char count (leaf: text length)
        int leftIdx = -1;            // index into node pool
        int rightIdx = -1;
        int refCount = 1;
        int version = 0;             // version this node was created
    };

    /** @brief Edit result with version tracking */
    struct EditResult {
        int rootIdx = -1;
        int version = 0;
        int length = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Version snapshot */
    struct VersionInfo {
        int rootIdx = -1;
        int length = 0;
        int editCount = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalEdits = 0;
        int numVersions = 0;
        int numNodes = 0;
        int totalLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope13(QObject *parent = nullptr);
    ~Rope13() override;

    void setLeafSize(int size);

    /** @brief Build rope from initial string */
    EditResult build(const QString& text);

    /** @brief Insert text at position (returns new version root) */
    EditResult insert(int rootIdx, int position, const QString& text);

    /** @brief Delete range [from, to) (returns new version root) */
    EditResult remove(int rootIdx, int from, int to);

    /** @brief Get full text from a specific version */
    QString toString(int rootIdx) const;

    /** @brief Get character at index */
    QChar at(int rootIdx, int index) const;

    /** @brief Get substring [from, to) */
    QString substring(int rootIdx, int from, int to) const;

    /** @brief Get current version info */
    const QVector<VersionInfo>& versions() const { return m_versions; }

    int currentRoot() const { return m_currentRoot; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void editDone(int version, int length, double timeMs);

private:
    int m_leafSize = 64;
    QVector<Node> m_nodes;
    int m_currentRoot = -1;
    int m_currentVersion = 0;
    QVector<VersionInfo> m_versions;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node (returns index) */
    int allocNode();

    /** @brief Deep-copy node for COW path copying */
    int cloneNode(int idx);

    /** @brief Build rope recursively */
    int buildHelper(const QString& text, int start, int end);

    /** @brief Compute weight (total chars in left subtree + self) */
    int computeWeight(int idx) const;

    /** @brief Split rope at position, returns two root indices */
    QPair<int, int> split(int idx, int position);

    /** @brief Concatenate two ropes */
    int concat(int leftIdx, int rightIdx);

    /** @brief Collect text recursively */
    void collectText(int idx, QString& out) const;

    /** @brief Rebalance if needed */
    int rebalance(int idx);

    /** @brief Check if node needs rebalancing */
    bool isUnbalanced(int idx) const;
};
