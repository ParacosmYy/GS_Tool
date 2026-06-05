/**
 * @file RoundRobinScheduler.cpp
 * @brief 轮询调度器实现
 */

#include "RoundRobinScheduler.h"
#include <QElapsedTimer>

RoundRobinScheduler::RoundRobinScheduler(QObject* parent)
    : QObject(parent)
    , m_currentIndex(0)
    , m_timeSum(0.0)
{
}

void RoundRobinScheduler::addTask(const QString& taskId, int weight)
{
    for (const auto& t : m_tasks) {
        if (t.id == taskId) return;
    }

    m_tasks.append({taskId, qMax(1, weight), 0, 0});
    m_stats.totalTasksAdded++;
}

bool RoundRobinScheduler::removeTask(const QString& taskId)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == taskId) {
            m_tasks.removeAt(i);
            if (m_currentIndex >= m_tasks.size())
                m_currentIndex = 0;
            m_stats.totalTasksRemoved++;
            return true;
        }
    }
    return false;
}

QString RoundRobinScheduler::next()
{
    QElapsedTimer timer;
    timer.start();

    if (m_tasks.isEmpty()) {
        m_timeSum += timer.elapsed();
        return QString();
    }

    /* 加权轮询: counter递减, 当counter=0时选择并重置 */
    while (true) {
        if (m_currentIndex >= m_tasks.size())
            m_currentIndex = 0;

        auto& task = m_tasks[m_currentIndex];

        if (task.counter > 0) {
            task.counter--;
            m_currentIndex++;
            continue;
        }

        task.counter = task.weight - 1;
        task.timesScheduled++;

        m_stats.totalScheduled++;
        m_timeSum += timer.elapsed();
        int total = m_stats.totalScheduled + m_stats.totalTasksAdded;
        if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

        m_currentIndex++;
        emit taskScheduled(task.id, task.timesScheduled);
        return task.id;
    }
}

QVector<QString> RoundRobinScheduler::schedule(int count)
{
    QVector<QString> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        QString task = next();
        if (task.isEmpty()) break;
        result.append(task);
    }
    return result;
}

int RoundRobinScheduler::taskCount() const { return m_tasks.size(); }

void RoundRobinScheduler::setWeight(const QString& taskId, int weight)
{
    for (auto& t : m_tasks) {
        if (t.id == taskId) {
            t.weight = qMax(1, weight);
            break;
        }
    }
}

RoundRobinScheduler::Stats RoundRobinScheduler::stats() const { return m_stats; }

void RoundRobinScheduler::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
