/**
 * @file FftEngine.cpp
 * @brief FFT频谱计算引擎实现 -- 构造/工具方法 + Cooley-Tukey蝶形运算
 *
 * 实现 FftEngine 构造函数、工具方法(nextPowerOf2/log2Int/bitReverse/
 * windowTypeName)、主计算接口(compute)和Cooley-Tukey核心蝶形运算(fftRadix2)。
 *
 * 窗函数(applyWindow)/幅度谱(magnitudeSpectrum)/统计接口见 FftEngineWindow.cpp。
 */

#include "chart/fft/FftEngine.h"

#include <QtMath>
#include <algorithm>

// ============================================================
// 构造 / 工具
// ============================================================

/** @brief 构造FFT引擎 @param parent 父对象 */
FftEngine::FftEngine(QObject* parent)
    : QObject(parent)
{
}

/** @brief 计算大于等于n的最小2的幂 @param n 输入值 @return >=n的最小2的幂 */
int FftEngine::nextPowerOf2(int n)
{
    if (n <= 0) {
        return 1;
    }
    if ((n & (n - 1)) == 0) {
        return n;
    }
    int highest = 0;
    while (n > 0) {
        n >>= 1;
        ++highest;
    }
    return (1 << highest);
}

/** @brief 计算以2为底的对数(整数部分) @param n 输入值(必须为2的幂) @return log2(n) */
int FftEngine::log2Int(int n)
{
    int bits = 0;
    while (n > 1) {
        n >>= 1;
        ++bits;
    }
    return bits;
}

/** @brief 计算位反转索引(蝶形运算前数据重排用) @param index 原始索引 @param bits 索引位数 @return 位反转后的索引 */
int FftEngine::bitReverse(int index, int bits)
{
    int reversed = 0;
    for (int i = 0; i < bits; ++i) {
        reversed = (reversed << 1) | (index & 1);
        index >>= 1;
    }
    return reversed;
}

/** @brief 将窗函数类型转换为可读字符串 @param window 窗函数类型 @return 窗函数英文名称 */
QString FftEngine::windowTypeName(WindowType window)
{
    switch (window) {
    case WindowType::Rectangular: return QStringLiteral("Rectangular");
    case WindowType::Hanning:     return QStringLiteral("Hanning");
    case WindowType::Hamming:     return QStringLiteral("Hamming");
    case WindowType::Blackman:    return QStringLiteral("Blackman");
    }
    return QStringLiteral("Unknown");
}


// compute/fftRadix2见 FftEngineCompute.cpp
// applyWindow/magnitudeSpectrum/统计接口见 FftEngineWindow.cpp
