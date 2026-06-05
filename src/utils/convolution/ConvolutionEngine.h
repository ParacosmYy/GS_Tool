/**
 * @file ConvolutionEngine.h
 * @brief 卷积引擎 — 直接/FFT卷积与互相关
 *
 * 功能: 支持直接卷积和FFT加速卷积，可配置边界处理模式，
 *       提供卷积和相关两种运算。
 *
 * 协作: DigitalFilter(滤波卷积) / DataWindowManager(窗函数)
 */
#ifndef CONVOLUTIONENGINE_H
#define CONVOLUTIONENGINE_H

#include <QObject>
#include <QVector>

class ConvolutionEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 边界模式 */
    enum class BorderMode {
        Zero,           ///< 零填充
        Replicate,      ///< 复制边界
        Reflect,        ///< 镜像反射
        Wrap            ///< 循环填充
    };
    Q_ENUM(BorderMode)

    /** @brief 统计 */
    struct Stats {
        quint64 totalConvolutions = 0;
        quint64 totalPointsProcessed = 0;
        double  peakLatencyUs = 0.0;
        quint64 totalFftConvolutions = 0;
    };

    explicit ConvolutionEngine(QObject* parent = nullptr);

    void setBorderMode(BorderMode mode);

    /** @brief 直接卷积 @param signal 输入信号 @param kernel 卷积核 @return 结果 */
    QVector<double> convolve(const QVector<double>& signal,
                              const QVector<double>& kernel);

    /** @brief FFT加速卷积 @param signal 信号 @param kernel 核 @return 结果 */
    QVector<double> convolveFft(const QVector<double>& signal,
                                 const QVector<double>& kernel);

    /** @brief 互相关 @param signal 信号 @param kernel 核 @return 相关结果 */
    QVector<double> correlate(const QVector<double>& signal,
                               const QVector<double>& kernel);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void convolutionComplete(int outputSize);

private:
    double borderValue(const QVector<double>& data, int index) const;

    BorderMode m_borderMode;
    Stats m_stats;
};

#endif // CONVOLUTIONENGINE_H
