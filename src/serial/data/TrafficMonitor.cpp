/**
 * @file TrafficMonitor.cpp
 * @brief 流量监控器实现 — 统计和计算串口收发数据速率
 */

#include "serial/data/TrafficMonitor.h"

/** @brief 速率历史最大保留点数 */
static constexpr int MAX_HISTORY_POINTS = 300;

/** @brief 构造函数，创建1000ms间隔的速率计算定时器并启动经过时间计时器 @param parent 父对象 */
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

/** @brief 析构函数，QTimer由QObject父子树管理自动销毁 */
TrafficMonitor::~TrafficMonitor()
{
    // QTimer 由 QObject 父子树管理
}

/** @brief 记录已发送字节数，累加到m_txBytes和m_totalTxBytes，下次计算周期反映在速率中 @param bytes 本次发送的字节数 */
void TrafficMonitor::recordTxBytes(qint64 bytes)
{
    m_txBytes += bytes;
    m_totalTxBytes += bytes;
    ++m_totalBytesOut;
    ++m_totalPacketsOut;
}

/** @brief 记录已接收字节数，累加到m_rxBytes和m_totalRxBytes @param bytes 本次接收的字节数 */
void TrafficMonitor::recordRxBytes(qint64 bytes)
{
    m_rxBytes += bytes;
    m_totalRxBytes += bytes;
    ++m_totalBytesIn;
    ++m_totalPacketsIn;
}

/** @brief 获取当前RX速率 @return 接收速率(字节/秒)，无历史数据时返回0.0 */
double TrafficMonitor::rxRate() const
{
    if (m_rxHistory.isEmpty()) {
        return 0.0;
    }
    return m_rxHistory.last().y();
}

/** @brief 获取当前TX速率 @return 发送速率(字节/秒)，无历史数据时返回0.0 */
double TrafficMonitor::txRate() const
{
    if (m_txHistory.isEmpty()) {
        return 0.0;
    }
    return m_txHistory.last().y();
}

/** @brief 获取RX速率历史数据 @return 数据点向量(x=秒, y=字节/秒) */
QVector<QPointF> TrafficMonitor::rxRateHistory() const
{
    return m_rxHistory;
}

/** @brief 获取TX速率历史数据 @return 数据点向量(x=秒, y=字节/秒) */
QVector<QPointF> TrafficMonitor::txRateHistory() const
{
    return m_txHistory;
}

/** @brief 重置所有统计数据(累计字节/速率历史/峰值/计时器)，清零后重新开始监控 */
void TrafficMonitor::reset()
{
    m_rxBytes = 0;
    m_txBytes = 0;
    m_totalRxBytes = 0;
    m_totalTxBytes = 0;
    m_peakRxRate = 0.0;
    m_peakTxRate = 0.0;
    m_rxHistory.clear();
    m_txHistory.clear();
    m_elapsed.restart();
}

/** @brief 获取会话累计RX总字节 @return 接收字节总数 */
qint64 TrafficMonitor::totalRxBytes() const
{
    return m_totalRxBytes;
}

/** @brief 获取会话累计TX总字节 @return 发送字节总数 */
qint64 TrafficMonitor::totalTxBytes() const
{
    return m_totalTxBytes;
}

/** @brief 获取历史最高RX速率 @return 峰值接收速率(字节/秒) */
double TrafficMonitor::peakRxRate() const
{
    return m_peakRxRate;
}

/** @brief 获取历史最高TX速率 @return 峰值发送速率(字节/秒) */
double TrafficMonitor::peakTxRate() const
{
    return m_peakTxRate;
}

/** @brief 获取速率历史点数 @return 当前历史队列中的数据点数量 */
int TrafficMonitor::historySize() const
{
    return m_rxHistory.size();
}

/** @brief 定时器超时处理，根据累计字节数和经过时间计算瞬时速率，追加到历史队列(保留最近300个点)，更新峰值并发射rateUpdated信号 */
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
    ++m_totalRateSamples;

    // 更新峰值
    if (rx > m_peakRxRate) { m_peakRxRate = rx; ++m_totalPeakRateExceededEvents; }
    if (tx > m_peakTxRate) { m_peakTxRate = tx; ++m_totalPeakRateExceededEvents; }

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

    // 统计计数器递增
    ++m_totalSamples;

    emit rateUpdated(rx, tx);
}

// 统计getter/resetTrafficStatistics已移至 TrafficMonitorStats.cpp
