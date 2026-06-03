/**
 * @file TrafficMonitorWidget.h
 * @brief 流量监控显示控件 — 展示收发速率和速率曲线
 *
 * 显示当前 RX/TX 速率数值，可选显示速率历史曲线图。
 * 通过 setMonitor() 绑定 TrafficMonitor 数据源。
 *
 * 协作关系:
 *   - TrafficMonitor: 提供速率数据和历史记录
 */
#ifndef TRAFFICMONITORWIDGET_H
#define TRAFFICMONITORWIDGET_H

#include <QWidget>
#include <QLabel>

class TrafficMonitor;

/**
 * @brief 流量监控显示控件
 *
 * 上方显示 RX/TX 速率数值标签，下方预留图表区域。
 */
class TrafficMonitorWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TrafficMonitorWidget(QWidget* parent = nullptr);

    /**
     * @brief 绑定流量监控数据源
     * @param monitor TrafficMonitor 实例（非空）
     */
    void setMonitor(TrafficMonitor* monitor);

    // ---- 统计接口 ----

    /** @brief 获取累计速率更新次数 */
    quint64 totalRateUpdates() const;

    /** @brief 获取历史最高RX速率（字节/秒） */
    double peakRxRate() const;

    /** @brief 获取历史最高TX速率（字节/秒） */
    double peakTxRate() const;

    /** @brief 重置所有统计计数器归零 */
    void resetTrafficWidgetStatistics();

private slots:
    /**
     * @brief 速率更新槽函数
     * @param rxRate 接收速率（字节/秒）
     * @param txRate 发送速率（字节/秒）
     */
    void onRateUpdated(double rxRate, double txRate);

private:
    /** @brief 初始化 UI 布局和控件 */
    void setupUI();

    TrafficMonitor* m_monitor = nullptr;   ///< 数据源（不拥有）
    QLabel* m_rxRateLabel;                 ///< RX 速率显示标签
    QLabel* m_txRateLabel;                 ///< TX 速率显示标签

    // ---- 统计计数器 ----
    quint64 m_totalRateUpdates = 0;        ///< 累计速率更新次数
    double m_peakRxRate = 0.0;             ///< 历史最高RX速率（字节/秒）
    double m_peakTxRate = 0.0;             ///< 历史最高TX速率（字节/秒）
};

#endif // TRAFFICMONITORWIDGET_H
