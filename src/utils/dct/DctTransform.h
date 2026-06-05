/**
 * @file DctTransform.h
 * @brief DCT(离散余弦变换) — 类型II(标准DCT)
 *
 * 功能: 实现标准DCT-II和逆DCT-IDCT变换。
 *       广泛用于图像/音频压缩(JPEG/MP3)和信号处理。
 *
 * 协作: FftEngine(FFT加速DCT) / MfccExtractor(MFCC特征)
 */
#ifndef DCTTRANSFORM_H
#define DCTTRANSFORM_H

#include <QObject>
#include <QVector>

/**
 * @brief DCT离散余弦变换
 */
class DctTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalForward = 0;        ///< 累计正变换次数
        quint64 totalInverse = 0;        ///< 累计逆变换次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit DctTransform(QObject* parent = nullptr);

    /** @brief 正向DCT-II变换
     *  @param input 输入信号
     *  @return DCT系数 */
    QVector<double> forward(const QVector<double>& input);

    /** @brief 逆DCT变换(IDCT)
     *  @param coefficients DCT系数
     *  @return 重建信号 */
    QVector<double> inverse(const QVector<double>& coefficients);

    /** @brief 计算DCT-II第k个系数
     *  @param input 输入信号
     *  @param k 系数索引
     *  @return 系数值 */
    double coefficient(const QVector<double>& input, int k) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 变换完成 @param size 信号长度 @param forward 是否正变换 */
    void transformCompleted(int size, bool forward);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // DCTTRANSFORM_H
