#include "TimedSender.h"

TimedSender::TimedSender(QObject* parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &TimedSender::onTimeout);
}

void TimedSender::setInterval(int ms)
{
    m_timer.setInterval(ms);
}

int TimedSender::interval() const
{
    return m_timer.interval();
}

void TimedSender::setData(const QByteArray& data)
{
    m_queue = {data};
    m_queueIndex = 0;
}

void TimedSender::setDataQueue(const QList<QByteArray>& queue)
{
    m_queue = queue;
    m_queueIndex = 0;
}

void TimedSender::start()
{
    if (m_queue.isEmpty()) return;
    m_queueIndex = 0;
    m_timer.start();
}

void TimedSender::stop()
{
    m_timer.stop();
}

bool TimedSender::isRunning() const
{
    return m_timer.isActive();
}

void TimedSender::onTimeout()
{
    if (m_queue.isEmpty()) {
        stop();
        return;
    }

    // 发送当前队列位置的数据
    emit sendData(m_queue[m_queueIndex]);

    // 移到下一个位置，循环
    m_queueIndex = (m_queueIndex + 1) % m_queue.size();
}
