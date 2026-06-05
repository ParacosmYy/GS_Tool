/**
 * @file ConstantQTransform.h
 * @brief 常数Q变换(Constant-Q Transform)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QVector2D>

/**
 * @class ConstantQTransform
 * @brief 常数Q变换 — 对数频率分辨率的频谱分析
 *
 * CQT在低频段有高频率分辨率，高频段有高时间分辨率，
 * 更符合音乐音高的对数分布特性。
 */
class ConstantQTransform : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTransforms = 0;    /**< 总变换次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param minFreq 最低频率(Hz)
     * @param maxFreq 最高频率(Hz)
     * @param binsPerOctave 每倍频程的频率箱数
     * @param sampleRate 采样率
     * @param parent 父对象
     */
    explicit ConstantQTransform(double minFreq = 32.7, double maxFreq = 2093.0,
                                 int binsPerOctave = 12,
                                 double sampleRate = 44100.0,
                                 QObject* parent = nullptr);

    /**
     * @brief 执行CQT变换
     * @param signal 输入信号
     * @return CQT频谱矩阵[bins × frames]
     */
    QVector<QVector<double>> transform(const QVector<double>& signal) const;

    /**
     * @brief 获取频率轴标签
     * @return 各bin的中心频率(Hz)
     */
    QVector<double> frequencies() const;

    /** @brief 获取总bin数 */
    int binCount() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 变换完成信号 */
    void transformCompleted(int bins, int frames);

private:
    double m_minFreq;
    double m_maxFreq;
    int m_binsPerOctave;
    double m_sampleRate;
    int m_numBins;
    double m_qualityFactor;

    mutable Stats m_stats;
    mutable double m_timeSum;
};
