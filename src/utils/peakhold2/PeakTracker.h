/**
 * @file PeakTracker.h
 * @brief 实时峰值追踪器 — 滑动窗口+衰减
 *
 * 功能: 在数据流中实时追踪峰值，支持滑动窗口和
 *       可配置衰减因子，适用于频谱峰值检测。
 *
 * 协作: CepstralAnalysis(频谱分析) / Butterworth(滤波预处理)
 */
#ifndef PEAKTRACKER_H
#define PEAKTRACKER_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 实时峰值追踪器
 */
class PeakTracker : public QObject {
    Q_OBJECT

public:
    /** @brief 峰值信息 */
    struct Peak {
        double value = 0.0;     ///< 峰值幅度
        int position = 0;       ///< 峰值位置索引
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalUpdates = 0;           ///< 累计更新次数
        quint64 totalPeaks = 0;             ///< 累计检测峰值数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param windowSize 滑动窗口大小
     * @param decayFactor 衰减因子(0-1, 越小衰减越快)
     * @param parent 父对象
     */
    explicit PeakTracker(int windowSize = 64,
                         double decayFactor = 0.95,
                         QObject* parent = nullptr);

    /**
     * @brief 更新数据流中的值
     * @param value 新输入值
     */
    void update(double value);

    /**
     * @brief 获取当前所有峰值
     * @return 峰值列表(按幅度降序)
     */
    QVector<Peak> getPeaks() const;

    /**
     * @brief 重置追踪器状态
     */
    void reset();

    /** @brief 获取当前峰值 */
    Peak currentPeak() const { return m_currentPeak; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测到新峰值 @param value 峰值幅度 @param position 峰值位置 */
    void peakDetected(double value, int position);

private:
    /** @brief 在滑动窗口中检测峰值 */
    void detectPeaks();

    int m_windowSize;                    ///< 窗口大小
    double m_decayFactor;                ///< 衰减因子
    QVector<double> m_buffer;            ///< 环形缓冲区
    int m_writeIdx = 0;                  ///< 写入位置
    Peak m_currentPeak;                  ///< 当前峰值
    QVector<Peak> m_peaks;               ///< 峰值列表
    Stats m_stats;                       ///< 统计信息
    double m_timeSum = 0.0;              ///< 累计耗时
};

#endif // PEAKTRACKER_H
