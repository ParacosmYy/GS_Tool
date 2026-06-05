/**
 * @file FilterTypes.h
 * @brief 数字滤波器设计基础数据类型 — 参数/系数/频率响应定义
 *
 * 设计: 纯数据结构(无行为)、与QPainter/QWidget解耦、全部字段零初始化
 * 协作: FilterDesigner(消费FilterParams/产生FilterCoeffs/FilterResponse)
 *        FilterDesignerWidget(显示FilterResponse)
 */

#ifndef FILTERTYPES_H
#define FILTERTYPES_H

#include <QVector>
#include <QString>

/**
 * @brief 滤波器响应类型 — 低通/高通/带通/带阻
 */
enum class FilterType {
    LowPass,   ///< 低通滤波器
    HighPass,  ///< 高通滤波器
    BandPass,  ///< 带通滤波器
    BandStop   ///< 带阻(陷波)滤波器
};

/**
 * @brief 滤波器族 — IIR近似方式或FIR窗函数法
 */
enum class FilterFamily {
    Butterworth,  ///< 巴特沃斯 — 最大平坦幅频
    Chebyshev1,   ///< 切比雪夫I型 — 等波纹通带
    Chebyshev2,   ///< 切比雪夫II型 — 等波纹阻带
    Bessel,       ///< 贝塞尔 — 最大平坦群延迟
    FIR           ///< FIR窗函数法 — 线性相位
};

/**
 * @brief FIR窗函数类型
 */
enum class WindowType {
    Hamming,   ///< 汉明窗
    Hanning,   ///< 汉宁窗
    Blackman,  ///< 布莱克曼窗
    Rectangular ///< 矩形窗
};

/**
 * @brief 滤波器设计参数 — 配置滤波器特性
 *
 * type/family决定设计算法；order为滤波器阶数；
 * cutoffFreq为低通/高通截止频率(Hz)，bandLow/bandHigh为带通/带阻频率(Hz)；
 * sampleRate为采样率(Hz)；rippleDb仅Chebyshev有效。
 */
struct FilterParams {
    FilterType   type       = FilterType::LowPass;  ///< 响应类型
    FilterFamily family     = FilterFamily::Butterworth; ///< 滤波器族
    int          order      = 4;                     ///< 滤波器阶数(N)
    double       cutoffFreq = 1000.0;               ///< 截止频率(Hz)，LP/HP
    double       bandLow    = 500.0;                 ///< 带通/带阻低频(Hz)
    double       bandHigh   = 2000.0;                ///< 带通/带阻高频(Hz)
    double       sampleRate = 44100.0;               ///< 采样率(Hz)
    double       rippleDb   = 0.5;                   ///< 通带波纹(dB)，Chebyshev用
    WindowType   window     = WindowType::Hamming;   ///< FIR窗函数类型
};

/**
 * @brief 滤波器系数 — 传递函数 H(z) = B(z) / A(z)
 *
 * b为分子系数(前馈)，a为分母系数(反馈)，a[0]归一化为1.0。
 */
struct FilterCoeffs {
    QVector<double> b;  ///< 分子系数 (前馈)
    QVector<double> a;  ///< 分母系数 (反馈)，a[0] = 1.0
};

/**
 * @brief 频率响应数据 — 幅度/相位随频率变化
 *
 * freq为频率轴(Hz)，mag为幅度(dB)，phase为相位(度)。
 */
struct FilterResponse {
    QVector<double> freq;   ///< 频率轴(Hz)
    QVector<double> mag;    ///< 幅度响应(dB)
    QVector<double> phase;  ///< 相位响应(度)
};

#endif // FILTERTYPES_H
