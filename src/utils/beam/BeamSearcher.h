/**
 * @file BeamSearcher.h
 * @brief Beam Search束搜索解码器
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <functional>

/**
 * @class BeamSearcher
 * @brief 束搜索算法，用于序列解码/路径搜索中的近似最优解
 *
 * 维护固定宽度的候选序列集合，每步扩展所有候选并保留得分最高的beamWidth个。
 * 支持自定义得分函数和终止条件。
 */
class BeamSearcher : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 候选路径结构
     */
    struct Candidate {
        QVector<int> tokens;   /**< 当前token序列 */
        double score;          /**< 累积得分 */
    };

    /**
     * @brief 搜索结果
     */
    struct Result {
        QVector<Candidate> beams; /**< 最终beam集合(按得分降序) */
        int stepsExpanded;        /**< 总扩展步数 */
        double bestScore;         /**< 最高得分 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;       /**< 总搜索次数 */
        int totalCandidates = 0;     /**< 总候选数 */
        int avgStepsExpanded = 0;    /**< 平均扩展步数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param beamWidth 束宽度(默认5)
     * @param parent 父对象
     */
    explicit BeamSearcher(int beamWidth = 5, QObject* parent = nullptr);

    /** @brief 设置束宽度 */
    void setBeamWidth(int width);

    /**
     * @brief 执行束搜索
     * @param scoreFn 得分函数: (tokenSequence, newToken) → 得分
     * @param vocabSize 词汇表大小(候选token范围0~vocabSize-1)
     * @param maxSteps 最大扩展步数
     * @param eosToken 终止token(遇到则停止该候选扩展)
     * @return 搜索结果
     */
    Result search(
        std::function<double(const QVector<int>&, int)> scoreFn,
        int vocabSize,
        int maxSteps,
        int eosToken = -1);

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 */
    void searchCompleted(int bestBeamSize, double bestScore);

private:
    int m_beamWidth;       /**< 束宽度 */
    Stats m_stats;         /**< 统计信息 */
    double m_timeSum;      /**< 累计时间 */
};
