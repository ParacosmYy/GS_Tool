/**
 * @file StationaryWavelet.h
 * @brief 静态小波变换(Stationary Wavelet Transform)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class StationaryWavelet
 * @brief 静态小波变换(SWT) — 平移不变小波变换
 *
 * 支持多级分解/重构、Haar/DB2/DB4小波、信号去噪。
 * SWT是平移不变的, 适用于信号去噪和特征提取。
 */
class StationaryWavelet : public QObject
{
    Q_OBJECT

public:
    /** @brief 小波类型 */
    enum WaveletType {
        Haar,     /**< Haar小波 */
        DB2,      /**< Daubechies-2 */
        DB4       /**< Daubechies-4 */
    };
    Q_ENUM(WaveletType)

    /** @brief 分解结果 */
    struct Decomposition {
        QVector<double> approximation; /**< 近似系数 */
        QVector<QVector<double>> details; /**< 各级细节系数 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalDecomposed = 0;  /**< 总分解次数 */
        int totalReconstructed = 0; /**< 总重构次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit StationaryWavelet(QObject* parent = nullptr);

    /**
     * @brief 多级SWT分解
     * @param signal 输入信号(长度必须为2^level的倍数)
     * @param level 分解级数
     * @param type 小波类型
     * @return 分解结果
     */
    Decomposition decompose(const QVector<double>& signal,
                             int level, WaveletType type = Haar);

    /**
     * @brief SWT重构
     * @param decomp 分解结果
     * @param type 小波类型
     * @return 重构信号
     */
    QVector<double> reconstruct(const Decomposition& decomp,
                                 WaveletType type = Haar);

    /**
     * @brief SWT软阈值去噪
     * @param signal 输入信号
     * @param level 分解级数
     * @param threshold 阈值
     * @param type 小波类型
     * @return 去噪后信号
     */
    QVector<double> denoise(const QVector<double>& signal,
                              int level, double threshold,
                              WaveletType type = Haar);

    /**
     * @brief 估计通用阈值(VisuShrink)
     * @param detailCoeffs 细节系数
     * @return 估计阈值
     */
    static double universalThreshold(const QVector<double>& detailCoeffs);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成信号 */
    void decomposed(int level, int length);

private:
    void getFilters(WaveletType type, QVector<double>& lo, QVector<double>& hi) const;

    Stats m_stats;
    double m_timeSum;
};
