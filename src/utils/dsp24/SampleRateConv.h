/**
 * @file SampleRateConv.h
 * @brief 采样率转换器 — 多相分解+线性插值
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 采样率转换器
 * 支持任意比率转换, 使用多相FIR+线性插值
 */
class SampleRateConv : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalConversions = 0;        ///< 累计转换次数
        int totalSamplesIn = 0;          ///< 累计输入采样数
        int totalSamplesOut = 0;         ///< 累计输出采样数
        double avgProcessingTimeMs = 0.0;
    };

    /**
     * @brief 构造函数
     * @param inputRate 输入采样率
     * @param outputRate 输出采样率
     * @param filterLength 抗混叠滤波器长度
     * @param parent 父对象
     */
    explicit SampleRateConv(double inputRate = 44100.0,
                            double outputRate = 48000.0,
                            int filterLength = 64,
                            QObject* parent = nullptr);

    /** @brief 转换采样率 @param input 输入信号 @return 输出信号 */
    QVector<double> convert(const QVector<double>& input);

    /** @brief 获取转换比率 */
    double ratio() const { return m_ratio; }

    /** @brief 获取输入采样率 */
    double inputRate() const { return m_inputRate; }

    /** @brief 获取输出采样率 */
    double outputRate() const { return m_outputRate; }

    /** @brief 获取滤波器延迟(采样数) */
    double filterDelay() const { return m_filterDelay; }

    /** @brief 获取抗混叠滤波器系数 */
    QVector<double> filterCoeffs() const { return m_filter; }

    /** @brief 设置转换比率 @param inputRate 输入采样率 @param outputRate 输出采样率 */
    void setRates(double inputRate, double outputRate);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 转换完成 @param samplesIn 输入采样数 @param samplesOut 输出采样数 */
    void conversionCompleted(int samplesIn, int samplesOut);

private:
    /** @brief 设计抗混叠滤波器 */
    void designFilter();

    double m_inputRate;                  ///< 输入采样率
    double m_outputRate;                 ///< 输出采样率
    double m_ratio;                      ///< 转换比率
    int m_filterLength;                  ///< 滤波器长度
    double m_filterDelay;                ///< 滤波器延迟
    QVector<double> m_filter;            ///< 抗混叠滤波器系数

    Stats m_stats;
    double m_timeSum = 0.0;
};
