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
#include <QList>
#include <QStringList>

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
     * @param microseconds 延迟（微秒）
     */
    void recordLatency(const QString &tag, quint64 microseconds);

    /// 便捷方法：标记一帧（beginFrame + endFrame）
    void recordFrame();

    /**
     * @brief 获取总帧数
     * @return 自创建以来的总帧计数
     */
    qint64 totalFrames() const;

    /**
     * @brief 获取指定模块的平均延迟
     * @param tag 模块标签
     * @return 平均延迟（微秒），无数据返回 0
     */
    double avgLatency(const QString& tag) const;

    /**
     * @brief 获取指定模块的最大延迟
     * @param tag 模块标签
     * @return 最大延迟（微秒），无数据返回 0
     */
    quint64 maxLatency(const QString& tag) const;

    /**
     * @brief 获取指定模块的最小延迟
     * @param tag 模块标签
     * @return 最小延迟（微秒），无数据返回 0
     */
    quint64 minLatency(const QString& tag) const;

    /**
     * @brief 获取所有已记录延迟的模块标签
     * @return 标签列表
     */
    QStringList latencyTags() const;

    /**
     * @brief 清除指定模块的延迟记录
     * @param tag 模块标签
     */
    void clearLatency(const QString& tag);

    /**
     * @brief 清除所有延迟记录
     */
    void clearAllLatency();

signals:
    /// 统计数据更新
    void statsUpdated(double fps, double avgFrameMs, qint64 memBytes);

private:
    QElapsedTimer m_frameTimer;                    ///< 帧计时器
    double        m_fps       = 0.0;               ///< 当前 FPS
    double        m_avgFrameMs = 0.0;              ///< 平均帧耗时（EMA）
    qint64        m_frameCount = 0;                ///< 帧计数
    QMap<QString, QList<quint64>> m_latencyMap;    ///< 模块延迟记录（每个标签保留最近100条）
};

#endif // PERFORMANCE_MONITOR_H
