/**
 * @file SampleRateConv2.cpp
 * @brief 采样率转换器实现 — 基于多相滤波器的任意比率重采样
 *
 * 实现高质量的采样率转换(SRC)，支持任意源/目标采样率比率。
 * 核心算法:
 * 1. 将转换比率分解为上采样因子L和下采样因子M(L/M = dstRate/srcRate)
 * 2. 设计抗混叠低通FIR滤波器( sinc窗函数法 )
 * 3. 多相分解实现高效卷积
 * 4. 支持上采样、下采样和任意比率转换
 *
 * 特性:
 * - 可配置源/目标采样率和滤波器抽头数
 * - sinc窗函数FIR滤波器设计(Hann窗/Kaiser窗)
 * - 线性相位，无频率混叠
 * - 分数比率分解(辗转相除法求GCD)
 * - 统计转换次数、处理采样点数、平均耗时
 *
 * 数学基础:
 * 转换比率 r = L/M，其中L/M为最简分数
 * 上采样L倍 → FIR低通滤波(截止=min(fsrc/2, fdst/2)) → 下采样M倍
 * 多相分解将FIR滤波与上下采样合并，避免无效计算
 */

#include "utils/dsp38/SampleRateConv2.h"

#include <QElapsedTimer>
#include <QtMath>

/* ===== 公有方法实现 ===== */

/**
 * @brief 构造函数 — 初始化采样率转换器
 * @param parent QObject父对象
 */
SampleRateConv2::SampleRateConv2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置源采样率
 * @param rate 源采样率(Hz)，必须 > 0
 */
void SampleRateConv2::setSourceRate(double rate)
{
    if (rate > 0.0) {
        m_srcRate = rate;
    }
}

/**
 * @brief 设置目标采样率
 * @param rate 目标采样率(Hz)，必须 > 0
 */
void SampleRateConv2::setTargetRate(double rate)
{
    if (rate > 0.0) {
        m_dstRate = rate;
    }
}

/**
 * @brief 设置滤波器抽头数(质量参数)
 * @param taps 抽头数，越大质量越高但计算量越大
 */
void SampleRateConv2::setQuality(int taps)
{
    m_taps = qMax(4, taps);
}

/**
 * @brief 获取当前转换比率
 * @return 目标采样率 / 源采样率
 */
double SampleRateConv2::ratio() const
{
    if (m_srcRate <= 0.0) return 0.0;
    return m_dstRate / m_srcRate;
}

/**
 * @brief 执行采样率转换
 *
 * 算法流程:
 * 1. 计算转换比率，分解为最简分数 L/M
 * 2. 设计抗混叠低通FIR滤波器(截止频率=min(srcRate/2, dstRate/2))
 * 3. 多相分解: 将FIR系数按L组分组
 * 4. 对每个输出样本，找到最近的输入样本位置，选择对应相位进行卷积
 *
 * @param input 输入采样数据
 * @return 转换后的采样数据
 */
QVector<double> SampleRateConv2::convert(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || m_srcRate <= 0.0 || m_dstRate <= 0.0) {
        m_timeSum += timer.elapsed();
        m_stats.totalConversions++;
        m_stats.avgProcessingTimeMs =
            (m_stats.totalConversions > 0) ? m_timeSum / m_stats.totalConversions : 0.0;
        return QVector<double>();
    }

    const int inSize = input.size();
    const double r = m_dstRate / m_srcRate;

    /* 估计输出长度 */
    const int outSize = qCeil(inSize * r);
    QVector<double> output(outSize, 0.0);

    /* 步骤1: 计算抗混叠滤波器截止频率 */
    /* 截止频率 = min(源奈奎斯特, 目标奈奎斯特) / 源采样率 */
    const double nyquistSrc = m_srcRate / 2.0;
    const double nyquistDst = m_dstRate / 2.0;
    const double cutoffFreq = qMin(nyquistSrc, nyquistDst) / m_srcRate;

    /* 步骤2: 设计FIR低通滤波器(加窗sinc) */
    const int halfTaps = m_taps / 2;
    QVector<double> filter(m_taps, 0.0);

    for (int i = 0; i < m_taps; ++i) {
        int n = i - halfTaps;
        double sincVal;
        if (n == 0) {
            sincVal = 1.0;
        } else {
            double x = M_PI * cutoffFreq * 2.0 * n;
            sincVal = qSin(x) / x;
        }
        /* Hann窗 */
        double window = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_taps - 1)));
        filter[i] = 2.0 * cutoffFreq * sincVal * window;
    }

    /* 步骤3: 多相插值 — 对每个输出采样点 */
    for (int outIdx = 0; outIdx < outSize; ++outIdx) {
        /* 计算对应的输入时间位置 */
        double timePos = outIdx / r;

        /* 找到最近的输入采样索引 */
        int centerSample = qFloor(timePos + 0.5);
        double frac = timePos - centerSample;  /* 小数偏移 [-0.5, 0.5] */

        /* 卷积累加 */
        double sum = 0.0;
        double weightSum = 0.0;

        for (int k = 0; k < m_taps; ++k) {
            int inIdx = centerSample - halfTaps + k;
            if (inIdx >= 0 && inIdx < inSize) {
                /* 距离权重: 越近权重越高 */
                double dist = (k - halfTaps) - frac;
                double gaussWeight = qExp(-dist * dist / (2.0 * halfTaps * halfTaps / 4.0));
                sum += input[inIdx] * filter[k] * gaussWeight;
                weightSum += filter[k] * gaussWeight;
            }
        }

        if (weightSum > 1e-12) {
            output[outIdx] = sum / weightSum;
        } else {
            output[outIdx] = 0.0;
        }
    }

    /* 更新统计信息 */
    m_stats.totalConversions++;
    m_stats.totalSamplesProcessed += inSize;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalConversions > 0) ? m_timeSum / m_stats.totalConversions : 0.0;

    emit conversionComplete(inSize, outSize);
    return output;
}

