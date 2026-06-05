/**
 * @file WindowFunction.h
 * @brief 窗函数库 — 20+种窗函数
 *
 * 功能: 提供二十余种常用窗函数的生成与应用，包括矩形窗、
 *       Hanning、Hamming、Blackman、Blackman-Harris、FlatTop、
 *       Kaiser、Gaussian、Bartlett、Nuttall、Tukey等。
 *       支持相干增益计算和窗函数应用。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / FftEngine(FFT)
 */
#ifndef WINDOWFUNCTION_H
#define WINDOWFUNCTION_H

#include <QObject>
#include <QVector>

/**
 * @brief 窗函数库 — 20+种窗函数
 */
class WindowFunction : public QObject {
    Q_OBJECT

public:
    /** @brief 窗函数类型 */
    enum class Type {
        Rectangular, Hanning, Hamming, Blackman, BlackmanHarris,
        FlatTop, Kaiser, Gaussian, Bartlett, Nuttall, Tukey,
        Bohman, Parzen, Welch, Cosine, Exponential, Taylor,
        Chebyshev, HannPoisson, Poisson, Lanczos
    };
    Q_ENUM(Type)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalApplications = 0;     ///< 累计应用次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit WindowFunction(QObject* parent = nullptr);

    /** @brief 创建窗函数系数
     *  @param type 窗函数类型
     *  @param length 窗长度
     *  @return 窗系数 */
    QVector<double> create(Type type, int length);

    /** @brief 将窗函数应用到信号
     *  @param signal 输入信号
     *  @param type 窗函数类型
     *  @return 加窗后信号 */
    QVector<double> apply(const QVector<double>& signal, Type type);

    /** @brief 计算窗函数的相干增益
     *  @param type 窗函数类型
     *  @param length 窗长度
     *  @return 相干增益(窗系数均值) */
    double coherentGain(Type type, int length);

    /** @brief 设置Kaiser窗beta参数 @param beta beta值 */
    void setKaiserBeta(double beta);

    /** @brief 设置Gaussian窗sigma参数 @param sigma sigma值 */
    void setGaussianSigma(double sigma);

    /** @brief 设置Tukey窗alpha参数 @param alpha alpha值 */
    void setTukeyAlpha(double alpha);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 窗应用完成 @param type 窗类型 @param length 窗长度 */
    void applicationCompleted(int type, int length);

private:
    double m_kaiserBeta;     ///< Kaiser窗beta参数
    double m_gaussianSigma;  ///< Gaussian窗sigma参数
    double m_tukeyAlpha;     ///< Tukey窗alpha参数
    double m_timeSum;        ///< 处理时间累加器
    Stats  m_stats;          ///< 统计信息
};

#endif // WINDOWFUNCTION_H
