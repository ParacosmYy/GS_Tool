/**
 * @file BinauralProcessor.h
 * @brief 双耳空间音频处理(HRTF建模+ITD/ILD+串扰消除) — HRTF-Based Binaural Spatial Audio with ITD/ILD Modeling and Cross-Talk Cancellation
 *
 * 功能: 实现双耳空间音频处理，包括头相关传递函数(HRTF)建模、
 *       基于ITD(耳间时间差)/ILD(耳间声级差)的空间定位及串扰消除。
 *
 * 协作: AdaptiveLineEnhancer2(自适应线增强) / DiscreteHartleyTransform(DHT) / FirFilter(滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 双耳空间音频处理器
 */
class BinauralProcessor : public QObject {
    Q_OBJECT

public:
    /** @brief 声源位置(方位角+仰角) */
    struct SourcePosition {
        double azimuth = 0.0;   ///< 方位角(度, -180~180)
        double elevation = 0.0; ///< 仰角(度, -90~90)
        double distance = 1.0;  ///< 距离(归一化)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProcessed = 0;       ///< 累计处理帧数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int lastFrameSize = 0;            ///< 最近帧大小
        int lastSampleRate = 0;           ///< 最近采样率
    };

    explicit BinauralProcessor(QObject *parent = nullptr);
    ~BinauralProcessor() override;

    /** @brief 设置采样率(Hz) */
    void setSampleRate(int rate);

    /** @brief 设置HRTF滤波器长度 */
    void setHrtfLength(int length);

    /** @brief 设置串扰消除强度(0~1) */
    void setCrossTalkStrength(double strength);

    /**
     * @brief 处理单声道信号为双耳信号
     * @param mono 单声道输入
     * @param pos 声源位置
     * @return QPair<左耳,右耳> 输出
     */
    QPair<QVector<double>, QVector<double>> process(
        const QVector<double>& mono, const SourcePosition& pos);

    /** @brief 获取左耳HRTF冲激响应 */
    QVector<double> leftHrtf(const SourcePosition& pos) const;

    /** @brief 获取右耳HRTF冲激响应 */
    QVector<double> rightHrtf(const SourcePosition& pos) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param frames 帧数 */
    void processingCompleted(int frames);

private:
    /** @brief 计算ITD(采样点数) */
    int computeITD(const SourcePosition& pos) const;

    /** @brief 计算ILD(线性增益比) */
    double computeILD(const SourcePosition& pos) const;

    /** @brief 生成简化HRTF */
    QVector<double> generateHrtf(double azimuth, bool isIpsi) const;

    /** @brief FIR卷积 */
    static QVector<double> convolve(const QVector<double>& signal,
                                    const QVector<double>& kernel);

    /** @brief 串扰消除 */
    void applyCrossTalk(QVector<double>& left, QVector<double>& right) const;

    int m_sampleRate = 44100;
    int m_hrtfLen = 64;
    double m_xtStrength = 0.4;

    Stats m_stats;
    double m_timeSum = 0.0;
};
