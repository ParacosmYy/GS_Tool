#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 常数Q变换(Constant-Q Transform)实现
 *
 * 在对数频率尺度上进行等Q值(品质因子)的频谱分析，
 * 适用于音乐信号处理中对数音阶频率分辨需求。
 */
class ConstantQ6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit ConstantQ6(QObject* parent = nullptr);

    /** @brief 设置最低分析频率(Hz) */
    void setMinFreq(double freq);

    /** @brief 设置最高分析频率(Hz) */
    void setMaxFreq(double freq);

    /** @brief 设置每八度的频率箱数 */
    void setBins(int binsPerOctave);

    /** @brief 计算常数Q频谱 */
    QVector<double> compute(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computationCompleted(int binCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_minFreq = 27.5;
    double m_maxFreq = 4186.0;
    int m_bins = 24;
};
