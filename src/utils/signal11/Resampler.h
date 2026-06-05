/**
 * @file Resampler.h
 * @brief 多相重采样器 — 任意采样率转换
 *
 * 功能: 实现基于多相分解的多相重采样器，支持任意比率的采样率转换。
 *       通过FIR抗混叠滤波器+多相分解结构，实现高效的有理数比
 *       重采样 (L/M)，可处理上采样、下采样和任意比转换。
 *
 * 协作: FirDesigner(FIR滤波器设计) / AdaptiveFft(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>
#include <QElapsedTimer>

/**
 * @brief 多相重采样器
 */
class Resampler : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int    totalResamples = 0;       ///< 累计重采样次数
        int    totalInputSamples = 0;    ///< 累计输入样本数
        int    totalOutputSamples = 0;   ///< 累计输出样本数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Resampler(QObject* parent = nullptr);

    /**
     * @brief 配置重采样参数
     * @param inputRate 输入采样率(Hz)
     * @param outputRate 输出采样率(Hz)
     * @param filterLength 滤波器长度(0=自动计算)
     * @param stopbandAttenuation 阻带衰减(dB，默认80)
     * @return 是否配置成功
     */
    bool configure(double inputRate, double outputRate,
                   int filterLength = 0,
                   double stopbandAttenuation = 80.0);

    /**
     * @brief 处理一批输入样本
     * @param input 输入采样数据
     * @return 重采样后的输出数据
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 冲刷残余样本(处理完所有数据后调用)
     * @return 冲刷出的残余输出样本
     */
    QVector<double> flush();

    /**
     * @brief 重置内部状态(不清除滤波器配置)
     */
    void reset();

    /**
     * @brief 获取理论输出长度
     * @param inputLength 输入样本数
     * @return 预计输出样本数
     */
    int expectedOutputLength(int inputLength) const;

    /** @brief 获取上采样因子L */
    int upFactor() const { return m_L; }
    /** @brief 获取下采样因子M */
    int downFactor() const { return m_M; }
    /** @brief 获取重采样比率 */
    double ratio() const { return m_M > 0 ?
        static_cast<double>(m_L) / m_M : 0.0; }
    /** @brief 是否已配置 */
    bool isConfigured() const { return m_configured; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param inCount 输入数 @param outCount 输出数 */
    void processCompleted(int inCount, int outCount);

private:
    /** @brief 设计抗混叠多相滤波器 */
    void designPolyphaseFilter(int filterLength, double stopbandAtten);

    /** @brief 计算最大公约数 */
    static int gcd(int a, int b);

    /** @brief sinc函数 */
    static double sinc(double x);

    /** @brief Kaiser窗函数 */
    static double kaiserWindow(int n, int N, double beta);

    /** @brief 计算Kaiser窗beta参数 */
    static double kaiserBeta(double attenuation);

    int m_L = 1;                 ///< 上采样因子
    int m_M = 1;                 ///< 下采样因子
    double m_inputRate = 0.0;    ///< 输入采样率
    double m_outputRate = 0.0;   ///< 输出采样率
    bool m_configured = false;   ///< 配置标志

    QVector<QVector<double>> m_polyPhase; ///< 多相滤波器系数 [phase][tap]
    int m_tapsPerPhase = 0;     ///< 每相抽头数

    int    m_phaseIndex = 0;    ///< 当前相位索引
    int    m_sampleIndex = 0;   ///< 当前样本索引
    QVector<double> m_delayLine; ///< 延迟线(环形缓冲)
    int    m_delayPos = 0;      ///< 延迟线写入位置
    int    m_delaySize = 0;     ///< 延迟线大小

    Stats              m_stats;
    double             m_timeSum = 0.0;
    mutable QElapsedTimer m_timer;
};
