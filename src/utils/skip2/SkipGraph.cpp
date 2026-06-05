/**
 * @file SkipGraph.cpp
 * @brief Skip Graph跳表图数据结构实现
 */

#include "SkipGraph.h"
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <algorithm>

SkipGraph::SkipGraph(int maxLevel, QObject* parent)
    : QObject(parent)
    , m_maxLevel(qMax(4, maxLevel))
    , m_level(0)
    , m_count(0)
    , m_timeSum(0.0)
{
    m_header = new Node{0.0, QVariant{}, QVector<Node*>(m_maxLevel, nullptr),
                        QVector<Node*>(m_maxLevel, nullptr)};
}

SkipGraph::~SkipGraph()
{
    Node* cur = m_header->forward[0];
    while (cur) {
        Node* next = cur->forward[0];
        delete cur;
        cur = next;
    }
    delete m_header;
}

int SkipGraph::randomLevel() const
{
    int lvl = 1;
    while (QRandomGenerator::global()->bounded(2) == 0 && lvl < m_maxLevel)
        lvl++;
    return lvl;
}

SkipGraph::Node* SkipGraph::findNode(double key) const
{
    Node* cur = m_header;
    for (int i = m_level - 1; i >= 0; --i) {
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
    }
    cur = cur->forward[0];
    return (cur && cur->key == key) ? cur : nullptr;
}

void SkipGraph::insert(double key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Node*> update(m_maxLevel, nullptr);
    Node* cur = m_header;

    for (int i = m_level - 1; i >= 0; --i) {
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
        update[i] = cur;
    }

    cur = cur->forward[0];
    if (cur && cur->key == key) {
        cur->value = value;
        return;
    }

    int newLevel = randomLevel();
    if (newLevel > m_level) {
        for (int i = m_level; i < newLevel; ++i)
            update[i] = m_header;
        m_level = newLevel;
    }

    Node* newNode = new Node{key, value, QVector<Node*>(newLevel, nullptr),
                             QVector<Node*>(newLevel, nullptr)};

    for (int i = 0; i < newLevel; ++i) {
        newNode->forward[i] = update[i]->forward[i];
        if (update[i]->forward[i])
            update[i]->forward[i]->backward[i] = newNode;
        update[i]->forward[i] = newNode;
        newNode->backward[i] = update[i];
    }

    m_count++;
    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalSearches);

    emit insertCompleted(key);
}

bool SkipGraph::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Node*> update(m_maxLevel, nullptr);
    Node* cur = m_header;

    for (int i = m_level - 1; i >= 0; --i) {
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
        update[i] = cur;
    }

    cur = cur->forward[0];
    if (!cur || cur->key != key) {
        emit removeCompleted(key, false);
        return false;
    }

    for (int i = 0; i < m_level; ++i) {
        if (update[i]->forward[i] != cur) break;
        update[i]->forward[i] = cur->forward[i];
        if (cur->forward[i])
            cur->forward[i]->backward[i] = update[i];
    }

    delete cur;
    m_count--;

    while (m_level > 1 && !m_header->forward[m_level - 1])
        m_level--;

    m_stats.totalRemoves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalSearches);

    emit removeCompleted(key, true);
    return true;
}

QVariant SkipGraph::search(double key) const
{
    QElapsedTimer timer;
    timer.start();

    Node* node = findNode(key);
    m_stats.totalSearches++;
    if (node) m_stats.totalHits++;

    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalSearches;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return node ? node->value : QVariant();
}

QVector<QPair<double, QVariant>> SkipGraph::rangeQuery(double lo, double hi) const
{
    QVector<QPair<double, QVariant>> result;
    Node* cur = m_header;

    for (int i = m_level - 1; i >= 0; --i) {
        while (cur->forward[i] && cur->forward[i]->key < lo)
            cur = cur->forward[i];
    }

    cur = cur->forward[0];
    while (cur && cur->key <= hi) {
        result.append({cur->key, cur->value});
        cur = cur->forward[0];
    }
    return result;
}

QVector<double> SkipGraph::keys() const
{
    QVector<double> k;
    Node* cur = m_header->forward[0];
    while (cur) {
        k.append(cur->key);
        cur = cur->forward[0];
    }
    return k;
}

int SkipGraph::size() const { return m_count; }

SkipGraph::Stats SkipGraph::stats() const { return m_stats; }

void SkipGraph::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
