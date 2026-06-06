/**
 * @file DiscreteSineTransform.h
 * @brief 离散正弦变换(DST-I/DST-VII) — Discrete Sine Transform Types I and VII with Odd-Length Fast Algorithm
 *
 * 功能: 实现DST-I和DST-VII正/逆变换，支持奇数长度快速算法。
 *       DST-I基于对称延拓可转换为DCT计算；DST-VII用于HEVC/AAC编解码。
 *       提供频谱分析和信号重构能力。
 *
 * 协作: DiscreteCosineTransform(DCT) / HaarWaveletTransform(Haar小波) / FftEngine(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散正弦变换处理器
 */
class DiscreteSineTransform : public QObject {
    Q_OBJECT

public:
    /** @brief DST类型 */
    enum Type { DST_I = 1, DST_VII = 7 };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalForward = 0;           ///< 累计正变换次数
        quint64 totalInverse = 0;           ///< 累计逆变换次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double lastPeakFrequency = 0.0;     ///< 最近峰值频率位置
    };

    explicit DiscreteSineTransform(QObject* parent = nullptr);
    ~DiscreteSineTransform() override;

    /** @brief 设置变换类型 */
    void setTransformType(Type type);

    /**
     * @brief DST正变换
     * @param signal 输入信号
     * @return DST系数
     */
    QVector<double> forward(const QVector<double>& signal);

    /**
     * @brief DST逆变换
     * @param coeffs DST系数
     * @param originalLength 原始长度
     * @return 重构信号
     */
    QVector<double> inverse(const QVector<double>& coeffs, int originalLength);

    /**
     * @brief 查找频谱峰值位置
     * @param coeffs DST系数
     * @return 峰值索引
     */
    int findPeak(const QVector<double>& coeffs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 正变换完成 @param length 信号长度 */
    void forwardCompleted(int length);
    /** @brief 逆变换完成 @param length 信号长度 */
    void inverseCompleted(int length);

private:
    /** @brief DST-I核心计算(通过DCT嵌入) */
    void computeDST1(QVector<double>& data) const;

    /** @brief DST-I逆变换 */
    void computeInverseDST1(QVector<double>& data) const;

    /** @brief DST-VII核心计算(矩阵乘法+快速路径) */
    void computeDST7(QVector<double>& data) const;

    /** @brief DST-VII逆变换(DST-VIII) */
    void computeInverseDST7(QVector<double>& data) const;

    /** @brief 补零到合适长度 */
    static int nextFastLength(int n);

    Type m_type = DST_I;

    Stats m_stats;
    double m_timeSum = 0.0;
};
