/**
 * @file TimedSender.cpp
 * @brief 定时发送器实现
 *
 * 实现:
 *   - 构造: 连接定时器超时信号到 onTimeout 槽
 *   - 线程安全: QMutex 保护 m_isRunning / m_interval / m_queue / m_queueIndex 的并发读写
 *   - 主线程调度: onTimeout 中使用 QMetaObject::invokeMethod
 *     确保实际发送动作在主线程执行
 *   - 连接检查: 发送前验证队列非空，空则自动停止定时器
 */

#include "serial/commands/TimedSender.h"

/**
 * @brief 构造定时发送器
 * @param parent 父对象
 *
 * 初始化定时器并连接超时信号。初始状态为停止，间隔 1000ms。
 */
TimedSender::TimedSender(QObject* parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &TimedSender::onTimeout);
}

/**
 * @brief 设置发送间隔（毫秒）
 * @param ms 间隔时间，必须 > 0
 *
 * 线程安全：加锁保护 m_interval。
 * 如果定时器正在运行，立即应用新间隔值。
 */
void TimedSender::setInterval(int ms)
{
    QMutexLocker locker(&m_mutex);
    m_interval = (ms > 0) ? ms : 1000;

    // 如果定时器正在运行，立即更新间隔
    if (m_timer.isActive()) {
        m_timer.setInterval(m_interval);
    }
}

/**
 * @brief 获取当前发送间隔（毫秒）
 * @return 间隔时间
 *
 * 线程安全：加锁读取 m_interval。
 */
int TimedSender::interval() const
{
    QMutexLocker locker(&m_mutex);
    return m_interval;
}

/**
 * @brief 设置要循环发送的单组数据
 * @param data 待发送的数据
 *
 * 重置队列索引为 0。
 */
void TimedSender::setData(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    m_queue = {data};
    m_queueIndex = 0;
}

/**
 * @brief 设置多组数据队列（按顺序循环发送）
 * @param queue 数据队列列表
 *
 * 重置队列索引为 0。
 */
void TimedSender::setDataQueue(const QList<QByteArray>& queue)
{
    QMutexLocker locker(&m_mutex);
    m_queue = queue;
    m_queueIndex = 0;
}

/**
 * @brief 启动定时发送
 *
 * 线程安全：可在定时器回调中安全调用。
 * 如果队列为空则不启动，避免无意义的定时触发。
 */
void TimedSender::start()
{
    QMutexLocker locker(&m_mutex);
    if (m_queue.isEmpty()) {
        return;
    }

    m_queueIndex = 0;
    m_sendCount = 0;
    m_totalBytesSent = 0;
    ++m_scheduleCount;
    m_isRunning = true;
    m_timer.start(m_interval);
}

/**
 * @brief 停止定时发送
 *
 * 线程安全：可在定时器回调中安全调用。
 */
void TimedSender::stop()
{
    QMutexLocker locker(&m_mutex);
    m_isRunning = false;
    m_timer.stop();
}

/**
 * @brief 查询定时发送是否正在运行
 * @return true=正在运行，false=已停止
 *
 * 线程安全：加锁读取 m_isRunning。
 */
bool TimedSender::isRunning() const
{
    QMutexLocker locker(&m_mutex);
    return m_isRunning;
}

/**
 * @brief 获取已发送次数
 * @return 定时发送已触发的次数
 */
int TimedSender::sendCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_sendCount;
}

/**
 * @brief 定时器超时处理槽
 *
 * 使用 QMetaObject::invokeMethod 将 doSend() 调度到主线程执行，
 * 确保 sendData 信号在主线程中发射，下游可安全操作串口和 UI。
 * 如果当前不在主线程，使用 QueuedConnection 异步调度。
 */
void TimedSender::onTimeout()
{
    // 使用 QMetaObject::invokeMethod 确保在主线程执行发送
    // 如果已经在主线程则同步执行，否则异步调度
    QMetaObject::invokeMethod(this, &TimedSender::doSend,
                              Qt::QueuedConnection);
}

/**
 * @brief 执行实际的发送动作（在主线程中调用）
 *
 * 1. 检查队列是否为空，空则停止定时器
 * 2. 检查连接有效性（通过 m_isRunning 标志判断）
 * 3. 从队列中取出当前数据并发射 sendData 信号
 * 4. 推进队列索引到下一个位置（循环）
 */
void TimedSender::doSend()
{
    QByteArray dataToSend;
    {
        QMutexLocker locker(&m_mutex);

        // 检查队列有效性
        if (m_queue.isEmpty()) {
            m_isRunning = false;
            m_timer.stop();
            return;
        }

        // 检查运行状态
        if (!m_isRunning) {
            return;
        }

        // 在锁内拷贝数据，避免 emit 时持锁导致信号回调死锁
        dataToSend = m_queue[m_queueIndex];
        m_queueIndex = (m_queueIndex + 1) % m_queue.size();
        ++m_sendCount;
        m_totalBytesSent += static_cast<quint64>(dataToSend.size());
    }

    // 释放锁后发射信号，避免下游回调死锁
    emit sendData(dataToSend);
}

/**
 * @brief 获取累计发送的总字节数
 * @return 总字节数
 *
 * 线程安全：加锁读取 m_totalBytesSent。
 */
quint64 TimedSender::totalBytesSent() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalBytesSent;
}

/**
 * @brief 获取定时发送调度次数（每次 start() 调用 +1）
 * @return 调度次数
 *
 * 线程安全：加锁读取 m_scheduleCount。
 */
quint64 TimedSender::scheduleCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_scheduleCount;
}

/**
 * @brief 重置所有统计计数器
 *
 * 将 sendCount、totalBytesSent、scheduleCount 全部归零。
 * 线程安全：内部加锁保护。
 */
void TimedSender::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    m_sendCount = 0;
    m_totalBytesSent = 0;
    m_scheduleCount = 0;
}
