/**
 * @file TrafficMonitor.cpp
 * @brief 流量监控器实现 — 骨架文件
 */

#include "serial/data/TrafficMonitor.h"

/**
 * @brief 构造函数
 *
 * 初始化计时器和定时器，但不自动启动计算。
 *
 * @param parent 父对象
 */
TrafficMonitor::TrafficMonitor(QObject* parent)
    : QObject(parent)
    , m_rxBytes(0)
    , m_txBytes(0)
    , m_calcTimer(nullptr)
{
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
    // TODO: 基于最近周期的字节数差值计算速率
    return 0.0;
}

/**
 * @brief 获取当前 TX 速率
 * @return 发送速率（字节/秒）
 */
double TrafficMonitor::txRate() const
{
    // TODO: 基于最近周期的字节数差值计算速率
    return 0.0;
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
