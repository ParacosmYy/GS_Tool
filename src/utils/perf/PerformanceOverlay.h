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

class PerformanceMonitor;

/**
 * @class PerformanceOverlay
 * @brief 性能叠加显示控件 —— 实时展示 FPS 与内存
 */
class PerformanceOverlay : public QWidget
{
    Q_OBJECT

public:
    /// 构造函数
    explicit PerformanceOverlay(QWidget *parent = nullptr);

    /// 析构函数
    ~PerformanceOverlay() override;

    /**
     * @brief 设置性能监视器数据源
     * @param monitor 监视器实例
     */
    void setMonitor(PerformanceMonitor *monitor);

private slots:
    /**
     * @brief 统计数据更新回调
     * @param fps       帧率
     * @param avgFrameMs 平均帧耗时
     * @param memBytes  内存占用
     */
    void onStatsUpdated(double fps, double avgFrameMs, qint64 memBytes);

private:
    /// 初始化 UI
    void setupUI();

    PerformanceMonitor *m_monitor  = nullptr;  ///< 性能监视器
    QLabel             *m_fpsLabel = nullptr;  ///< FPS 标签
    QLabel             *m_memLabel = nullptr;  ///< 内存标签
};

#endif // PERFORMANCE_OVERLAY_H
