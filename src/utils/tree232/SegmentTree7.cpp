/**
 * @file SegmentTree7.cpp
 * @brief SegmentTree7 实现
 *
 * 实现线段树：节拍节奏音乐时间查询与持久化版本撤销重做。
 */

#include "utils/tree232/SegmentTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SegmentTree7::SegmentTree7(QObject *parent) : QObject(parent) {}
SegmentTree7::~SegmentTree7() = default;

/* ---- Set range ---- */

void SegmentTree7::setRange(int totalBeats)
{
    m_totalBeats = qMax(1, totalBeats);
    m_nodePool.clear();
    m_versions.clear();
    m_currentVersionIdx = -1;
    m_nextVersionId = 0;
    m_currentRoot = buildTree(0.0, static_cast<double>(m_totalBeats));
    commitVersion("initial");
}

/* ---- Allocate node (copy-on-write) ---- */

int SegmentTree7::allocNode(const Node& src)
{
    int idx = m_nodePool.size();
    m_nodePool.append(src);
    return idx;
}

/* ---- Build initial tree ---- */

int SegmentTree7::buildTree(double lo, double hi)
{
    Node node;
    node.rangeLo = lo;
    node.rangeHi = hi;
    node.totalDuration = 0.0;
    node.lazyClear = false;

    if (hi - lo <= 1.0) {
        node.left = -1;
        node.right = -1;
    } else {
        double mid = qFloor((lo + hi) / 2.0);
        node.left = buildTree(lo, mid);
        node.right = buildTree(mid, hi);
    }
    return allocNode(node);
}

/* ---- Insert event ---- */

void SegmentTree7::insertEvent(const BeatEvent& event)
{
    QElapsedTimer timer;
    timer.start();

    m_currentRoot = insertAt(m_currentRoot, event);

    m_stats.numEvents++;
    m_stats.treeSize = m_nodePool.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit eventInserted(event.beat, event.pitch);
}

/* ---- Insert at node (persistent) ---- */

int SegmentTree7::insertAt(int nodeIdx, const BeatEvent& event)
{
    if (nodeIdx < 0) return nodeIdx;

    // Copy-on-write: duplicate the node
    int newIdx = allocNode(m_nodePool[nodeIdx]);
    Node& node = m_nodePool[newIdx];

    // If event fully contained in this segment, store it here
    if (event.beat >= node.rangeLo && event.beat < node.rangeHi) {
        // Check if we should push down to children
        if (node.left >= 0 && node.rangeHi - node.rangeLo > 1.0) {
            double mid = qFloor((node.rangeLo + node.rangeHi) / 2.0);
            if (event.beat < mid) {
                int newLeft = insertAt(node.left, event);
                m_nodePool[newIdx].left = newLeft;
            } else {
                int newRight = insertAt(node.right, event);
                m_nodePool[newIdx].right = newRight;
            }
        } else {
            // Leaf: store event
            node.events.append(event);
        }
        node.totalDuration += event.duration;
    }
    return newIdx;
}

/* ---- Remove range ---- */

void SegmentTree7::removeRange(double lo, double hi)
{
    QElapsedTimer timer;
    timer.start();

    int countBefore = 0;
    QVector<BeatEvent> events = queryRange(lo, hi);
    countBefore = events.size();

    m_currentRoot = removeAt(m_currentRoot, lo, hi);

    m_stats.numEvents -= countBefore;
    m_stats.treeSize = m_nodePool.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit rangeRemoved(lo, hi, countBefore);
}

/* ---- Remove at node (persistent) ---- */

int SegmentTree7::removeAt(int nodeIdx, double lo, double hi)
{
    if (nodeIdx < 0) return nodeIdx;

    int newIdx = allocNode(m_nodePool[nodeIdx]);
    Node& node = m_nodePool[newIdx];

    // Check overlap
    if (node.rangeHi <= lo || node.rangeLo >= hi) return newIdx;

    // Remove matching events
    double durRemoved = 0.0;
    QVector<BeatEvent> kept;
    for (const auto& ev : node.events) {
        if (ev.beat >= lo && ev.beat < hi) {
            durRemoved += ev.duration;
        } else {
            kept.append(ev);
        }
    }
    node.events = kept;
    node.totalDuration -= durRemoved;

    // Recurse to children
    if (node.left >= 0) {
        int newLeft = removeAt(node.left, lo, hi);
        m_nodePool[newIdx].left = newLeft;
    }
    if (node.right >= 0) {
        int newRight = removeAt(node.right, lo, hi);
        m_nodePool[newIdx].right = newRight;
    }

    // Push lazy clear
    if (node.lazyClear) {
        newIdx = pushLazy(newIdx);
    }

    return newIdx;
}

/* ---- Push lazy ---- */

int SegmentTree7::pushLazy(int nodeIdx)
{
    Node& node = m_nodePool[nodeIdx];
    node.lazyClear = false;
    node.events.clear();
    node.totalDuration = 0.0;
    return nodeIdx;
}

