/**
 * @file ZeroCrossingDetector.h
 * @brief 过零率与周期检测器 — 基Fundamental频率估计
 *
 * 检测信号过零点并计算过零率和基频, 适用于音频基频估计、
 * 周期性分析、信号特征提取等嵌入式调试场景。
 */
#ifndef ZEROCROSSINGDETECTOR_H
#define ZEROCROSSINGDETECTOR_H

#include <QObject>
#include <QVector>

/**
 * @class ZeroCrossingDetector
 * @brief 过零检测器 — 过零率/周期/基频分析
 *
 * 典型用法:
 * @code
 *   ZeroCrossingDetector detector;
 *   detector.process(signal);
 *   double rate = detector.getCrossingRate();
 *   double freq = detector.getFundamentalFreq(44100);
 * @endcode
 */
class ZeroCrossingDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalProcessings = 0;    ///< 总处理次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit ZeroCrossingDetector(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~ZeroCrossingDetector() override;

    // ── 核心接口 ──

    /**
     * @brief 处理信号, 检测所有过零点
     * @param signal 输入信号
     */
    void process(const QVector<double>& signal);

    /**
     * @brief 获取过零率(每采样点过零次数)
     * @return 过零率(0.0~1.0之间)
     */
    double getCrossingRate() const;

    /**
     * @brief 估计基频
     * @param sampleRate 采样率(Hz)
     * @return 估计基频(Hz), 无信号时返回0
     */
    double getFundamentalFreq(double sampleRate) const;

    /**
     * @brief 获取过零点位置索引列表
     * @return 过零点采样索引列表
     */
    QVector<int> getCrossingIndices() const;

    /**
     * @brief 获取相邻过零点间隔(样本数)
     * @return 周期列表(样本数)
     */
    QVector<int> getPeriods() const;

    /** @brief 获取过零点总数 */
    int crossingCount() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 处理完成信号 @param crossings 过零点数量 */
    void processingCompleted(int crossings);

private:
    /** @brief 检测到的过零点索引 */
    QVector<int> m_crossings;

    /** @brief 过零率 */
    double m_crossingRate = 0.0;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // ZEROCROSSINGDETECTOR_H
