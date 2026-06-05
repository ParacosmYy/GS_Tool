/**
 * @file HaarWavelet3D.h
 * @brief 三维Haar小波变换 — 体数据分析/压缩/去噪
 *
 * 功能: 对三维体数据执行Haar小波正/逆变换，支持多尺度分解与重构，
 *       提供阈值去噪功能，适用于3D传感器数据、体积渲染预处理等场景。
 *
 * 协作: DataCompressor(数据压缩) / SpectrumAnalyzer(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三维Haar小波变换引擎
 *
 * 典型用法:
 * @code
 *   HaarWavelet3D wavelet;
 *   wavelet.setData(volume, nx, ny, nz);
 *   auto coeffs = wavelet.forward();
 *   wavelet.threshold(0.1);
 *   auto reconstructed = wavelet.inverse();
 * @endcode
 */
class HaarWavelet3D : public QObject {
    Q_OBJECT

public:
    /** @brief 小波系数结构 */
    struct Coefficients {
        QVector<double> approximation;     ///< 低频近似分量
        QVector<double> detailX;           ///< X方向细节
        QVector<double> detailY;           ///< Y方向细节
        QVector<double> detailZ;           ///< Z方向细节
        QVector<double> detailXY;          ///< XY方向细节
        QVector<double> detailXZ;          ///< XZ方向细节
        QVector<double> detailYZ;          ///< YZ方向细节
        int nx = 0, ny = 0, nz = 0;       ///< 尺寸
        int levels = 0;                    ///< 分解层数
    };

    /** @brief 统计数据 */
    struct Stats {
        int totalTransforms = 0;                ///< 累计变换次数
        int totalInverseTransforms = 0;         ///< 累计逆变换次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
        int totalCoefficientsThresholded = 0;   ///< 阈值化系数数
        qint64 totalVoxelsProcessed = 0;        ///< 累计体素数
    };

    explicit HaarWavelet3D(QObject* parent = nullptr);

    /**
     * @brief 设置输入体数据
     * @param data 展平的3D数据(行优先)
     * @param nx X方向尺寸
     * @param ny Y方向尺寸
     * @param nz Z方向尺寸
     */
    void setData(const QVector<double>& data, int nx, int ny, int nz);

    /**
     * @brief 执行前向Haar小波变换(多尺度)
     * @param levels 分解层数(0表示自动计算最大层数)
     * @return 小波系数
     */
    Coefficients forward(int levels = 0);

    /**
     * @brief 执行逆Haar小波变换
     * @param coeffs 小波系数
     * @return 重构的体数据
     */
    QVector<double> inverse(const Coefficients& coeffs);

    /**
     * @brief 对系数执行硬阈值去噪
     * @param coeffs 系数(会被原地修改)
     * @param threshold 阈值
     * @return 被置零的系数个数
     */
    int applyHardThreshold(Coefficients& coeffs, double threshold) const;

    /**
     * @brief 对系数执行软阈值去噪
     * @param coeffs 系数(会被原地修改)
     * @param threshold 阈值
     * @return 被修改的系数个数
     */
    int applySoftThreshold(Coefficients& coeffs, double threshold) const;

    /**
     * @brief 计算系数能量分布
     * @param coeffs 小波系数
     @return 各频带能量占比(共7个: approx, dx, dy, dz, dxy, dxz, dyz)
     */
    QVector<double> energyDistribution(const Coefficients& coeffs) const;

    /** @brief 获取统计 @return 统计数据 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 前向变换完成 @param levels 层数 @param elapsedMs 耗时 */
    void forwardCompleted(int levels, double elapsedMs);

    /** @brief 逆变换完成 @param voxelCount 体素数 @param elapsedMs 耗时 */
    void inverseCompleted(qint64 voxelCount, double elapsedMs);

private:
    /** @brief 单层1D Haar变换(前向) */
    static void haar1DForward(QVector<double>& data, int n);

    /** @brief 单层1D Haar变换(逆向) */
    static void haar1DInverse(QVector<double>& data, int n);

    /** @brief 单层3D Haar变换(前向) */
    void haar3DForward(Coefficients& coeffs);

    /** @brief 单层3D Haar变换(逆向) */
    void haar3DInverse(Coefficients& coeffs);

    /** @brief 计算向量能量(平方和) */
    static double vectorEnergy(const QVector<double>& v);

    int m_nx = 0;                  ///< X方向尺寸
    int m_ny = 0;                  ///< Y方向尺寸
    int m_nz = 0;                  ///< Z方向尺寸
    QVector<double> m_data;        ///< 输入数据缓存
    Stats m_stats;                 ///< 统计数据
    double m_timeSum = 0.0;        ///< 时间累加器
};
