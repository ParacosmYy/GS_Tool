/**
 * @file SymmetricWavelet.h
 * @brief 对称小波变换 — 边界修正的离散小波变换
 *
 * 功能: 使用对称边界扩展的离散小波变换(DWT)，
 *       支持正变换、逆变换和阈值去噪。
 *
 * 协作: WaveletTransform(标准DWT) / DaubechiesWavelet(Daubechies小波)
 */
#ifndef SYMMETRICWAVELET_H
#define SYMMETRICWAVELET_H

#include <QObject>
#include <QVector>

/**
 * @brief 对称(边界修正)小波变换
 */
class SymmetricWavelet : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalTransforms = 0;      ///< 累计变换次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit SymmetricWavelet(QObject* parent = nullptr);

    /** @brief 正向小波变换
     *  @param signal 输入信号
     *  @return 小波系数(低频在前，高频在后) */
    QVector<double> forward(const QVector<double>& signal);

    /** @brief 逆小波变换
     *  @param coeffs 小波系数
     *  @return 重建信号 */
    QVector<double> inverse(const QVector<double>& coeffs);

    /** @brief 阈值去噪
     *  @param signal 输入信号
     *  @param threshold 软阈值参数
     *  @return 去噪后信号 */
    QVector<double> denoise(const QVector<double>& signal, double threshold);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 变换完成 @param outputSize 输出长度 */
    void transformCompleted(int outputSize);

private:
    /** @brief 对称扩展卷积 @param signal 信号 @param filter 滤波器 @return 卷积结果 */
    QVector<double> symConvolve(const QVector<double>& signal,
                                const QVector<double>& filter) const;

    /** @brief 软阈值函数 @param x 输入 @param t 阈值 @return 阈值后结果 */
    double softThreshold(double x, double t) const;

    int m_originalSize; ///< 原始信号长度(用于逆变换)
    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // SYMMETRICWAVELET_H
