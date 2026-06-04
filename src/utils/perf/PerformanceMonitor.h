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
    /** @brief 构造函数 @param parent 父对象 */
    explicit PerformanceMonitor(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~PerformanceMonitor() override;

    /** @brief 标记帧开始 */
    void beginFrame();

    /** @brief 标记帧结束，更新统计数据 */
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

    /** @brief 便捷方法：标记完整一帧（beginFrame + endFrame） */
    void recordFrame();

    /** @brief 通知发生一次GC暂停事件，递增totalGcPauses计数器 */
    void notifyGcPause();

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

    // ---- 统计计数器接口 ----

    /** @brief 获取累计采样帧数 @return 帧数 */
    quint64 totalSamples() const;

    /** @brief 获取历史最高FPS @return 最大FPS值 */
    quint64 maxFps() const;

    /** @brief 获取历史最低FPS(至少采样一帧后有效) @return 最小FPS值 */
    quint64 minFps() const;

    /** @brief 获取累计延迟测量次数 @return 测量次数 */
    quint64 totalMeasurements() const;

    /** @brief 获取累计统计报告次数(每30帧一次) */
    quint64 totalReports() const { return m_totalReports; }
    /** @brief 获取历史最高FPS(别名) @return 最大FPS值 */
    quint64 peakFps() const { return m_maxFps; }
    /** @brief 获取累计GC暂停次数 */
    quint64 totalGcPauses() const { return m_totalGcPauses; }
    /** @brief 获取累计慢帧数(帧耗时>100ms) */
    quint64 totalSlowFrames() const { return m_totalSlowFrames; }

    /** @brief 重置所有统计计数器(采样/FPS/测量次数) */
    void resetPerformanceStatistics();

signals:
    /** @brief 统计数据更新信号 @param fps 帧率 @param avgFrameMs 平均帧耗时(ms) @param memBytes 内存占用(字节) */
    void statsUpdated(double fps, double avgFrameMs, qint64 memBytes);

private:
    QElapsedTimer m_frameTimer;                    ///< 帧计时器
    double        m_fps       = 0.0;               ///< 当前 FPS
    double        m_avgFrameMs = 0.0;              ///< 平均帧耗时（EMA）
    qint64        m_frameCount = 0;                ///< 帧计数
    QMap<QString, QList<quint64>> m_latencyMap;    ///< 模块延迟记录（每个标签保留最近100条）

    // 统计计数器
    quint64 m_totalSamples = 0;       ///< 累计采样帧数
    quint64 m_maxFps = 0;             ///< 历史最高FPS
    quint64 m_minFps = 0;             ///< 历史最低FPS
    quint64 m_totalMeasurements = 0;  ///< 累计延迟测量次数
    quint64 m_totalReports = 0;       ///< 累计统计报告次数(每30帧一次)
    quint64 m_totalGcPauses = 0;      ///< 累计GC暂停次数
    quint64 m_totalSlowFrames = 0;    ///< 累计慢帧数(帧耗时>100ms)
};

#endif // PERFORMANCE_MONITOR_H
