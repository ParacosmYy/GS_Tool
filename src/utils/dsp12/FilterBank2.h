/**
 * @file FilterBank2.h
 * @brief 二进滤波器组 — 小波包分解
 *
 * 功能: 实现二进 (Dyadic) 滤波器组，用于小波包分解，
 *       支持多级分解与重构、能量特征提取、子带分析。
 *       内置 Haar 和 DB2 (Daubechies-2) 滤波器。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / SignalDecomposer(信号分解)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 二进滤波器组引擎
 *
 * 二进滤波器组通过递归地将信号分解为低频 (近似) 和高频 (细节)
 * 子带，实现多分辨率分析。每级分解将频带一分为二，共 J 级分解
 * 产生 2^J 个子带。
 */
class FilterBank2 : public QObject {
    Q_OBJECT

public:
    /** @brief 滤波器类型 */
    enum class FilterType {
        Haar,      ///< Haar 小波 (长度 2，最简单)
        DB2,       ///< Daubechies-2 (长度 4)
        DB4        ///< Daubechies-4 (长度 8)
    };
    Q_ENUM(FilterType)

    /** @brief 子带信息 */
    struct Subband {
        int level = 0;              ///< 分解层级
        int index = 0;              ///< 子带索引 (在该层级内)
        double energy = 0.0;        ///< 子带能量
        double ratio = 0.0;         ///< 能量占比
        QVector<double> samples;    ///< 子带样本
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalDecompositions = 0;       ///< 累计分解次数
        int totalReconstructions = 0;      ///< 累计重构次数
        int totalLevelsProcessed = 0;      ///< 累计处理层级数
        int totalSamplesProcessed = 0;     ///< 累计处理采样数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit FilterBank2(QObject* parent = nullptr);

    /**
     * @brief 设置滤波器类型
     * @param type 滤波器类型
     */
    void setFilterType(FilterType type);

    /**
     * @brief 多级分解
     * @param data 输入信号
     * @param levels 分解层级数
     * @return 各级子带列表 (从第 1 级到第 levels 级)
     */
    QList<Subband> decompose(const QVector<double>& data, int levels);

    /**
     * @brief 从分解结果重构信号
     * @param subbands 分解后的子带列表
     * @param originalLength 原始信号长度
     * @return 重构后的信号
     */
    QVector<double> reconstruct(const QList<Subband>& subbands,
                                int originalLength);

    /**
     * @brief 提取指定层级的子带能量特征
     * @param subbands 分解结果
     * @return 各子带能量向量
     */
    QVector<double> energyFeatures(const QList<Subband>& subbands) const;

    /**
     * @brief 获取当前滤波器的低通分解系数
     * @return 低通滤波器系数
     */
    QVector<double> lowpassCoefficients() const;

    /**
     * @brief 获取当前滤波器的高通分解系数
     * @return 高通滤波器系数
     */
    QVector<double> highpassCoefficients() const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 分解完成 @param levels 分解层级数 @param subbandCount 子带总数 */
    void decompositionComplete(int levels, int subbandCount);

    /** @brief 重构完成 @param length 重构信号长度 */
    void reconstructionComplete(int length);

private:
    void initFilterCoeffs();
    QVector<double> downsampleFilter(const QVector<double>& data,
                                     const QVector<double>& filter) const;
    QVector<double> upsampleFilter(const QVector<double>& data,
                                   const QVector<double>& filter,
                                   int outputLength) const;
    double computeEnergy(const QVector<double>& data) const;

    FilterType m_filterType;      ///< 当前滤波器类型
    QVector<double> m_lowDec;     ///< 低通分解系数
    QVector<double> m_highDec;    ///< 高通分解系数
    QVector<double> m_lowRec;     ///< 低通重构系数
    QVector<double> m_highRec;    ///< 高通重构系数

    Stats m_stats;                ///< 统计信息
    double m_timeSum = 0.0;       ///< 处理时间累加器
};
