/**
 * @file ChirpZ4.h
 * @brief Chirp-z变换(Bluestein算法任意FFT尺寸) — Chirp-Z Transform with Bluestein's Algorithm for Arbitrary FFT Sizes
 *
 * 功能: 实现Chirp-z变换，支持Bluestein算法将任意长度DFT转化为
 *       2的幂FFT计算，以及可配置螺旋线和频率范围。
 *
 * 协作: Goertzel4(Goertzel) / FftEngine(FFT引擎) / DftEngine(DFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Chirp-z变换处理器(Bluestein算法)
 */
class ChirpZ4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;       ///< 累计变换次数
        int inputSize = 0;                 ///< 输入长度
        int outputSize = 0;                ///< 输出长度
        int fftSize = 0;                   ///< 实际FFT长度
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit ChirpZ4(QObject *parent = nullptr);
    ~ChirpZ4() override;

    /** @brief 设置输出点数M */
    void setOutputSize(int M);

    /** @brief 设置频率范围起始W和步进A (复数) */
    void setSpiralParameters(double wReal, double wImag,
                             double aReal, double aImag);

    /**
     * @brief 执行Chirp-z变换
     * @param input 输入信号
     * @return {幅度谱, 相位谱}
     */
    QPair<QVector<double>, QVector<double>> transform(
        const QVector<double>& input);

    /**
     * @brief Bluestein算法: 任意长度DFT
     * @param input 输入信号
     * @return {实部, 虚部}
     */
    QPair<QVector<double>, QVector<double>> bluesteinDFT(
        const QVector<double>& input);

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int inputSize, int outputSize);

private:
    /** @brief 基2 Cooley-Tukey FFT(原地) */
    void fft(QVector<double>& re, QVector<double>& im, bool inverse) const;

    /** @brief 计算下一个2的幂 */
    static int nextPow2(int n);

    /** @brief 生成Chirp滤波器系数 */
    void generateChirp(int N, int M);

    int m_outputSize = 0;    ///< M=0时自动=N
    double m_wReal = 0.0;    ///< W实部(频率步进)
    double m_wImag = 0.0;    ///< W虚部
    double m_aReal = 1.0;    ///< A实部(起始点)
    double m_aImag = 0.0;    ///< A虚部

    QVector<double> m_chirpRe;   ///< 预计算的chirp系数实部
    QVector<double> m_chirpIm;   ///< 预计算的chirp系数虚部
    QVector<double> m_chirpFFTRe;///< chirp的FFT实部
    QVector<double> m_chirpFFTIm;///< chirp的FFT虚部
    int m_lastFFTSize = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
