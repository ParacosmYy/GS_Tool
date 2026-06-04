/**
 * @file TimedSenderStats.cpp
 * @brief 定时发送器 — 统计计数器查询与重置实现
 *
 * 从 TimedSender.cpp 拆分而来，包含发送字节/调度次数/
 * 定时发送累计等统计 getter 和 resetStatistics 方法。
 */

#include "serial/commands/TimedSender.h"

/** @brief 获取累计发送的总字节数，线程安全 @return 总字节数 */
quint64 TimedSender::totalBytesSent() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalBytesSent;
}

/** @brief 获取定时发送调度次数（每次start()调用+1），线程安全 @return 调度次数 */
quint64 TimedSender::scheduleCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_scheduleCount;
}

/** @brief 获取定时器触发发送的总次数，线程安全 @return 触发总次数 */
quint64 TimedSender::totalTimedSends() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalTimedSends;
}

/** @brief 获取定时发送累计字节数，线程安全 @return 定时发送写入的总字节数 */
quint64 TimedSender::totalTimedBytesSent() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalTimedBytesSent;
}

/** @brief 获取定时发送累计错误次数，线程安全 @return 发送失败/队列为空等错误次数 */
quint64 TimedSender::totalTimedErrors() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalTimedErrors;
}

/** @brief 重置所有统计计数器(sendCount/totalBytesSent/scheduleCount/totalTimedSends/totalTimedBytesSent/totalTimedErrors归零)，线程安全 */
void TimedSender::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    m_sendCount = 0;
    m_totalBytesSent = 0;
    m_scheduleCount = 0;
    m_totalTimedSends = 0;
    m_totalTimedBytesSent = 0;
    m_totalTimedErrors = 0;
}
