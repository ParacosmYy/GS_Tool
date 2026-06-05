/**
 * @file DataResampler.h
 * @brief 数据重采样引擎 — 线性/三次/Sinc/最近邻插值重采样
 *
 * 功能: 4种插值方法改变数据采样率，支持指定输出长度，
 *       适用于波形缩放和采样率转换。
 *
 * 协作: DataReducer(降采样) / FftEngine(频谱重采样)
 */
#ifndef DATARESAMPLER_H
#define DATARESAMPLER_H

#include <QObject>
#include <QVector>

class DataResampler : public QObject {
    Q_OBJECT

public:
    /** @brief 重采样方法 */
    enum class ResampleMethod {
        Nearest,    ///< 最近邻插值
        Linear,     ///< 线性插值
        Cubic,      ///< 三次样条插值(Catmull-Rom)
        Sinc        ///< Sinc插值(Lanczos窗)
    };
    Q_ENUM(ResampleMethod)

    /** @brief 重采样配置 */
    struct ResampleConfig {
        double sourceRate = 1000.0;     ///< 源采样率
        double targetRate = 2000.0;     ///< 目标采样率
        int sincTaps = 16;             ///< Sinc插值抽头数
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalSamplesProcessed = 0;     ///< 累计处理采样数
        quint64 totalResamples = 0;            ///< 累计重采样次数
        double  peakLatencyUs = 0.0;           ///< 峰值延迟(us)
        quint64 totalOutputSamples = 0;        ///< 累计输出采样数
    };

    explicit DataResampler(QObject* parent = nullptr);

    void setMethod(ResampleMethod method);
    void setConfig(const ResampleConfig& config);

    /** @brief 重采样 @param data 输入数据 @return 重采样数据 */
    QVector<double> resample(const QVector<double>& data);

    /** @brief 重采样到指定长度 @param data 数据 @param targetSize 目标长度 @return 重采样数据 */
    QVector<double> resampleToSize(const QVector<double>& data, int targetSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resampled(int inputCount, int outputCount);

private:
    double interpolateNearest(const QVector<double>& data, double idx) const;
    double interpolateLinear(const QVector<double>& data, double idx) const;
    double interpolateCubic(const QVector<double>& data, double idx) const;
    double interpolateSinc(const QVector<double>& data, double idx) const;

    static double sinc(double x);

    ResampleMethod m_method;
    ResampleConfig m_config;
    Stats m_stats;
};

#endif // DATARESAMPLER_H
