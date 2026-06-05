/**
 * @file SpectralWhitening.h
 * @brief 谱白化 — 频域均衡/去相关
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 谱白化引擎
 * 将信号功率谱白化(平坦化),用于去相关和增强微弱信号
 */
class SpectralWhitening : public QObject
{
    Q_OBJECT

public:
    /** @brief 白化方法 */
    enum Method {
        Direct,        ///< 直接除以幅度谱
        PhaseOnly,     ///< 仅保留相位(置幅度为1)
        FrequencyMask, ///< 频率掩模平滑
        Adaptive       ///< 自适应均衡
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalWhitens = 0;             ///< 累计白化次数
        int totalSamplesProcessed = 0;    ///< 累计处理采样数
        double avgProcessingTimeMs = 0.0;
    };

    /**
     * @brief 构造函数
     * @param fftSize FFT大小(2的幂)
     * @param parent 父对象
     */
    explicit SpectralWhitening(int fftSize = 1024, QObject* parent = nullptr);

    /** @brief 白化信号 @param signal 输入信号 @param method 白化方法 @return 白化后信号 */
    QVector<double> whiten(const QVector<double>& signal, Method method = Direct);

    /** @brief 自适应均衡(估计参考谱后白化) @param signal 输入信号 @param smoothingFactor 平滑因子[0,1] */
    QVector<double> adaptiveWhiten(const QVector<double>& signal,
                                    double smoothingFactor = 0.98);

    /** @brief 计算谱平坦度 @param signal 输入信号 @return 平坦度[0,1], 1=完全平坦 */
    double spectralFlatness(const QVector<double>& signal) const;

    /** @brief 计算功率谱 @param signal 输入信号 @return 功率谱 */
    QVector<double> powerSpectrum(const QVector<double>& signal) const;

    /** @brief 获取FFT大小 */
    int fftSize() const { return m_fftSize; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 白化完成 @param flatnessBefore 白化前平坦度 @param flatnessAfter 白化后平坦度 */
    void whiteningCompleted(double flatnessBefore, double flatnessAfter);

private:
    /** @brief 基2 FFT */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    int m_fftSize;
    Stats m_stats;
    double m_timeSum = 0.0;
};
