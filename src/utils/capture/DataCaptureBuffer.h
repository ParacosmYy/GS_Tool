/**
 * @file DataCaptureBuffer.h
 * @brief 环形捕获缓冲区 — 存储最近N个数据采样用于分析
 *
 * 功能: 可配置缓冲区大小的环形缓冲，支持连续/触发/单次三种模式，
 *       提供CSV导出、溢出统计、触发门限检测。
 *
 * 协作: DataAggregator(数据流采集) / DataThresholdMonitor(触发联动)
 */
#ifndef DATACAPTUREBUFFER_H
#define DATACAPTUREBUFFER_H

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 环形捕获缓冲区 — 存储最近N个采样用于分析
 */
class DataCaptureBuffer : public QObject {
    Q_OBJECT

public:
    /** @brief 捕获模式 */
    enum class CaptureMode {
        Continuous, ///< 连续模式: 始终捕获(环形覆盖)
        Triggered,  ///< 触发模式: 仅在触发值超过门限后捕获
        OneShot     ///< 单次模式: 捕获至缓冲区满后停止
    };
    Q_ENUM(CaptureMode)

    /** @brief 统计 */
    struct Stats {
        quint64 totalSamplesCaptured = 0; ///< 累计捕获采样数
        quint64 totalTriggersFired   = 0; ///< 累计触发次数
        quint64 totalOverflows       = 0; ///< 累计溢出次数
        int     peakBufferSize       = 0; ///< 峰值缓冲区使用量
        quint64 samplesDropped       = 0; ///< 累计丢弃采样数
    };

    explicit DataCaptureBuffer(QObject* parent = nullptr);

    /** @brief 开始捕获 */
    void startCapture();

    /** @brief 停止捕获 */
    void stopCapture();

    /** @brief 写入一个采样值 @param value 数据值 */
    void addSample(double value);

    /** @brief 触发捕获(触发模式下激活采集) @param value 触发值 */
    void trigger(double value);

    /** @brief 设置缓冲区大小 @param size 最大采样数(默认10000) */
    void setBufferSize(int size);

    /** @brief 设置捕获模式 @param mode 捕获模式 */
    void setCaptureMode(CaptureMode mode);

    /** @brief 设置触发门限值 @param threshold 门限值 */
    void setTriggerThreshold(double threshold);

    /** @brief 获取已捕获的数据(按写入顺序) @return 数据列表 */
    QVector<double> capturedData() const;

    /** @brief 导出已捕获数据到CSV @param path 文件路径 @return 是否成功 */
    bool exportToCsv(const QString& path) const;

    /** @brief 当前缓冲区中的采样数 @return 采样数 */
    int sampleCount() const;

    /** @brief 是否正在捕获 @return 捕获状态 */
    bool isCapturing() const { return m_isCapturing; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void clearBuffer();

signals:
    /** @brief 捕获已开始 */
    void captureStarted();
    /** @brief 捕获已停止 */
    void captureStopped();
    /** @brief 数据溢出(环形覆盖) @param lostSamples 丢失的采样数 */
    void dataOverflow(int lostSamples);
    /** @brief 触发已触发 */
    void triggerFired();

private:
    void writeSample(double value);

    QVector<double> m_buffer;      ///< 环形缓冲区
    int m_writeIndex;              ///< 写入位置(环形)
    int m_count;                   ///< 当前缓冲区中的有效采样数
    CaptureMode m_captureMode;     ///< 捕获模式
    bool m_isCapturing;            ///< 捕获状态
    bool m_triggered;              ///< 触发模式下的触发标志
    double m_triggerThreshold;     ///< 触发门限值
    int m_bufferSize;              ///< 缓冲区最大容量

    Stats m_stats;
};

#endif // DATACAPTUREBUFFER_H
