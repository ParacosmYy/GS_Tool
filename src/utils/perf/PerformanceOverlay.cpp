/**
 * @file PerformanceOverlay.cpp
 * @brief 性能叠加显示控件实现
 */

#include "utils/perf/PerformanceOverlay.h"
#include "utils/perf/PerformanceMonitor.h"
#include <QVBoxLayout>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
PerformanceOverlay::PerformanceOverlay(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("PerformanceOverlay");
    setupUI();
}

/** @brief 析构函数 */
PerformanceOverlay::~PerformanceOverlay() = default;

/**
 * @brief 设置性能监视器数据源
 * @param monitor 监视器实例
 */
void PerformanceOverlay::setMonitor(PerformanceMonitor *monitor)
{
    m_monitor = monitor;
    // TODO: connect statsUpdated signal
}

/**
 * @brief 统计数据更新回调
 * @param fps       帧率
 * @param avgFrameMs 平均帧耗时
 * @param memBytes  内存占用
 */
void PerformanceOverlay::onStatsUpdated(double fps, double avgFrameMs, qint64 memBytes)
{
    Q_UNUSED(fps)
    Q_UNUSED(avgFrameMs)
    Q_UNUSED(memBytes)
    // TODO: 更新 m_fpsLabel 和 m_memLabel 文本
}

/** @brief 初始化 UI */
void PerformanceOverlay::setupUI()
{
    m_fpsLabel = new QLabel("FPS: --", this);
    m_memLabel = new QLabel("MEM: --", this);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_fpsLabel);
    layout->addWidget(m_memLabel);
    setLayout(layout);
}
