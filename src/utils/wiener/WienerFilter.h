/**
 * @file WienerFilter.h
 * @brief 维纳反卷积滤波器 — 信号恢复与去模糊
 *
 * 功能: 基于频域的维纳滤波，利用信号和噪声功率谱密度(PDS)估计
 *       进行最优线性滤波，支持正则化参数调节，适用于信号恢复、
 *       去卷积和降噪场景。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DigitalFilter(时域滤波)
 */
#ifndef WIENERFILTER_H
#define WIENERFILTER_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 维纳反卷积滤波器 — 频域信号恢复
 */
class WienerFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFilterings = 0;        ///< 累计滤波次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit WienerFilter(QObject* parent = nullptr);

    /** @brief 维纳滤波
     *  @param signal    输入信号
     *  @param noisePsd  噪声功率谱密度
     *  @param signalPsd 信号功率谱密度
     *  @return 滤波后信号 */
    QVector<double> filter(const QVector<double>& signal,
                           const QVector<double>& noisePsd,
                           const QVector<double>& signalPsd);

    /** @brief 设置正则化参数(防止除零)
     *  @param lambda 正则化系数(默认1e-6) */
    void setRegularization(double lambda);

    /** @brief 频域维纳滤波(自动FFT)
     *  @param signal  输入信号
     *  @param fftSize FFT大小(0表示自动)
     *  @return 滤波后信号 */
    QVector<double> frequencyDomainFilter(const QVector<double>& signal,
                                          int fftSize = 0);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 滤波完成 @param inputSize 输入信号长度 */
    void filteringCompleted(int inputSize);

private:
    /** @brief 基2 FFT(就地) @param real 实部 @param imag 虚部 @param inverse 是否逆变换 */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    /** @brief 估计功率谱 @param signal 信号 @return 功率谱 */
    QVector<double> estimatePsd(const QVector<double>& signal) const;

    double m_regularization;       ///< 正则化参数
    double m_timeSum;              ///< 处理时间累加器

    mutable Stats m_stats;         ///< 可变统计(支持const方法)
};

#endif // WIENERFILTER_H
