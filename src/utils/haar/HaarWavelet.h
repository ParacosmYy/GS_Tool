/**
 * @file HaarWavelet.h
 * @brief Haar小波变换 — 最简正交小波
 *
 * 功能: 实现Haar小波正变换/逆变换，支持多级分解与重构，
 *       统计变换次数/分解级数/耗时。
 */
#ifndef HAARWAVELET_H
#define HAARWAVELET_H

#include <QObject>
#include <QVector>

class HaarWavelet : public QObject {
    Q_OBJECT
public:
    /** 变换统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        quint64 totalInverseTransforms = 0;
        quint64 totalLevelsProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    /** @brief 多级分解结果 */
    struct Decomposition {
        QVector<double> approximation;   ///< 最终近似系数
        QVector<QVector<double>> details; ///< 各级细节系数
        int levels = 0;                   ///< 分解级数
    };

    explicit HaarWavelet(QObject* parent = nullptr);

    /** @brief 正变换(单级) @param data 输入数据(长度须为偶数) @return {近似,细节} */
    QPair<QVector<double>, QVector<double>> forward(
        const QVector<double>& data);

    /** @brief 逆变换(单级) @param approx 近似系数 @param detail 细节系数 @return 重构数据 */
    QVector<double> inverse(const QVector<double>& approx,
                            const QVector<double>& detail);

    /** @brief 多级分解 @param data 输入数据 @param levels 级数 @return 分解结果 */
    Decomposition decompose(const QVector<double>& data, int levels);

    /** @brief 多级重构 @param decomp 分解结果 @return 重构数据 */
    QVector<double> reconstruct(const Decomposition& decomp);

    /** @brief 去噪(阈值法) @param data 输入 @param levels 分解级数 @param threshold 阈值 @return 去噪数据 */
    QVector<double> denoise(const QVector<double>& data,
                            int levels, double threshold);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int inputSize, int levels);
    void inverseCompleted(int outputSize);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // HAARWAVELET_H
