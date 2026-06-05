/**
 * @file ButterflyOperation.h
 * @brief 蝶形运算 — FFT核心计算单元
 *
 * 功能: 实现FFT/IFFT的蝶形运算和比特逆序，
 *       支持任意2的幂次长度的变换。
 *
 * 协作: FftEngine(FFT引擎) / GoertzelSpectrum(Goertzel算法)
 */
#ifndef BUTTERFLYOPERATION_H
#define BUTTERFLYOPERATION_H

#include <QObject>
#include <QVector>
#include <complex>

/**
 * @brief FFT蝶形运算器
 */
class ButterflyOperation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalTransforms = 0;       ///< 累计变换次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit ButterflyOperation(QObject* parent = nullptr);

    /**
     * @brief FFT/IFFT蝶形运算
     * @param data 输入数据(长度须为2的幂)
     * @param inverse true为IFFT，false为FFT
     * @return 变换结果
     */
    QVector<std::complex<double>> compute(
        const QVector<std::complex<double>>& data, bool inverse = false);

    /**
     * @brief 比特逆序重排
     * @param data 输入数据
     * @return 重排后的数据
     */
    QVector<std::complex<double>> bitReverse(
        const QVector<std::complex<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param size 数据长度 */
    void transformCompleted(int size);

private:
    /** @brief 计算比特逆序索引 */
    int reverseBits(int x, int log2n) const;

    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // BUTTERFLYOPERATION_H