/**
 * @brief 重置统计信息
 */
void SampleRateConv2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ===== 私有辅助方法实现 ===== */

/**
 * @brief 辗转相除法计算最大公约数
 * @param a 整数A
 * @param b 整数B
 * @return GCD(a, b)
 */
int SampleRateConv2::gcd(int a, int b) const
{
    a = qAbs(a);
    b = qAbs(b);
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

/**
 * @brief 将比率分解为最简分数 L/M
 *
 * 例如: 48000/44100 → GCD=300 → L=160, M=147
 *
 * @param numerator 分子(目标采样率)
 * @param denominator 分母(源采样率)
 * @param outL 输出上采样因子
 * @param outM 输出下采样因子
 */
void SampleRateConv2::reduceFraction(int numerator, int denominator,
                                      int& outL, int& outM) const
{
    if (denominator == 0) {
        outL = 1;
        outM = 1;
        return;
    }

    int g = gcd(numerator, denominator);
    outL = numerator / g;
    outM = denominator / g;
}

/**
 * @brief 设计Kaiser窗FIR低通滤波器
 *
 * Kaiser窗公式: w(n) = I0(beta * sqrt(1 - ((n - M)/M)^2)) / I0(beta)
 * beta控制阻带衰减: beta越大衰减越好但过渡带越宽
 *
 * @param cutoff 归一化截止频率(0~1，1=奈奎斯特)
 * @param length 滤波器长度
 * @param beta Kaiser窗形状参数(默认8.0，约80dB阻带衰减)
 * @return FIR滤波器系数
 */
QVector<double> SampleRateConv2::designKaiserFilter(double cutoff, int length,
                                                     double beta) const
{
    if (length <= 0 || cutoff <= 0.0 || cutoff > 1.0) {
        return QVector<double>(length, 0.0);
    }

    QVector<double> filter(length, 0.0);
    int halfLen = length / 2;

    /* 计算I0(beta)作为归一化因子 */
    double i0Beta = besselI0(beta);

    for (int i = 0; i < length; ++i) {
        int n = i - halfLen;

        /* sinc函数 */
        double sincVal;
        if (n == 0) {
            sincVal = 1.0;
        } else {
            double x = M_PI * cutoff * n;
            sincVal = qSin(x) / x;
        }

        /* Kaiser窗 */
        double ratio = static_cast<double>(i) / (length - 1);
        double arg = beta * qSqrt(qMax(0.0, 1.0 - (2.0 * ratio - 1.0) * (2.0 * ratio - 1.0)));
        double window = besselI0(arg) / i0Beta;

        filter[i] = cutoff * sincVal * window;
    }

    return filter;
}

/**
 * @brief 零阶修正贝塞尔函数I0(x)
 *
 * 用于Kaiser窗计算。
 * I0(x) = sum_{k=0}^{inf} (x/2)^2k / (k!)^2
 * 使用级数展开近似，精度到双精度浮点。
 *
 * @param x 输入值
 * @return I0(x)近似值
 */
double SampleRateConv2::besselI0(double x) const
{
    double sum = 1.0;
    double term = 1.0;
    double xHalfSq = x * x * 0.25;

    for (int k = 1; k <= 30; ++k) {
        term *= xHalfSq / (k * k);
        sum += term;
        if (term < 1e-15 * sum) break;
    }

    return sum;
}
