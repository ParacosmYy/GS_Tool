/**
 * @file RenyiEntropy.h
 * @brief Rényi熵与Tsallis熵 — 广义信息熵度量
 *
 * 功能: 计算Rényi熵(Shannon熵的推广，参数alpha控制权重)
 *       和Tsallis熵(非广延统计力学的熵度量)，
 *       用于信号复杂度分析和异常检测。
 *
 * 协作: EntropyCalculator(熵计算) / DataQualityScorer(质量评估)
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMap>

#include <cmath>

/**
 * @brief Rényi熵与Tsallis熵计算器
 */
class RenyiEntropy : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations      = 0;   ///< 累计计算次数
        quint64 totalBytesProcessed    = 0;   ///< 累计处理字节数
        double  avgProcessingTimeMs    = 0.0; ///< 平均处理时间(ms)
        double  lastRenyiEntropy       = 0.0; ///< 最近Rényi熵
        double  lastTsallisEntropy     = 0.0; ///< 最近Tsallis熵
    };

    explicit RenyiEntropy(QObject* parent = nullptr);

    /**
     * @brief 计算Rényi熵(字节数据)
     * @param data 输入数据
     * @param alpha 阶数(>0, alpha=1退化为Shannon熵)
     * @return Rényi熵值
     */
    double renyiEntropy(const QByteArray& data, double alpha = 2.0);

    /**
     * @brief 计算Rényi熵(符号序列)
     * @param symbols 符号列表
     * @param alpha 阶数
     * @return Rényi熵值
     */
    double renyiEntropySymbols(const QVector<int>& symbols, double alpha = 2.0);

    /**
     * @brief 计算Tsallis熵(字节数据)
     * @param data 输入数据
     * @param q 非广延参数(≠1, q→1退化为Shannon熵)
     * @return Tsallis熵值
     */
    double tsallisEntropy(const QByteArray& data, double q = 2.0);

    /**
     * @brief 计算Tsallis熵(符号序列)
     * @param symbols 符号列表
     * @param q 非广延参数
     * @return Tsallis熵值
     */
    double tsallisEntropySymbols(const QVector<int>& symbols, double q = 2.0);

    /**
     * @brief 计算Rényi熵谱(多个alpha值)
     * @param data 输入数据
     * @param alphaMin 最小alpha
     * @param alphaMax 最大alpha
     * @param steps 步数
     * @return (alpha列表, 熵值列表)
     */
    QPair<QVector<double>, QVector<double>> renyiSpectrum(
        const QByteArray& data, double alphaMin = 0.1,
        double alphaMax = 10.0, int steps = 50);

    /**
     * @brief 计算碰撞熵(alpha=2的特殊情况)
     * @param data 输入数据
     * @return 碰撞熵
     */
    double collisionEntropy(const QByteArray& data);

    /**
     * @brief 计算最小熵(alpha→∞的极限)
     * @param data 输入数据
     * @return 最小熵
     */
    double minEntropy(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief Rényi熵计算完成 @param value 熵值 @param alpha 阶数 */
    void renyiComputed(double value, double alpha);

    /** @brief Tsallis熵计算完成 @param value 熵值 @param q 参数 */
    void tsallisComputed(double value, double q);

    /** @brief 熵谱计算完成 @param steps 步数 */
    void spectrumComputed(int steps);

private:
    /**
     * @brief 计算字节频率分布
     * @param data 输入数据
     * @return 概率分布
     */
    QMap<int, double> byteDistribution(const QByteArray& data) const;

    /**
     * @brief 计算符号频率分布
     * @param symbols 符号列表
     * @return 概率分布
     */
    QMap<int, double> symbolDistribution(const QVector<int>& symbols) const;

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
