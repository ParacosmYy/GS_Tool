/**
 * @file HaarWavelet2DEnhanced.h
 * @brief 二维Haar小波变换 — 信号/图像多分辨率分析
 *
 * 功能: 对二维数据(如传感器矩阵、图像帧)进行Haar小波
 *       正变换和逆变换，支持多级分解与重构。
 *
 * 协作: SpectrumAnalyzer(频域分析) / SignalDecomposer(信号分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 二维Haar小波变换
 */
class HaarWavelet2DEnhanced : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;      ///< 累计变换次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit HaarWavelet2DEnhanced(QObject* parent = nullptr);

    /**
     * @brief 正向Haar小波变换
     * @param image 二维输入数据(行/列须为2的幂)
     * @return 变换系数矩阵(与输入同尺寸)
     */
    QVector<QVector<double>> forward(const QVector<QVector<double>>& image);

    /**
     * @brief 逆向Haar小波变换
     * @param coeffs 变换系数矩阵
     * @return 重构后的二维数据
     */
    QVector<QVector<double>> inverse(const QVector<QVector<double>>& coeffs);

    /**
     * @brief 设置分解级数
     * @param levels 级数(默认1，最大log2(min(rows,cols)))
     */
    void setLevels(int levels);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param size 输出矩阵尺寸 */
    void transformCompleted(int size);

private:
    /** @brief 单级行变换 @param data 行数据 */
    void transformRows(QVector<QVector<double>>& data) const;

    /** @brief 单级列变换 @param data 列数据 */
    void transformCols(QVector<QVector<double>>& data) const;

    /** @brief 单级行逆变换 */
    void inverseRows(QVector<QVector<double>>& data) const;

    /** @brief 单级列逆变换 */
    void inverseCols(QVector<QVector<double>>& data) const;

    /** @brief 检查是否为2的幂 */
    static bool isPowerOfTwo(int n);

    /** @brief 向上取整到2的幂 */
    static int nextPowerOfTwo(int n);

    /** @brief 填充到2的幂尺寸 */
    static QVector<QVector<double>> padToPowerOfTwo(
        const QVector<QVector<double>>& data);

    int m_levels = 1;  ///< 分解级数

    mutable Stats        m_stats;
    mutable double       m_totalTimeMs = 0.0;
    mutable QElapsedTimer m_timer;
};
