/**
 * @file PerformanceOverlay.h
 * @brief 性能叠加显示控件
 *
 * 悬浮在界面上方的性能统计面板，显示 FPS 和内存占用。
 */

#ifndef PERFORMANCE_OVERLAY_H
#define PERFORMANCE_OVERLAY_H

#include <QWidget>
#include <QLabel>
#include <QVector>

class PerformanceMonitor;

/**
 * @brief FPS统计快照
 */
struct FpsStats {
    double minFps = 0.0;       ///< 最低FPS
    double maxFps = 0.0;       ///< 最高FPS
    double avgFps = 0.0;       ///< 平均FPS
    double currentFps = 0.0;   ///< 当前FPS
    int sampleCount = 0;       ///< 采样数
};

/**
 * @class PerformanceOverlay
 * @brief 性能叠加显示控件 —— 实时展示 FPS 与内存
 */
class PerformanceOverlay : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父控件 */
    explicit PerformanceOverlay(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~PerformanceOverlay() override;

    /**
     * @brief 设置性能监视器数据源
     * @param monitor 监视器实例
     */
    void setMonitor(PerformanceMonitor *monitor);

    /**
     * @brief 手动更新统计数据显示
     * @param fps        帧率
     * @param avgFrameMs 平均帧耗时（ms）
     * @param memBytes   内存占用（字节）
     */
    void updateStats(double fps, double avgFrameMs, qint64 memBytes);

    /**
     * @brief 获取FPS统计数据（min/max/avg/current）
     * @return FPS统计快照
     */
    FpsStats fpsStats() const;

    /**
     * @brief 设置FPS警告阈值
     * @param threshold 低于此值视为低帧率（默认30.0）
     */
    void setFpsWarningThreshold(double threshold);

    /**
     * @brief 查询当前是否低于FPS警告阈值
     * @return true=当前FPS低于阈值
     */
    bool isBelowFpsThreshold() const;

    /**
     * @brief 生成性能摘要文本
     * @return 格式化的性能摘要（FPS min/max/avg + 内存）
     */
    QString performanceSummary() const;

    /** @brief 重置统计数据 */
    void resetStats();

private slots:
    /**
     * @brief 统计数据更新回调
     * @param fps       帧率
     * @param avgFrameMs 平均帧耗时
     * @param memBytes  内存占用
     */
    void onStatsUpdated(double fps, double avgFrameMs, qint64 memBytes);

private:
    /** @brief 初始化 UI */
    void setupUI();

    PerformanceMonitor *m_monitor  = nullptr;  ///< 性能监视器
    QLabel             *m_fpsLabel = nullptr;  ///< FPS 标签
    QLabel             *m_memLabel = nullptr;  ///< 内存标签

    /** @brief FPS历史采样（滑动窗口，最近120个采样） */
    QVector<double> m_fpsHistory;
    static constexpr int kFpsHistorySize = 120; ///< FPS历史窗口大小

    double m_minFps = 999.0;     ///< 最低FPS
    double m_maxFps = 0.0;       ///< 最高FPS
    double m_currentFps = 0.0;   ///< 当前FPS
    qint64 m_lastMemBytes = 0;   ///< 最近一次内存值
    double m_fpsWarningThreshold = 30.0; ///< FPS警告阈值

    // ---- 统计计数器 ----
    quint64 m_totalUpdates = 0;               ///< 累计统计更新次数
    quint64 m_totalLowFpsWarnings = 0;        ///< 累计低FPS警告次数
    quint64 m_totalFpsSamples = 0;            ///< 累计FPS有效采样次数(fps>0)
    quint64 m_totalPipelineLatencySamples = 0;///< 累计管线延迟采样次数(avgFrameMs>0)
    quint64 m_totalThroughputSamples = 0;     ///< 累计吞吐量采样次数(memBytes更新)
public:
    /** @brief 获取累计统计更新次数 @return 更新次数 */
    quint64 totalUpdates() const { return m_totalUpdates; }
    /** @brief 获取累计低FPS警告次数 @return 警告次数 */
    quint64 totalLowFpsWarnings() const { return m_totalLowFpsWarnings; }
    /** @brief 获取累计FPS有效采样次数 @return FPS>0的采样次数 */
    quint64 totalFpsSamples() const { return m_totalFpsSamples; }
    /** @brief 获取累计管线延迟采样次数 @return avgFrameMs>0的采样次数 */
    quint64 totalPipelineLatencySamples() const { return m_totalPipelineLatencySamples; }
    /** @brief 获取累计吞吐量采样次数 @return 内存更新采样次数 */
    quint64 totalThroughputSamples() const { return m_totalThroughputSamples; }
    /** @brief 重置叠加层统计计数器(更新次数/低FPS警告/FPS采样/管线延迟采样/吞吐量采样) */
    void resetOverlayStatistics();
};

#endif // PERFORMANCE_OVERLAY_H
