/**
 * @file PerformanceMonitor.cpp
 * @brief 性能监视器实现
 */

#include "utils/perf/PerformanceMonitor.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
PerformanceMonitor::PerformanceMonitor(QObject *parent)
    : QObject(parent)
{
    m_frameTimer.start();
}

/** @brief 析构函数 */
PerformanceMonitor::~PerformanceMonitor() = default;

/** @brief 标记帧开始 */
void PerformanceMonitor::beginFrame()
{
    m_frameTimer.restart();
}

/** @brief 标记帧结束 */
void PerformanceMonitor::endFrame()
{
    m_frameCount++;
    // TODO: 根据 elapsed 计算 m_fps 和 m_avgFrameMs
}

/**
 * @brief 获取当前 FPS
 * @return 0.0（暂未实现）
 */
double PerformanceMonitor::fps() const
{
    return m_fps;
}

/**
 * @brief 获取平均帧耗时
 * @return 0.0（暂未实现）
 */
double PerformanceMonitor::avgFrameTimeMs() const
{
    return m_avgFrameMs;
}

/**
 * @brief 获取内存使用量
 * @return 0（暂未实现）
 */
qint64 PerformanceMonitor::memoryUsageBytes() const
{
    // TODO: 平台相关内存查询
    return 0;
}

/**
 * @brief 记录模块延迟
 * @param tag     模块标签
 * @param usNanos 延迟微秒数
 */
void PerformanceMonitor::recordLatency(const QString &tag, quint64 usNanos)
{
    m_latencyMap[tag] = usNanos;
}
