/**
 * @file RingHash.cpp
 * @brief 一致性哈希实现
 */

#include "utils/ringhash/RingHash.h"

#include <QElapsedTimer>
#include <QCryptographicHash>

RingHash::RingHash(int virtualNodesPerNode, QObject* parent)
    : QObject(parent), m_vnodesPerNode(virtualNodesPerNode), m_timeSum(0.0) {}

void RingHash::addNode(const QString& node)
{
    QElapsedTimer timer;
    timer.start();

    if (m_nodes.contains(node)) return;

    m_nodes.insert(node);
    for (int i = 0; i < m_vnodesPerNode; ++i) {
        quint32 h = hashVirtual(node, i);
        m_ring[h] = node;
    }

    m_stats.totalAdds++;
    m_stats.nodeCount = m_nodes.size();
    m_stats.virtualNodes = m_ring.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalLookups, 1ULL);

    emit nodeAdded(node);
}

void RingHash::removeNode(const QString& node)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_nodes.contains(node)) return;

    m_nodes.remove(node);
    QVector<quint32> toRemove;
    for (auto it = m_ring.constBegin(); it != m_ring.constEnd(); ++it) {
        if (it.value() == node) toRemove.append(it.key());
    }
    for (quint32 h : toRemove) {
        m_ring.remove(h);
    }

    m_stats.totalRemoves++;
    m_stats.nodeCount = m_nodes.size();
    m_stats.virtualNodes = m_ring.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalLookups, 1ULL);

    emit nodeRemoved(node);
}

QString RingHash::lookup(const QString& key) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_ring.isEmpty()) return QString();

    quint32 h = hashKey(key);

    /* 找到第一个 >= h 的虚拟节点 */
    auto it = m_ring.lowerBound(h);
    if (it == m_ring.constEnd()) {
        it = m_ring.constBegin();  // 环绕
    }

    m_stats.totalLookups++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalLookups, 1ULL);

    return it.value();
}

QVector<QString> RingHash::lookupN(const QString& key, int count) const
{
    QVector<QString> result;
    if (m_ring.isEmpty() || count <= 0) return result;

    quint32 h = hashKey(key);
    auto it = m_ring.lowerBound(h);
    if (it == m_ring.constEnd()) it = m_ring.constBegin();

    QSet<QString> seen;
    int iterations = 0;
    while (result.size() < count && iterations < m_ring.size()) {
        if (!seen.contains(it.value())) {
            result.append(it.value());
            seen.insert(it.value());
        }
        ++it;
        if (it == m_ring.constEnd()) it = m_ring.constBegin();
        ++iterations;
    }

    return result;
}

quint32 RingHash::hashKey(const QString& key) const
{
    QByteArray hash = QCryptographicHash::hash(
        key.toUtf8(), QCryptographicHash::Md5);
    /* 取前4字节作为32位哈希 */
    quint32 result = 0;
    for (int i = 0; i < 4; ++i) {
        result = (result << 8) | static_cast<unsigned char>(hash[i]);
    }
    return result;
}

quint32 RingHash::hashVirtual(const QString& node, int replica) const
{
    QString combined = node + QStringLiteral("#") + QString::number(replica);
    return hashKey(combined);
}

void RingHash::resetStatistics()
{
    m_stats = Stats{};
    m_stats.nodeCount = m_nodes.size();
    m_stats.virtualNodes = m_ring.size();
    m_timeSum = 0.0;
}
