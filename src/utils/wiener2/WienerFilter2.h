/**
 * @file WienerFilter2.h
 * @brief 频域维纳滤波器 — 信号去噪
 *
 * 功能: 基于信号功率和噪声功率估计，在频域构建最优滤波器，
 *       实现最小均方误差意义下的信号去噪。支持自动功率谱估计
 *       和手动指定功率谱两种模式。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DigitalFilter(时域滤波)
 */
#ifndef WIENERFILTER2_H
#define WIENERFILTER2_H

#include <QObject>
#include <QVector>

/**
 * @brief 频域维纳滤波器 — 信号去噪
 */
class WienerFilter2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDesigns = 0;          ///< 累计设计次数
        quint64 totalApplications = 0;     ///< 累计应用次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit WienerFilter2(QObject* parent = nullptr);

    /** @brief 设计维纳滤波器
     *  @param signalPower 信号功率谱
     *  @param noisePower  噪声功率谱
     *  @return 滤波器频率响应(增益数组) */
    QVector<double> design(const QVector<double>& signalPower,
                           const QVector<double>& noisePower);

    /** @brief 应用维纳滤波器去噪
     *  @param signal 输入信号
     *  @return 去噪后信号 */
    QVector<double> apply(const QVector<double>& signal);

    /** @brief 设置正则化参数
     *  @param lambda 正则化系数(防止除零) */
    void setRegularization(double lambda);

    /** @brief 获取当前滤波器频率响应 @return 频率响应 */
    QVector<double> frequencyResponse() const { return m_response; }

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 滤波器设计完成 @param length 滤波器长度 */
    void designCompleted(int length);

    /** @brief 滤波应用完成 @param inputSize 输入信号长度 */
    void applicationCompleted(int inputSize);

private:
    /** @brief 基2 FFT(就地) @param real 实部 @param imag 虚部 @param inverse 是否逆变换 */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    /** @brief 估计功率谱 @param signal 信号 @return 功率谱 */
    QVector<double> estimatePsd(const QVector<double>& signal) const;

    QVector<double> m_response;     ///< 当前频率响应
    double m_regularization;        ///< 正则化参数
    double m_timeSum;               ///< 处理时间累加器
    Stats  m_stats;                 ///< 统计信息
};

#endif // WIENERFILTER2_H
