/**
 * @file WaveGenTypes.h
 * @brief 波形发生器类型定义 -- 枚举/参数结构体/统计结构体
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * WaveformGenerator 和 WaveformGeneratorWidget 共享的基础类型。
 * 独立于 QObject，可被非 Qt 模块直接包含。
 */

#ifndef WAVEGENTYPES_H
#define WAVEGENTYPES_H

#include <QByteArray>
#include <QVector>
#include <QtGlobal>

/**
 * @brief 波形发生器类型定义命名空间
 *
 * 将枚举、参数、统计结构体集中于此，避免全局污染。
 */
namespace WaveGen {

/** @brief 波形类型枚举 */
enum class WaveformType {
    Sine     = 0,  ///< 正弦波
    Square   = 1,  ///< 方波
    Triangle = 2,  ///< 三角波
    Sawtooth = 3,  ///< 锯齿波
    Noise    = 4,  ///< 白噪声
    Custom   = 5   ///< 用户自定义波形(通过数据点线性插值)
};

/** @brief 量化位宽枚举 */
enum class BitDepth {
    Bits8  = 8,   ///< 8 位无符号 (0~255)
    Bits16 = 16,  ///< 16 位有符号 (-32768~32767)
    Bits32 = 32   ///< 32 位有符号
};

/** @brief 字节序枚举 */
enum class Endianness {
    LittleEndian,   ///< 小端序 (x86/ARM 默认)
    BigEndian       ///< 大端序 (网络字节序)
};

/** @brief 调制类型枚举 */
enum class ModulationType {
    None = 0,  ///< 无调制
    AM,        ///< 幅度调制
    FM         ///< 频率调制
};

/**
 * @brief 波形生成参数
 */
struct WaveGenParams {
    WaveformType type       = WaveformType::Sine;  ///< 波形类型
    double frequency        = 1000.0;              ///< 信号频率 (Hz)
    double amplitude        = 1.0;                 ///< 振幅
    double offset           = 0.0;                 ///< 直流偏移
    double phase            = 0.0;                 ///< 初始相位 (弧度)
    double dutyCycle        = 0.5;                 ///< 方波占空比 (0.0~1.0)
    int    sampleRate       = 48000;               ///< 采样率 (Sa/s)
    int    sampleCount      = 4800;                ///< 生成采样点数
    BitDepth bitDepth       = BitDepth::Bits16;    ///< 量化位宽
    Endianness endian       = Endianness::LittleEndian; ///< 输出字节序
    ModulationType modType  = ModulationType::None; ///< 调制类型
    double modFrequency     = 100.0;               ///< 调制频率 (Hz)
    double modDepth         = 0.5;                 ///< 调制深度 (0.0~1.0)
    double sweepEndFreq     = 5000.0;              ///< 扫频终止频率 (Hz)
    QVector<double> customData;                    ///< 自定义波形数据点
};

/**
 * @brief 波形发生器统计信息
 */
struct Stats {
    quint64 totalGenerations   = 0;  ///< 累计生成次数
    quint64 totalSamples       = 0;  ///< 累计采样点数
    quint64 totalBytesOutput   = 0;  ///< 累计输出字节数
    quint64 streamingChunks    = 0;  ///< 累计流式输出块数
    quint64 errorCount         = 0;  ///< 累计错误次数
};

} // namespace WaveGen

#endif // WAVEGENTYPES_H
