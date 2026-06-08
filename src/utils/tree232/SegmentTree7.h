/**
 * @file SegmentTree7.h
 * @brief 线段树(节拍节奏音乐时间查询+持久化版本撤销重做) — Segment Tree with Beat-Based Rhythm for Musical Time Queries and Persistent Versioning for Undo/Redo Timeline
 *
 * 功能: 实现线段树(segment tree)数据结构，支持基于节拍(beat)的音乐时间查询，
 *       通过持久化版本(persistent versioning)实现完整的撤销/重做(undo/redo)时间线。
 *
 * 协作: BTree6(B树) / AVLTree5(AVL树) / FenwickTree4(树状数组)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 线段树(节拍节奏音乐时间查询+持久化版本撤销重做)
 */
class SegmentTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief Musical event at a beat position */
    struct BeatEvent {
        double beat = 0.0;
        double duration = 1.0;     // in beats
        int pitch = 60;            // MIDI note number
        int velocity = 100;
        int channel = 0;
        QString label;
    };

    /** @brief Persistent version snapshot */
    struct Version {
        int versionId = -1;
        int parentVersion = -1;
        QString description;
        int rootIndex = -1;        // root node index in node pool
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBeats = 0;
        int numEvents = 0;
        int numVersions = 0;
        int treeSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SegmentTree7(QObject *parent = nullptr);
    ~SegmentTree7() override;

    /** @brief Set total beat range [0, totalBeats) */
    void setRange(int totalBeats);

    /** @brief Insert a musical event at a beat position */
    void insertEvent(const BeatEvent& event);

    /** @brief Remove events in beat range [lo, hi) */
    void removeRange(double lo, double hi);

    /** @brief Query all events overlapping a beat position */
    QVector<BeatEvent> queryBeat(double beat) const;

    /** @brief Query all events in beat range [lo, hi) */
    QVector<BeatEvent> queryRange(double lo, double hi) const;

    /** @brief Get total duration of events in range */
    double rangeDuration(double lo, double hi) const;

    /** @brief Create a new persistent version (for undo checkpoint) */
    int commitVersion(const QString& description = QString());

    /** @brief Undo: revert to previous version */
    bool undo();

    /** @brief Redo: move to next version */
    bool redo();

    /** @brief Get current version ID */
    int currentVersion() const;

    /** @brief Get version history */
    QVector<Version> versionHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void eventInserted(double beat, int pitch);
    void rangeRemoved(double lo, double hi, int count);
    void versionCommitted(int versionId, const QString& description);
    void versionRestored(int versionId);

private:
    int m_totalBeats = 64;

    /** @brief Segment tree node (path-copied for persistence) */
    struct Node {
        int left = -1;          // index in pool
        int right = -1;
        double rangeLo = 0.0;
        double rangeHi = 0.0;
        QVector<BeatEvent> events;
        double totalDuration = 0.0;
        bool lazyClear = false;
    };

    QVector<Node> m_nodePool;   // persistent node storage
    int m_currentRoot = -1;

    // Version control
    QVector<Version> m_versions;
    int m_currentVersionIdx = -1;
    int m_nextVersionId = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node (copy-on-write) */
    int allocNode(const Node& src);

    /** @brief Build initial empty tree */
    int buildTree(double lo, double hi);

    /** @brief Insert event into subtree rooted at nodeIdx, returns new root */
    int insertAt(int nodeIdx, const BeatEvent& event);

    /** @brief Remove range from subtree, returns new root */
    int removeAt(int nodeIdx, double lo, double hi);

    /** @brief Query events at a single beat position */
    void queryBeatAt(int nodeIdx, double beat, QVector<BeatEvent>& result) const;

    /** @brief Query events in range */
    void queryRangeAt(int nodeIdx, double lo, double hi,
                       QVector<BeatEvent>& result) const;

    /** @brief Compute total duration in range */
    double durationAt(int nodeIdx, double lo, double hi) const;

    /** @brief Push lazy clear tag */
    int pushLazy(int nodeIdx);
};
