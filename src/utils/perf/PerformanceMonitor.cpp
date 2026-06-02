/**
 * @file PerformanceMonitor.cpp
 * @brief 性能监视器实现
 *
 * 实现帧计时（指数移动平均）、内存查询与延迟记录。
 */

#include "utils/perf/PerformanceMonitor.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

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

/** @brief 标记帧结束，使用指数移动平均更新 FPS */
void PerformanceMonitor::endFrame()
{
    m_frameCount++;

    const qint64 elapsed = m_frameTimer.elapsed(); ///< 本次帧耗时（ms）
    if (elapsed <= 0) {
        return;
    }

    // 指数移动平均：α=0.05，平滑帧耗时
    m_avgFrameMs = (m_avgFrameMs * 0.95) + (static_cast<double>(elapsed) * 0.05);

    // 由平均帧耗时计算 FPS
    m_fps = 1000.0 / m_avgFrameMs;

    // 每 30 帧发射一次统计信号，避免过于频繁
    if (m_frameCount % 30 == 0) {
        emit statsUpdated(m_fps, m_avgFrameMs, memoryUsageBytes());
    }
}

/**
 * @brief 获取当前 FPS
 * @return 每秒帧数
 */
double PerformanceMonitor::fps() const
{
    return m_fps;
}

/**
 * @brief 获取平均帧耗时
 * @return 毫秒
 */
double PerformanceMonitor::avgFrameTimeMs() const
{
    return m_avgFrameMs;
}

/**
 * @brief 获取当前进程内存使用量
 * @return 字节数（Windows 使用 PrivateUsage）
 *
 * Windows 平台通过 GetProcessMemoryInfo 获取 PrivateUsage；
 * 其他平台暂返回 0。
 */
qint64 PerformanceMonitor::memoryUsageBytes() const
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc;
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                             sizeof(pmc))) {
        return static_cast<qint64>(pmc.PrivateUsage);
    }
#endif
    return 0;
}

/**
 * @brief 记录模块延迟
 * @param tag          模块标签
 * @param microseconds 延迟微秒数
 *
 * 每个标签保留最近 100 条记录，超出后移除最旧的。
 */
void PerformanceMonitor::recordLatency(const QString &tag, quint64 microseconds)
{
    QList<quint64> &list = m_latencyMap[tag];
    list.append(microseconds);

    // 保留最近 100 条
    while (list.size() > 100) {
        list.removeFirst();
    }
}

/** @brief 便捷方法：标记完整一帧 */
void PerformanceMonitor::recordFrame()
{
    beginFrame();
    endFrame();
}

/**
 * @brief 获取总帧数
 * @return 自创建以来的累计帧数
 */
qint64 PerformanceMonitor::totalFrames() const
{
    return m_frameCount;
}
