/**
 * @file CepstrumLifter.h
 * @brief 倒谱提升器 — 低通/高通倒滤波/Mel倒谱/倒谱距离
 *
 * 功能: 实现倒谱域滤波(提升)，包括低通倒滤波(频谱包络)、
 *       高通倒滤波(激励源)、倒谱距离度量和Mel倒谱分析。
 *
 * 协作: SpectrumAnalyzer(频谱) / SignalDecomposer(信号分解)
 */
#ifndef CEPSTRUMLIFTER_H
#define CEPSTRUMLIFTER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 倒谱提升处理器
 */
class CepstrumLifter : public QObject {
    Q_OBJECT

public:
    /** @brief 提升类型 */
    enum class LifterType {
        LowPass,        ///< 低通提升(保留频谱包络)
        HighPass,       ///< 高通提升(保留激励分量)
        BandPass        ///< 带通提升
    };
    Q_ENUM(LifterType)

    /** @brief 统计 */
    struct Stats {
        quint64 totalLiftings = 0;          ///< 累计提升次数
        quint64 totalSamplesProcessed = 0;  ///< 累计处理采样数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  peakCepstralDistance = 0.0; ///< 峰值倒谱距离
    };

    explicit CepstrumLifter(QObject* parent = nullptr);

    /** @brief 计算实倒谱 @param signal 输入信号 @return 倒谱序列 */
    QVector<double> computeRealCepstrum(const QVector<double>& signal);

    /** @brief 计算复倒谱 @param signal 输入信号 @return (实部, 虚部) */
    QPair<QVector<double>, QVector<double>> computeComplexCepstrum(
        const QVector<double>& signal);

    /** @brief 低通倒滤波(频谱包络) @param cepstrum 倒谱 @param cutoff 提升截止quefrency @return 提升后倒谱 */
    QVector<double> lowPassLifter(const QVector<double>& cepstrum, int cutoff);

    /** @brief 高通倒滤波(激励源) @param cepstrum 倒谱 @param cutoff 提升截止quefrency @return 提升后倒谱 */
    QVector<double> highPassLifter(const QVector<double>& cepstrum, int cutoff);

    /** @brief 带通倒滤波 @param cepstrum 倒谱 @param lowCut 低截止 @param highCut 高截止 @return 提升后倒谱 */
    QVector<double> bandPassLifter(const QVector<double>& cepstrum,
                                   int lowCut, int highCut);

    /** @brief 倒谱距离 @param c1 倒谱1 @param c2 倒谱2 @return 距离值 */
    double cepstralDistance(const QVector<double>& c1,
                           const QVector<double>& c2) const;

    /** @brief 计算Mel倒谱系数(MFCC) @param signal 输入信号 @param numCoeffs 系数数 @return MFCC系数 */
    QVector<double> melCepstrum(const QVector<double>& signal, int numCoeffs = 13);

    /** @brief 从倒谱重建信号 @param cepstrum 倒谱 @return 重建信号 */
    QVector<double> reconstructSignal(const QVector<double>& cepstrum);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 倒谱计算完成 @param cepstrumLength 倒谱长度 */
    void cepstrumComputed(int cepstrumLength);

    /** @brief 提升完成 @param type 提升类型 @param cutoff 截止quefrency */
    void lifteringComplete(int type, int cutoff);

private:
    void fft(QVector<double>& re, QVector<double>& im);
    void ifft(QVector<double>& re, QVector<double>& im);
    QVector<double> generateMelFilterBank(int fftSize, int numFilters,
                                          double sampleRate) const;
    double hzToMel(double hz) const;
    double melToHz(double mel) const;

    double m_sampleRate;            ///< 采样率

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

#endif // CEPSTRUMLIFTER_H
