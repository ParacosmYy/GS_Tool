#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 常数Q变换(Constant-Q Transform)实现
 *
 * 在对数频率轴上提供恒定Q值的频谱分析,低频高分辨、高频低分辨,
 * 符合音乐音程特性,适用于音符检测、和弦分析与音乐信息检索。
 */
class ConstantQ5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit ConstantQ5(QObject* parent = nullptr);

    /** @brief 设置最低分析频率(Hz) */
    void setMinFreq(double freq);

    /** @brief 设置最高分析频率(Hz) */
    void setMaxFreq(double freq);

    /** @brief 设置每八度频率箱数 */
    void setBins(int binsPerOctave);

    /** @brief 计算常数Q变换 */
    void compute(const QVector<double>& samples);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号,返回频率箱数 */
    void computationCompleted(int binCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_minFreq = 27.5;
    double m_maxFreq = 4186.0;
    int m_bins = 12;
};