/* ---- Query beat ---- */

QVector<SegmentTree7::BeatEvent> SegmentTree7::queryBeat(double beat) const
{
    QVector<BeatEvent> result;
    queryBeatAt(m_currentRoot, beat, result);
    return result;
}

/* ---- Query beat at node ---- */

void SegmentTree7::queryBeatAt(int nodeIdx, double beat, QVector<BeatEvent>& result) const
{
    if (nodeIdx < 0) return;
    const Node& node = m_nodePool[nodeIdx];

    if (beat < node.rangeLo || beat >= node.rangeHi) return;

    // Check events at this node
    for (const auto& ev : node.events) {
        if (beat >= ev.beat && beat < ev.beat + ev.duration)
            result.append(ev);
    }

    // Recurse
    queryBeatAt(node.left, beat, result);
    queryBeatAt(node.right, beat, result);
}

/* ---- Query range ---- */

QVector<SegmentTree7::BeatEvent> SegmentTree7::queryRange(double lo, double hi) const
{
    QVector<BeatEvent> result;
    queryRangeAt(m_currentRoot, lo, hi, result);
    return result;
}

/* ---- Query range at node ---- */

void SegmentTree7::queryRangeAt(int nodeIdx, double lo, double hi,
                                   QVector<BeatEvent>& result) const
{
    if (nodeIdx < 0) return;
    const Node& node = m_nodePool[nodeIdx];

    if (node.rangeHi <= lo || node.rangeLo >= hi) return;

    for (const auto& ev : node.events) {
        if (ev.beat < hi && ev.beat + ev.duration > lo)
            result.append(ev);
    }

    queryRangeAt(node.left, lo, hi, result);
    queryRangeAt(node.right, lo, hi, result);
}

/* ---- Range duration ---- */

double SegmentTree7::rangeDuration(double lo, double hi) const
{
    return durationAt(m_currentRoot, lo, hi);
}

/* ---- Duration at node ---- */

double SegmentTree7::durationAt(int nodeIdx, double lo, double hi) const
{
    if (nodeIdx < 0) return 0.0;
    const Node& node = m_nodePool[nodeIdx];

    if (node.rangeHi <= lo || node.rangeLo >= hi) return 0.0;
    if (lo <= node.rangeLo && hi >= node.rangeHi) return node.totalDuration;

    double dur = 0.0;
    for (const auto& ev : node.events) {
        if (ev.beat < hi && ev.beat + ev.duration > lo)
            dur += qMin(ev.beat + ev.duration, hi) - qMax(ev.beat, lo);
    }

    dur += durationAt(node.left, lo, hi);
    dur += durationAt(node.right, lo, hi);
    return dur;
}

/* ---- Commit version ---- */

int SegmentTree7::commitVersion(const QString& description)
{
    Version ver;
    ver.versionId = m_nextVersionId++;
    ver.rootIndex = m_currentRoot;
    ver.description = description;

    if (m_currentVersionIdx >= 0)
        ver.parentVersion = m_versions[m_currentVersionIdx].versionId;
    else
        ver.parentVersion = -1;

    // Truncate future versions if we're not at the tip
    if (m_currentVersionIdx >= 0 && m_currentVersionIdx < m_versions.size() - 1)
        m_versions.resize(m_currentVersionIdx + 1);

    m_versions.append(ver);
    m_currentVersionIdx = m_versions.size() - 1;

    m_stats.numVersions = m_versions.size();
    emit versionCommitted(ver.versionId, description);
    return ver.versionId;
}

/* ---- Undo ---- */

bool SegmentTree7::undo()
{
    if (m_currentVersionIdx <= 0) return false;
    m_currentVersionIdx--;
    m_currentRoot = m_versions[m_currentVersionIdx].rootIndex;
    emit versionRestored(m_versions[m_currentVersionIdx].versionId);
    return true;
}

/* ---- Redo ---- */

bool SegmentTree7::redo()
{
    if (m_currentVersionIdx >= m_versions.size() - 1) return false;
    m_currentVersionIdx++;
    m_currentRoot = m_versions[m_currentVersionIdx].rootIndex;
    emit versionRestored(m_versions[m_currentVersionIdx].versionId);
    return true;
}

/* ---- Current version ---- */

int SegmentTree7::currentVersion() const
{
    if (m_currentVersionIdx < 0) return -1;
    return m_versions[m_currentVersionIdx].versionId;
}

/* ---- Version history ---- */

QVector<SegmentTree7::Version> SegmentTree7::versionHistory() const
{
    return m_versions;
}

/* ---- Reset ---- */

void SegmentTree7::resetStatistics()
{
    m_nodePool.clear();
    m_versions.clear();
    m_currentRoot = -1;
    m_currentVersionIdx = -1;
    m_nextVersionId = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
