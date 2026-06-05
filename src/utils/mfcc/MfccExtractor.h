/**
 * @file MfccExtractor.h
 * @brief MFCC特征提取 — 梅尔频率倒谱系数
 *
 * 功能: 从音频信号中提取梅尔频率倒谱系数(MFCC)，广泛应用于
 *       语音识别、声纹识别和音频分类。支持可调滤波器组数量、
 *       FFT大小和倒谱系数数量。
 *
 * 协作: SpectralFeatures(频谱特征) / PitchDetector2(音高检测)
 */
#ifndef MFFCEXTRACTOR_H
#define MFFCEXTRACTOR_H

#include <QObject>
#include <QVector>

/**
 * @brief MFCC特征提取 — 梅尔频率倒谱系数
 */
class MfccExtractor : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalExtractions = 0;      ///< 累计提取次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit MfccExtractor(QObject* parent = nullptr);

    /** @brief 提取MFCC特征
     *  @param signal 输入信号
     *  @param sampleRate 采样率(Hz)
     *  @param numCoeffs 倒谱系数数量(默认13)
     *  @return MFCC系数向量 */
    QVector<double> extract(const QVector<double>& signal,
                            double sampleRate = 48000.0,
                            int numCoeffs = 13);

    /** @brief 生成梅尔滤波器组
     *  @param numFilters 滤波器数量(默认26)
     *  @param fftSize FFT大小
     *  @param sampleRate 采样率
     *  @return 滤波器组矩阵(numFilters x fftSize/2+1) */
    QVector<QVector<double>> melFilterbank(int numFilters = 26,
                                           int fftSize = 512,
                                           double sampleRate = 48000.0);

    /** @brief 设置FFT大小 @param size FFT大小(2的幂) */
    void setFftSize(int size);

    /** @brief 设置梅尔滤波器数量 @param num 滤波器数量 */
    void setNumFilters(int num);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 特征提取完成 @param numCoeffs 系数数量 */
    void extractionCompleted(int numCoeffs);

private:
    /** @brief Hz转Mel @param hz 频率(Hz) @return Mel值 */
    static double hzToMel(double hz);

    /** @brief Mel转Hz @param mel Mel值 @return 频率(Hz) */
    static double melToHz(double mel);

    /** @brief 基2 FFT(就地) @param real 实部 @param imag 虚部 @param inverse 逆变换 */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    /** @brief 离散余弦变换(DCT-II) @param input 输入 @param numCoeffs 系数数 @return DCT结果 */
    static QVector<double> dct(const QVector<double>& input, int numCoeffs);

    int m_fftSize;          ///< FFT大小
    int m_numFilters;       ///< 梅尔滤波器数量
    double m_timeSum;       ///< 处理时间累加器
    Stats  m_stats;         ///< 统计信息
};

#endif // MFFCEXTRACTOR_H
