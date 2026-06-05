/**
 * @file SampleRateConv4.cpp
 * @brief 采样率转换器实现
 *
 * 实现基于多相滤波器的采样率转换，支持任意比率的
 * 上采样和下采样。使用Kaiser窗设计低通抗混叠滤波器。
 */

#include "utils/dsp69/SampleRateConv4.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数并设计滤波器
 * @param parent 父对象指针
 */
SampleRateConv4::SampleRateConv4(QObject* parent)
    : QObject(parent)
{
    designFilter();
}

/**
 * @brief 设置源采样率
 * @param sr 源采样率(Hz)
 */
void SampleRateConv4::setSourceRate(double sr)
{
    m_source = qBound(8000.0, sr, 192000.0);
    designFilter();
}

/**
 * @brief 设置目标采样率
 * @param tr 目标采样率(Hz)
 */
void SampleRateConv4::setTargetRate(double tr)
{
    m_target = qBound(8000.0, tr, 192000.0);
    designFilter();
}

/**
 * @brief 设置滤波器长度
 * @param len 滤波器阶数
 */
void SampleRateConv4::setFilterLength(int len)
{
    m_filterLen = qBound(8, len, 512);
    designFilter();
}

/**
 * @brief 设置窗函数类型
 * @param type 窗函数名称："kaiser"、"hamming"、"blackman"
 */
void SampleRateConv4::setWindowType(const QString& type)
{
    if (type == "kaiser" || type == "hamming" || type == "blackman") {
        m_winType = type;
        designFilter();
    }
}

/**
 * @brief 处理采样率转换
 * @param input 输入采样数据
 * @return 转换后的采样数据
 */
QVector<double> SampleRateConv4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || m_filter.isEmpty()) return QVector<double>();

    // 计算整数上下采样因子（近似比率为分数L/M）
    double ratioVal = m_target / m_source;
    int L = 1, M = 1;

    // 寻找近似的整数比
    const int maxFactor = 128;
    double bestErr = 1e18;
    for (int l = 1; l <= maxFactor; ++l) {
        for (int m = 1; m <= maxFactor; ++m) {
            double err = qAbs(static_cast<double>(l) / m - ratioVal);
            if (err < bestErr) {
                bestErr = err;
                L = l;
                M = m;
                if (err < 1e-6) break;
            }
        }
        if (bestErr < 1e-6) break;
    }

    QVector<double> result;

    if (L > 1) {
        // 上采样
        QVector<double> up = polyphaseUp(input, L);
        if (M > 1) {
            // 下采样
            result = polyphaseDown(up, M);
        } else {
            result = up;
        }
    } else if (M > 1) {
        result = polyphaseDown(input, M);
    } else {
        result = input;
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalConversions++;
    m_stats.totalSamples += input.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConversions;

    emit conversionCompleted(input.size(), result.size());
    return result;
}

/**
 * @brief 重置统计信息
 */
void SampleRateConv4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设计抗混叠低通滤波器
 *
 * 使用窗函数法设计线性相位FIR低通滤波器，
 * 截止频率为min(fs_source, fs_target)/2。
 */
void SampleRateConv4::designFilter()
{
    m_filter.resize(m_filterLen);
    double cutoff = qMin(m_source, m_target) / 2.0 / qMax(m_source, m_target);
    double beta = 5.0; // Kaiser窗参数

    for (int i = 0; i < m_filterLen; ++i) {
        double n = i - (m_filterLen - 1) / 2.0;
        // sinc函数
        double sinc;
        if (qAbs(n) < 1e-10) {
            sinc = 1.0;
        } else {
            sinc = qSin(M_PI * 2.0 * cutoff * n) / (M_PI * n);
        }

        // 窗函数
        double win = 1.0;
        if (m_winType == "kaiser") {
            double alpha = (m_filterLen - 1) / 2.0;
            double x = (i - alpha) / alpha;
            if (qAbs(x) <= 1.0) {
                double a = qSqrt(1.0 - x * x);
                // 近似Kaiser窗使用Bessel函数
                double bessel = 1.0;
                double term = 1.0;
                for (int k = 1; k <= 10; ++k) {
                    term *= (beta * a / (2.0 * k));
                    bessel += term * term;
                }
                double bessel0 = 1.0;
                term = 1.0;
                for (int k = 1; k <= 10; ++k) {
                    term *= (beta / (2.0 * k));
                    bessel0 += term * term;
                }
                win = bessel / bessel0;
            }
        } else if (m_winType == "hamming") {
            win = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (m_filterLen - 1));
        } else if (m_winType == "blackman") {
            win = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (m_filterLen - 1))
                  + 0.08 * qCos(4.0 * M_PI * i / (m_filterLen - 1));
        }

        m_filter[i] = sinc * win * 2.0 * cutoff;
    }

    // 归一化
    double sum = 0.0;
    for (double c : m_filter) sum += c;
    if (sum > 0.0) {
        for (double& c : m_filter) c /= sum;
    }
}

/**
 * @brief 多相上采样
 * @param in 输入信号
 * @param factor 上采样因子
 * @return 上采样并滤波后的信号
 */
QVector<double> SampleRateConv4::polyphaseUp(const QVector<double>& in, int factor)
{
    // 插零后滤波
    int outLen = in.size() * factor;
    QVector<double> out(outLen, 0.0);

    for (int i = 0; i < in.size(); ++i) {
        out[i * factor] = in[i] * factor; // 补偿插零的幅度损失
    }

    // 卷积
    QVector<double> filtered(outLen, 0.0);
    for (int i = 0; i < outLen; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m_filterLen && (i - j) >= 0; ++j) {
            sum += m_filter[j] * out[i - j];
        }
        filtered[i] = sum;
    }

    return filtered;
}

/**
 * @brief 多相下采样
 * @param in 输入信号
 * @param factor 下采样因子
 * @return 滤波并下采样后的信号
 */
QVector<double> SampleRateConv4::polyphaseDown(const QVector<double>& in, int factor)
{
    int outLen = (in.size() + factor - 1) / factor;
    QVector<double> out(outLen, 0.0);

    for (int i = 0; i < outLen; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m_filterLen && (i * factor - j) >= 0; ++j) {
            sum += m_filter[j] * in[i * factor - j];
        }
        out[i] = sum;
    }

    return out;
}
