/**
 * @file WaveformGenerator.h
 * @brief 数学波形发生器 -- 生成9种标准测试波形,支持噪声叠加/归一化/二进制转换
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 用于示波器/图表组件的测试数据生成,也可作为信号分析模块的基准源。
 * 所有波形计算基于 <cmath>,无外部依赖。
 * 线程安全: 本类为可重入,但非线程安全;多线程环境需外部同步。
 */

#ifndef WAVEFORMGENERATOR_H
#define WAVEFORMGENERATOR_H

#include <QByteArray>
#include <QObject>
#include <QVector>

/**
 * @class WaveformGenerator
 * @brief 数学波形发生器,生成标准测试波形并转换为二进制数据流
 *
 * 支持正弦/方波/三角/锯齿/白噪声/粉红噪声/扫频/脉冲/直流共9种波形,
 * 提供噪声叠加、归一化、二进制转换等后处理工具方法,
 * 以及峰值/均值/RMS/频率估计等统计功能。
 */
class WaveformGenerator : public QObject {
    Q_OBJECT

public:
    /** @brief 波形类型枚举 */
    enum WaveformType {
        Sine        = 0,  ///< 正弦波: A*sin(2*pi*f*t + phi) + offset
        Square      = 1,  ///< 方波: A*sign(sin(2*pi*f*t + phi)) + offset
        Triangle    = 2,  ///< 三角波: A*(2*|2*(ft+phi)/T - 1| - 1) + offset
        Sawtooth    = 3,  ///< 锯齿波: A*(2*(ft+phi)/T - floor) + offset
        WhiteNoise  = 4,  ///< 白噪声: 均匀分布U(-A,A) + offset
        PinkNoise   = 5,  ///< 粉红噪声: Voss-McCartney算法
        Chirp       = 6,  ///< 线性扫频: f0→f1线性变化
        Impulse     = 7,  ///< 脉冲: 仅首采样点为A,其余为offset
        DC          = 8   ///< 直流: 所有采样点恒等于offset+A
    };
    Q_ENUM(WaveformType)

    /**
     * @brief 波形生成参数结构体
     */
    struct GenParams {
        WaveformType type = Sine;    ///< 波形类型
        double frequency   = 1.0;    ///< 频率(Hz), Chirp时为起始频率
        double amplitude   = 1.0;    ///< 振幅(峰值偏移量)
        double offset      = 0.0;    ///< 直流偏移
        double phase       = 0.0;    ///< 初始相位(弧度)
        double sampleRate  = 1000.0; ///< 采样率(Sa/s)
        int    sampleCount = 1000;   ///< 采样点数
        double noiseLevel  = 0.0;    ///< 叠加噪声强度(0.0~1.0)
        double chirpEndFreq = 100.0; ///< Chirp终止频率(Hz),仅Chirp有效
    };

    /**
     * @brief 波形统计信息结构体
     */
    struct Stats {
        quint64 totalGenerations    = 0;  ///< 累计生成次数
        quint64 totalSamples        = 0;  ///< 累计采样点数
        quint64 totalConversions    = 0;  ///< 累计二进制转换次数
        quint64 totalBytesConverted = 0;  ///< 累计转换字节数
        quint64 errorCount          = 0;  ///< 累计错误次数(参数无效等)
    };

    /**
     * @brief 波形数据分析结果
     */
    struct WaveformAnalysis {
        double peak      = 0.0;  ///< 峰值(最大绝对值)
        double peakToPeak = 0.0; ///< 峰峰值(max - min)
        double mean      = 0.0;  ///< 均值
        double rms       = 0.0;  ///< 均方根值
        double min       = 0.0;  ///< 最小值
        double max       = 0.0;  ///< 最大值
    };

    /** @brief 构造波形发生器 @param parent 父对象 */
    explicit WaveformGenerator(QObject *parent = nullptr);

    // ── 核心生成接口 ──

    /** @brief 按参数结构体生成波形 @param params 生成参数 @return 采样值向量 */
    QVector<double> generate(const GenParams &params);

    /** @brief 生成正弦波 @param freqHz 频率 @param amp 振幅 @param offset 偏移 @param sampleRate 采样率 @param count 采样数 @param phase 相位(弧度) */
    QVector<double> generateSine(double freqHz, double amp, double offset,
                                 double sampleRate, int count, double phase = 0.0);

    /** @brief 生成方波 */
    QVector<double> generateSquare(double freqHz, double amp, double offset,
                                   double sampleRate, int count, double phase = 0.0);

    /** @brief 生成三角波 */
    QVector<double> generateTriangle(double freqHz, double amp, double offset,
                                     double sampleRate, int count, double phase = 0.0);

    /** @brief 生成锯齿波 */
    QVector<double> generateSawtooth(double freqHz, double amp, double offset,
                                     double sampleRate, int count, double phase = 0.0);

    /** @brief 生成白噪声 */
    QVector<double> generateWhiteNoise(double amp, double offset, int count);

    /** @brief 生成粉红噪声(Voss-McCartney) */
    QVector<double> generatePinkNoise(double amp, double offset, int count);

    /** @brief 生成线性扫频信号 @param startFreq 起始频率 @param endFreq 终止频率 */
    QVector<double> generateChirp(double startFreq, double endFreq, double amp,
                                  double offset, double sampleRate, int count);

    /** @brief 生成脉冲信号(仅首点为amp) */
    QVector<double> generateImpulse(double amp, double offset, int count);

    /** @brief 生成直流信号 */
    QVector<double> generateDC(double amp, double offset, int count);

    // ── 后处理接口 ──

    /** @brief 向数据叠加高斯白噪声 @param data 原始数据 @param level 噪声强度(标准差比例) @return 叠加噪声后的数据 */
    QVector<double> addNoise(const QVector<double> &data, double level) const;

    /** @brief 归一化数据到[-1, 1]范围 @param data 原始数据 @return 归一化后的数据 */
    QVector<double> normalize(const QVector<double> &data) const;

    /** @brief 将采样数据转换为二进制字节流 @param data 采样数据 @param bitsPerSample 每采样位宽(8/16/32) @return 字节数组(小端序) */
    QByteArray toByteArray(const QVector<double> &data, int bitsPerSample) const;

    // ── 分析接口 ──

    /** @brief 分析波形数据,计算峰值/均值/RMS等 @param data 采样数据 @return 分析结果 */
    WaveformAnalysis analyze(const QVector<double> &data) const;

    // ── 统计接口 ──

    /** @brief 获取统计信息快照 @return 当前统计数据 */
    Stats stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

private:
    Stats m_stats;  ///< 统计计数器
};

#endif // WAVEFORMGENERATOR_H
