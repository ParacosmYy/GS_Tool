/**
 * @file EnvelopeDetector.h
 * @brief 信号包络检测器2 — 峰值/RMS/Hilbert/Attack-Release
 *
 * 功能: 提取信号的包络曲线，支持峰值包络、RMS包络、Hilbert变换包络
 *       和Attack-Release包络四种方法，用于信号分析和音频处理。
 *       与envelope/EnvelopeDetector不同，本模块提供更细粒度的方法控制。
 *
 * 协作: SpectrumAnalyzer(频谱) / DigitalFilter(滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号包络检测器(增强版)
 */
class EnvelopeDetector2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDetections = 0;       ///< 累计检测次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit EnvelopeDetector2(QObject* parent = nullptr);

    /**
     * @brief 峰值包络
     * @param signal 输入信号
     * @param windowSize 窗口大小(必须为正奇数)
     * @return 包络曲线
     */
    QVector<double> peakEnvelope(const QVector<double>& signal, int windowSize);

    /**
     * @brief RMS包络
     * @param signal 输入信号
     * @param windowSize 窗口大小
     * @return RMS包络曲线
     */
    QVector<double> rmsEnvelope(const QVector<double>& signal, int windowSize);

    /**
     * @brief Hilbert变换包络
     * @param signal 输入信号
     * @return 包络曲线
     */
    QVector<double> hilbertEnvelope(const QVector<double>& signal);

    /**
     * @brief Attack-Release包络(音频风格)
     * @param signal 输入信号
     * @param attackCoeff 攻击系数(0-1, 越大越快)
     * @param releaseCoeff 释放系数(0-1, 越大越快)
     * @return 包络曲线
     */
    QVector<double> attackReleaseEnvelope(const QVector<double>& signal,
                                          double attackCoeff,
                                          double releaseCoeff);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 检测完成 @param method 检测方法名 @param length 结果长度 */
    void envelopeDetected(const QString& method, int length);

private:
    /**
     * @brief 计算Hilbert变换(通过DFT)
     * @param signal 输入信号
     * @return 解析信号的虚部
     */
    QVector<double> hilbertTransform(const QVector<double>& signal) const;

    Stats m_stats;       ///< 统计信息
    double m_timeSum;    ///< 累计耗时
};
