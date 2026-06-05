/**
 * @file ChirpZTransform.h
 * @brief Chirp-Z变换 — 螺旋等间隔频率采样
 *
 * 功能: 实现Bluestein算法的Chirp-Z变换，支持任意起始/终止频率、
 *       任意输出点数(M不依赖N)、频带细化分析(Zoom-FFT)，
 *       统计变换次数/平均处理耗时。
 */
#ifndef CHIRPZTRANSFORM_H
#define CHIRPZTRANSFORM_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class ChirpZTransform
 * @brief Chirp-Z变换器，支持螺旋等间隔频率采样
 */
class ChirpZTransform : public QObject {
    Q_OBJECT
public:
    /** 变换结果 */
    struct CztResult {
        QVector<double> frequencies;    ///< 频率轴(Hz)
        QVector<double> magnitude;      ///< 幅度谱
        QVector<double> phase;          ///< 相位谱(rad)
        double peakFrequency;           ///< 峰值频率(Hz)
        double peakMagnitude;           ///< 峰值幅度
    };

    /** 统计信息 */
    struct Stats {
        quint64 totalTransforms = 0;        ///< 总变换次数
        quint64 totalPointsProcessed = 0;   ///< 累计处理点数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit ChirpZTransform(QObject* parent = nullptr);

    /** 设置参数 */
    void setSampleRate(double rate);
    void setFrequencyRange(double startHz, double endHz);
    void setOutputPoints(int m);
    void setInputLength(int n);

    /** 执行Chirp-Z变换 */
    CztResult transform(const QVector<double>& data);

    /** Zoom-FFT: 对指定频段做高分辨率分析 */
    CztResult zoomFft(const QVector<double>& data, double centerHz,
                      double bandwidthHz, int resolution);

    /** 逆Chirp-Z变换 */
    QVector<double> inverseTransform(const QVector<double>& magnitude,
                                     const QVector<double>& phase);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** 变换完成信号 */
    void transformComplete(int inputLen, int outputLen, double peakHz);
    /** 峰值检测信号 */
    void peakDetected(double frequencyHz, double magnitude);

private:
    /** Bluestein FFT核心(利用FFT实现卷积) */
    void bluesteinFft(const QVector<double>& input,
                      QVector<double>& outReal, QVector<double>& outImag);
    /** 基2 FFT */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse);
    /** 计算下一个2的幂 */
    int nextPow2(int n) const;
    /** 生成chirp信号 */
    void generateChirp(int len, QVector<double>& real, QVector<double>& imag);

    double m_sampleRate;
    double m_startFreq;
    double m_endFreq;
    int m_outputPoints;
    int m_inputLength;
    Stats m_stats;
    double m_timeSum;
};

#endif // CHIRPZTRANSFORM_H
