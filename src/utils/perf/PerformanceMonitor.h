/**
 * @file PerformanceMonitor.h
 * @brief 性能监视器
 *
 * 跟踪帧率、帧耗时和内存使用，记录各模块延迟。
 */

#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <QObject>
#include <QElapsedTimer>
#include <QMap>

/**
 * @class PerformanceMonitor
 * @brief 性能监视器 —— FPS、帧耗时、内存占用与延迟记录
 */
class PerformanceMonitor : public QObject
{
    Q_OBJECT

public:
    /// 构造函数
    explicit PerformanceMonitor(QObject *parent = nullptr);

    /// 析构函数
    ~PerformanceMonitor() override;

    /// 标记帧开始
    void beginFrame();

    /// 标记帧结束，更新统计数据
    void endFrame();

    /**
     * @brief 获取当前 FPS
     * @return 每秒帧数
     */
    double fps() const;

    /**
     * @brief 获取平均帧耗时
     * @return 毫秒
     */
    double avgFrameTimeMs() const;

    /**
     * @brief 获取当前进程内存使用
     * @return 字节数
     */
    qint64 memoryUsageBytes() const;

    /**
     * @brief 记录某模块的延迟
     * @param tag   模块标签
     * @param usNanos 延迟（微秒）
     */
    void recordLatency(const QString &tag, quint64 usNanos);

signals:
    /// 统计数据更新
    void statsUpdated(double fps, double avgFrameMs, qint64 memBytes);

private:
    QElapsedTimer m_frameTimer;       ///< 帧计时器
    double        m_fps       = 0.0;  ///< 当前 FPS
    double        m_avgFrameMs = 0.0; ///< 平均帧耗时
    int           m_frameCount = 0;   ///< 帧计数
    QMap<QString, quint64> m_latencyMap; ///< 模块延迟记录
};

#endif // PERFORMANCE_MONITOR_H
