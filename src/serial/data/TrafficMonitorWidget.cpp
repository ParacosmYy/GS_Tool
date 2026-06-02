/**
 * @file TrafficMonitorWidget.cpp
 * @brief 流量监控显示控件实现 — 骨架文件
 */

#include "serial/data/TrafficMonitorWidget.h"
#include "serial/data/TrafficMonitor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
TrafficMonitorWidget::TrafficMonitorWidget(QWidget* parent)
    : QWidget(parent)
    , m_monitor(nullptr)
    , m_rxRateLabel(nullptr)
    , m_txRateLabel(nullptr)
{
    setObjectName(QStringLiteral("TrafficMonitorWidget"));
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
    Q_UNUSED(monitor)
    // TODO: 保存指针，连接 rateUpdated 信号
}

/**
 * @brief 速率更新槽函数
 *
 * 更新 RX/TX 速率标签的显示文本。
 *
 * @param rxRate 接收速率（字节/秒）
 * @param txRate 发送速率（字节/秒）
 */
void TrafficMonitorWidget::onRateUpdated(double rxRate, double txRate)
{
    Q_UNUSED(rxRate)
    Q_UNUSED(txRate)
    // TODO: 格式化速率并更新标签文本
}

/**
 * @brief 初始化 UI 布局和控件
 *
 * 水平排列 RX/TX 速率标签，下方预留图表区域。
 */
void TrafficMonitorWidget::setupUI()
{
    // TODO: 创建并布局所有 UI 控件
    // m_rxRateLabel = new QLabel(tr("RX: 0 B/s"), this);
    // m_txRateLabel = new QLabel(tr("TX: 0 B/s"), this);
}
