/**
 * @file ConcurrentSkipList.cpp
 * @brief 并发跳表实现
 */

#include "utils/skiplist2/ConcurrentSkipList.h"

#include <QElapsedTimer>
#include <QMutexLocker>

ConcurrentSkipList::ConcurrentSkipList(int maxLevel, QObject* parent)
    : QObject(parent), m_maxLevel(maxLevel), m_currentLevel(0),
      m_count(0), m_timeSum(0.0)
{
    m_head = new Node(std::numeric_limits<double>::lowest(), m_maxLevel);
}

ConcurrentSkipList::~ConcurrentSkipList()
{
    Node* current = m_head;
    while (current) {
        Node* next = current->forward[0];
        delete current;
        current = next;
    }
}

bool ConcurrentSkipList::insert(double value)
{
    QElapsedTimer timer;
    timer.start();

    QMutexLocker locker(&m_globalMutex);

    QVector<Node*> update(m_maxLevel + 1, nullptr);
    Node* current = m_head;

    /* 从最高层向下查找插入位置 */
    for (int i = m_currentLevel; i >= 0; --i) {
        while (current->forward[i] && current->forward[i]->value < value) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    /* 检查是否已存在 */
    current = current->forward[0];
    if (current && current->value == value) {
        return false;  // 已存在
    }

    /* 生成随机层数 */
    int newLevel = randomLevel();
    if (newLevel > m_currentLevel) {
        for (int i = m_currentLevel + 1; i <= newLevel; ++i) {
            update[i] = m_head;
        }
        m_currentLevel = newLevel;
    }

    /* 创建新节点并插入 */
    Node* newNode = new Node(value, newLevel);
    for (int i = 0; i <= newLevel; ++i) {
        newNode->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = newNode;
    }

    ++m_count;
    m_stats.totalInsertions++;
    m_stats.elementCount = m_count;
    m_stats.maxHeight = m_currentLevel;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInsertions + m_stats.totalDeletions +
             m_stats.totalLookups, 1ULL);

    emit valueInserted(value);
    return true;
}

bool ConcurrentSkipList::remove(double value)
{
    QElapsedTimer timer;
    timer.start();

    QMutexLocker locker(&m_globalMutex);

    QVector<Node*> update(m_maxLevel + 1, nullptr);
    Node* current = m_head;

    for (int i = m_currentLevel; i >= 0; --i) {
        while (current->forward[i] && current->forward[i]->value < value) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (!current || current->value != value) {
        return false;  // 不存在
    }

    /* 从所有层移除 */
    for (int i = 0; i <= m_currentLevel; ++i) {
        if (update[i]->forward[i] != current) break;
        update[i]->forward[i] = current->forward[i];
    }

    delete current;

    /* 缩减层数 */
    while (m_currentLevel > 0 && m_head->forward[m_currentLevel] == nullptr) {
        --m_currentLevel;
    }

    --m_count;
    m_stats.totalDeletions++;
    m_stats.elementCount = m_count;
    m_stats.maxHeight = m_currentLevel;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInsertions + m_stats.totalDeletions +
             m_stats.totalLookups, 1ULL);

    emit valueRemoved(value);
    return true;
}

bool ConcurrentSkipList::contains(double value)
{
    QElapsedTimer timer;
    timer.start();

    QMutexLocker locker(&m_globalMutex);

    Node* current = m_head;
    for (int i = m_currentLevel; i >= 0; --i) {
        while (current->forward[i] && current->forward[i]->value < value) {
            current = current->forward[i];
        }
    }

    current = current->forward[0];
    bool found = (current && current->value == value);

    m_stats.totalLookups++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInsertions + m_stats.totalDeletions +
             m_stats.totalLookups, 1ULL);

    return found;
}

QVector<double> ConcurrentSkipList::rangeQuery(double minVal, double maxVal) const
{
    QMutexLocker locker(&m_globalMutex);
    QVector<double> result;
    Node* current = m_head;

    for (int i = m_currentLevel; i >= 0; --i) {
        while (current->forward[i] && current->forward[i]->value < minVal) {
            current = current->forward[i];
        }
    }

    current = current->forward[0];
    while (current && current->value <= maxVal) {
        result.append(current->value);
        current = current->forward[0];
    }

    return result;
}

QVector<double> ConcurrentSkipList::toSortedList() const
{
    QMutexLocker locker(&m_globalMutex);
    QVector<double> result;
    Node* current = m_head->forward[0];
    while (current) {
        result.append(current->value);
        current = current->forward[0];
    }
    return result;
}

int ConcurrentSkipList::randomLevel() const
{
    int level = 0;
    while (QRandomGenerator::global()->bounded(2) == 0 && level < m_maxLevel) {
        ++level;
    }
    return level;
}

void ConcurrentSkipList::resetStatistics()
{
    m_stats = Stats{};
    m_stats.elementCount = m_count;
    m_stats.maxHeight = m_currentLevel;
    m_timeSum = 0.0;
}
