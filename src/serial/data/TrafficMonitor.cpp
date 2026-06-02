/**
 * @file TrafficMonitor.cpp
 * @brief 流量监控器实现 — 统计和计算串口收发数据速率
 */

#include "serial/data/TrafficMonitor.h"

/** @brief 速率历史最大保留点数 */
static constexpr int MAX_HISTORY_POINTS = 300;

/**
 * @brief 构造函数
 *
 * 创建速率计算定时器（1000ms 间隔），启动经过时间计时器。
 *
 * @param parent 父对象
 */
TrafficMonitor::TrafficMonitor(QObject* parent)
    : QObject(parent)
    , m_rxBytes(0)
    , m_txBytes(0)
    , m_calcTimer(new QTimer(this))
{
    m_calcTimer->setInterval(1000);
    m_calcTimer->setSingleShot(false);

    connect(m_calcTimer, &QTimer::timeout,
            this, &TrafficMonitor::calculateRates);

    m_elapsed.start();
    m_calcTimer->start();
}

/** @brief 析构函数 */
TrafficMonitor::~TrafficMonitor()
{
    // QTimer 由 QObject 父子树管理
}

/**
 * @brief 记录已发送字节数
 *
 * 累加到 m_txBytes，下次计算周期会反映在速率中。
 *
 * @param bytes 本次发送的字节数
 */
void TrafficMonitor::recordTxBytes(qint64 bytes)
{
    m_txBytes += bytes;
}

/**
 * @brief 记录已接收字节数
 * @param bytes 本次接收的字节数
 */
void TrafficMonitor::recordRxBytes(qint64 bytes)
{
    m_rxBytes += bytes;
}

/**
 * @brief 获取当前 RX 速率
 * @return 接收速率（字节/秒）
 */
double TrafficMonitor::rxRate() const
{
    if (m_rxHistory.isEmpty()) {
        return 0.0;
    }
    return m_rxHistory.last().y();
}

/**
 * @brief 获取当前 TX 速率
 * @return 发送速率（字节/秒）
 */
double TrafficMonitor::txRate() const
{
    if (m_txHistory.isEmpty()) {
        return 0.0;
    }
    return m_txHistory.last().y();
}

/**
 * @brief 获取 RX 速率历史数据
 * @return 数据点向量（x=秒, y=字节/秒）
 */
QVector<QPointF> TrafficMonitor::rxRateHistory() const
{
    return m_rxHistory;
}

/**
 * @brief 获取 TX 速率历史数据
 * @return 数据点向量（x=秒, y=字节/秒）
 */
QVector<QPointF> TrafficMonitor::txRateHistory() const
{
    return m_txHistory;
}

/**
 * @brief 重置所有统计数据
 *
 * 清零累计字节数和速率历史，重启计时器。
 */
void TrafficMonitor::reset()
{
    m_rxBytes = 0;
    m_txBytes = 0;
    m_rxHistory.clear();
    m_txHistory.clear();
    m_elapsed.restart();
}

/**
 * @brief 定时器超时处理 — 计算瞬时速率
 *
 * 根据累计字节数和经过时间计算平均速率，
 * 追加到历史队列（保留最近 300 个点），
 * 重置计数器并发出 rateUpdated 信号。
 */
void TrafficMonitor::calculateRates()
{
    double elapsed = m_elapsed.elapsed() / 1000.0;  // 转为秒
    if (elapsed <= 0.0) {
        return;
    }

    // 计算速率
    double rx = m_rxBytes / elapsed;
    double tx = m_txBytes / elapsed;

    // 计算总经过时间（秒），用作历史 X 轴
    double timestamp = m_elapsed.elapsed() / 1000.0;

    // 追加到历史队列
    m_rxHistory.append(QPointF(timestamp, rx));
    m_txHistory.append(QPointF(timestamp, tx));

    // 裁剪历史到最大长度
    while (m_rxHistory.size() > MAX_HISTORY_POINTS) {
        m_rxHistory.removeFirst();
    }
    while (m_txHistory.size() > MAX_HISTORY_POINTS) {
        m_txHistory.removeFirst();
    }

    // 重置字节数和计时器
    m_rxBytes = 0;
    m_txBytes = 0;
    m_elapsed.restart();

    emit rateUpdated(rx, tx);
}
