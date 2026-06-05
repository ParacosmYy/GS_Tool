/**
 * @file SpectrumTypes.h
 * @brief 频谱监控器基础数据类型 — 参数/窗函数枚举/频谱切片
 *
 * 设计: 纯数据结构(无行为)、与QWidget解耦、全部字段零初始化
 * 协作: SpectrumMonitor(消费SpectrumParams/产生SpectrumSlice)
 *        SpectrumMonitorWidget(显示SpectrumSlice)
 */

#ifndef SPECTRUMTYPES_H
#define SPECTRUMTYPES_H

#include <QVector>
#include <QString>

/**
 * @brief 窗函数类型 — 控制FFT频谱泄漏与主瓣宽度的权衡
 */
enum class WindowFunction {
    Hamming,   ///< 海明窗 — 旁瓣衰减43dB，主瓣较窄
    Hann,      ///< 汉宁窗 — 旁瓣衰减31dB，频率分辨率均衡
    Blackman,  ///< 布莱克曼窗 — 旁瓣衰减58dB，主瓣最宽
    FlatTop,   ///< 平顶窗 — 幅度精度最高，频率分辨率最低
    Kaiser     ///< 凯塞窗 — 可调旁瓣衰减(beta参数)
};

/**
 * @brief 频谱监控参数 — 配置FFT分析特性
 *
 * sampleRate/fftSize决定频率分辨率(binWidth = sampleRate/fftSize)；
 * overlap控制STFT时间分辨率；window影响频谱泄漏特性。
 */
struct SpectrumParams {
    double sampleRate   = 48000.0; ///< 采样率(Hz)，默认48kHz
    int    fftSize      = 1024;    ///< FFT运算长度(2的幂)，默认1024
    WindowFunction window = WindowFunction::Hann; ///< 窗函数类型
    double overlap      = 0.5;     ///< 帧重叠比例(0.0~0.75)，默认50%
    double kaiserBeta   = 8.6;     ///< 凯塞窗beta参数(仅Kaiser窗使用)
};

/**
 * @brief 频谱切片 — 单次FFT计算的结果
 *
 * freqs/magnitudes长度为fftSize/2+1(单边频谱)；
 * magnitudes单位为dBFS(相对于满量程的分贝值)；
 * timestamp为数据采集起始时间戳(毫秒)。
 */
struct SpectrumSlice {
    QVector<double> freqs;       ///< 频率轴(Hz)，长度fftSize/2+1
    QVector<double> magnitudes;  ///< 幅度谱(dBFS)，长度fftSize/2+1
    double timestamp  = 0.0;     ///< 时间戳(毫秒，从启动起计时)
};

/**
 * @brief 将窗函数枚举转为可读字符串
 * @param wf 窗函数类型
 * @return 窗函数名称
 */
inline QString windowFunctionName(WindowFunction wf)
{
    switch (wf) {
    case WindowFunction::Hamming:  return QStringLiteral("Hamming");
    case WindowFunction::Hann:     return QStringLiteral("Hann");
    case WindowFunction::Blackman: return QStringLiteral("Blackman");
    case WindowFunction::FlatTop:  return QStringLiteral("FlatTop");
    case WindowFunction::Kaiser:   return QStringLiteral("Kaiser");
    }
    return QStringLiteral("Unknown");
}

#endif // SPECTRUMTYPES_H
