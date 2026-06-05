/**
 * @file WaveletPacket.h
 * @brief 小波包分解与重构 — Haar/DB2/DB4
 *
 * 实现离散小波包变换(DWPT)，支持三种常用小波基:
 *   - Haar: 最简单的小波基，适合分段常数信号
 *   - DB2(Daubechies-2): 适合一般信号分析
 *   - DB4(Daubechies-4): 更高阶消失矩，适合光滑信号
 *
 * 协作: SpectrumAnalyzer(频谱分析) / SignalDecomposer(信号分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @class WaveletPacket
 * @brief 小波包分解与重构引擎
 *
 * 提供完全二叉树结构的小波包分解，可灵活选择分解层数
 * 和小波基函数，支持部分重构和能量谱分析。
 */
class WaveletPacket : public QObject
{
    Q_OBJECT

public:
    /** @brief 小波基类型 */
    enum class WaveletType {
        Haar,   ///< Haar小波
        DB2,    ///< Daubechies-2
        DB4     ///< Daubechies-4
    };
    Q_ENUM(WaveletType)

    /** @brief 分解节点 */
    struct PacketNode {
        int level = 0;                ///< 分解层级
        int index = 0;                ///< 节点索引(0=近似,1=细节)
        QVector<double> coefficients; ///< 小波系数
        double energy = 0.0;          ///< 节点能量
    };

    /** @brief 分解结果 */
    struct DecompositionResult {
        QVector<PacketNode> nodes;    ///< 所有分解节点
        int levels = 0;               ///< 分解层数
        int originalSize = 0;         ///< 原始信号长度
        WaveletType wavelet = WaveletType::Haar; ///< 使用的小波基
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0;   ///< 累计分解次数
        quint64 totalReconstructions = 0;  ///< 累计重构次数
        quint64 totalSamplesProcessed = 0; ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit WaveletPacket(QObject* parent = nullptr);

    /**
     * @brief 设置小波基类型
     * @param type 小波基
     */
    void setWaveletType(WaveletType type);

    /**
     * @brief 设置分解层数
     * @param levels 层数(1-10)
     */
    void setDecompositionLevels(int levels);

    /**
     * @brief 执行小波包分解
     * @param signal 输入信号(长度应为2^N)
     * @return 分解结果
     */
    DecompositionResult decompose(const QVector<double>& signal);

    /**
     * @brief 从分解结果重构信号
     * @param result 分解结果
     * @return 重构后的信号
     */
    QVector<double> reconstruct(const DecompositionResult& result);

    /**
     * @brief 单层小波分解
     * @param signal 输入信号
     * @return (近似系数, 细节系数)
     */
    QPair<QVector<double>, QVector<double>> singleLevelDecompose(
        const QVector<double>& signal);

    /**
     * @brief 单层小波重构
     * @param approx 近似系数
     * @param detail 细节系数
     * @return 重构信号
     */
    QVector<double> singleLevelReconstruct(
        const QVector<double>& approx,
        const QVector<double>& detail);

    /**
     * @brief 计算各节点能量分布
     * @param result 分解结果
     * @return 节点能量向量(按层级排列)
     */
    QVector<double> energySpectrum(const DecompositionResult& result) const;

    /**
     * @brief 阈值去噪
     * @param signal 输入信号
     * @param threshold 阈值(绝对值)
     * @return 去噪后信号
     */
    QVector<double> denoise(const QVector<double>& signal, double threshold);

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成 @param levels 层数 @param nodeCount 节点数 */
    void decompositionCompleted(int levels, int nodeCount);

    /** @brief 重构完成 @param size 信号长度 @param error 重构误差 */
    void reconstructionCompleted(int size, double error);

private:
    /** @brief 获取小波滤波器系数 */
    void getFilters(WaveletType type,
                    QVector<double>& lowDec, QVector<double>& highDec,
                    QVector<double>& lowRec, QVector<double>& highRec) const;

    /** @brief 卷积运算 */
    QVector<double> convolve(const QVector<double>& signal,
                             const QVector<double>& filter) const;

    /** @brief 计算节点能量 */
    double computeEnergy(const QVector<double>& coeffs) const;

    WaveletType m_waveletType = WaveletType::Haar; ///< 小波基
    int m_levels = 3;                               ///< 分解层数

    mutable Stats m_stats;         ///< 操作统计
    mutable double m_timeSum = 0.0;///< 累计耗时
};
