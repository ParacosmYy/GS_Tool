/**
 * @file TurboInterleaver.h
 * @brief Turbo码交织器 — S-random与UMTS交织模式生成
 *
 * 支持生成交织/解交织置换表, 包括经典S-random约束交织器和
 * UMTS/Turbo码标准交织器。用于Turbo码编译码器的信道交织。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class TurboInterleaver
 * @brief Turbo码交织器 — 生成满足约束条件的置换表
 *
 * 支持两种交织模式:
 * - S-random: 经典随机交织, 满足最小距离S约束
 * - UMTS: 3GPP标准交织器, 基于素数分解+内排列
 */
class TurboInterleaver : public QObject
{
    Q_OBJECT

public:
    /** @brief 交织模式 */
    enum class Mode {
        SRandom,   ///< S-random约束交织
        UMTS       ///< UMTS/Turbo码标准交织
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalGenerations = 0;      ///< 总生成次数
        quint64 totalElementsProcessed = 0; ///< 总处理元素数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit TurboInterleaver(QObject* parent = nullptr);

    /**
     * @brief 生成S-random交织置换表
     * @param length 交织长度N
     * @param spread 最小距离约束S(典型值 sqrt(N/2))
     * @return 置换表 perm[i]=j 表示位置i映射到位置j
     */
    QVector<int> generateSRandom(int length, int spread) const;

    /**
     * @brief 生成UMTS标准交织置换表
     * @param length 帧长(必须为40~5114之间的合法值)
     * @return 置换表
     */
    QVector<int> generateUMTS(int length) const;

    /**
     * @brief 生成解交织表(置换表逆映射)
     * @param interleaver 交织置换表
     * @return 解交织表
     */
    QVector<int> generateDeinterleaver(const QVector<int>& interleaver) const;

    /**
     * @brief 应用置换表到数据
     * @param data 输入数据
     * @param permutation 置换表
     * @return 置换后数据
     */
    QVector<double> applyPermutation(const QVector<double>& data,
                                     const QVector<int>& permutation) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 交织生成完成 @param length 交织长度 @param mode 交织模式 */
    void generationCompleted(int length, Mode mode);

private:
    /** @brief UMTS素数表(R, 辅助素数序列) */
    static const QVector<int> s_umtsPrimes;

    /** @brief UMTS内排列查找 @param length 帧长 @return 内排列参数对 */
    QPair<int, int> findUmtsParams(int length) const;

    /** @brief UMTS内排列生成 @param p 素数 @return 内排列向量 */
    QVector<int> generateIntraPermutation(int p) const;

    mutable Stats m_stats;  ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时(ms)
};
