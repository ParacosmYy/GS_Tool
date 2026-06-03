/**
 * @file TrafficMonitorWidget.cpp
 * @brief 流量监控显示控件实现 — 展示收发速率
 */

#include "serial/data/TrafficMonitorWidget.h"
#include "serial/data/TrafficMonitor.h"
#include <QHBoxLayout>

/**
 * @brief 格式化速率为人类可读字符串
 *
 * 根据速率大小自动选择合适单位 (B/s, KB/s, MB/s)。
 *
 * @param bytesPerSec 字节/秒速率
 * @return 格式化后的字符串
 */
static QString formatRate(double bytesPerSec)
{
    if (bytesPerSec < 1024.0) {
        return QObject::tr("%1 B/s").arg(qRound(bytesPerSec));
    } else if (bytesPerSec < 1024.0 * 1024.0) {
        return QObject::tr("%1 KB/s").arg(bytesPerSec / 1024.0, 0, 'f', 1);
    } else {
        return QObject::tr("%1 MB/s").arg(bytesPerSec / (1024.0 * 1024.0), 0, 'f', 2);
    }
}

/**
 * @brief 构造函数
 * @param parent 父控件
 */
TrafficMonitorWidget::TrafficMonitorWidget(QWidget* parent)
    : QWidget(parent)
    , m_monitor(nullptr)
    , m_rxRateLabel(nullptr)
    , m_txRateLabel(nullptr)
    , m_totalRateUpdates(0)
    , m_peakRxRate(0.0)
    , m_peakTxRate(0.0)
{
    setObjectName(QStringLiteral("trafficMonitorWidget"));
    setupUI();
}

/**
 * @brief 绑定流量监控数据源
 *
 * 连接 TrafficMonitor 的 rateUpdated 信号到本控件的 onRateUpdated 槽。
 *
 * @param monitor TrafficMonitor 实例
 */
void TrafficMonitorWidget::setMonitor(TrafficMonitor* monitor)
{
    if (m_monitor) {
        disconnect(m_monitor, &TrafficMonitor::rateUpdated,
                   this, &TrafficMonitorWidget::onRateUpdated);
    }

    m_monitor = monitor;

    if (m_monitor) {
        connect(m_monitor, &TrafficMonitor::rateUpdated,
                this, &TrafficMonitorWidget::onRateUpdated);
    }
}

/**
 * @brief 速率更新槽函数
 *
 * 格式化速率并更新 RX/TX 标签的显示文本。
 *
 * @param rxRate 接收速率（字节/秒）
 * @param txRate 发送速率（字节/秒）
 */
void TrafficMonitorWidget::onRateUpdated(double rxRate, double txRate)
{
    ++m_totalRateUpdates;
    if (rxRate > m_peakRxRate) m_peakRxRate = rxRate;
    if (txRate > m_peakTxRate) m_peakTxRate = txRate;
    m_rxRateLabel->setText(tr("RX: %1").arg(formatRate(rxRate)));
    m_txRateLabel->setText(tr("TX: %1").arg(formatRate(txRate)));
}

/**
 * @brief 初始化 UI 布局和控件
 *
 * 水平排列 RX/TX 速率标签。
 */
void TrafficMonitorWidget::setupUI()
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(12);

    m_rxRateLabel = new QLabel(tr("RX: 0 B/s"), this);
    m_rxRateLabel->setObjectName(QStringLiteral("rxRateLabel"));
    layout->addWidget(m_rxRateLabel);

    m_txRateLabel = new QLabel(tr("TX: 0 B/s"), this);
    m_txRateLabel->setObjectName(QStringLiteral("txRateLabel"));
    layout->addWidget(m_txRateLabel);

    layout->addStretch();
}

// ---- 统计接口 ----

/** @brief 获取累计速率更新次数 @return 更新总次数 */
quint64 TrafficMonitorWidget::totalRateUpdates() const
{
    return m_totalRateUpdates;
}

/** @brief 获取历史最高RX速率（字节/秒） @return RX峰值速率 */
double TrafficMonitorWidget::peakRxRate() const
{
    return m_peakRxRate;
}

/** @brief 获取历史最高TX速率（字节/秒） @return TX峰值速率 */
double TrafficMonitorWidget::peakTxRate() const
{
    return m_peakTxRate;
}

/** @brief 重置所有统计计数器归零 */
void TrafficMonitorWidget::resetTrafficWidgetStatistics()
{
    m_totalRateUpdates = 0;
    m_peakRxRate = 0.0;
    m_peakTxRate = 0.0;
}
