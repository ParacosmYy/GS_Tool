/**
 * @file NeedlemanWunsch.h
 * @brief Needleman-Wunsch全局序列对齐引擎
 *
 * 实现经典的Needleman-Wunsch动态规划全局序列对齐算法,
 * 支持自定义匹配/失配/缺口罚分, 适用于DNA序列比对、
 * 字符串相似度分析、协议帧对齐等场景。
 */
#ifndef NEEDLEMANWUNSCH_H
#define NEEDLEMANWUNSCH_H

#include <QObject>
#include <QString>
#include <QPair>

/**
 * @class NeedlemanWunsch
 * @brief NW全局序列对齐 — 动态规划+回溯
 *
 * 典型用法:
 * @code
 *   NeedlemanWunsch nw;
 *   nw.setScores(1, -1, -2);
 *   auto result = nw.align("ACGT", "AGCT");
 *   int s = nw.score("ACGT", "AGCT");
 * @endcode
 */
class NeedlemanWunsch : public QObject {
    Q_OBJECT

public:
    /** @brief 对齐结果结构 */
    struct AlignmentResult {
        QString aligned1;     ///< 对齐后的序列1(含gap '-')
        QString aligned2;     ///< 对齐后的序列2(含gap '-')
        int     score = 0;    ///< 对齐得分
        int     matches = 0;  ///< 匹配数
        int     mismatches = 0; ///< 失配数
        int     gaps = 0;     ///< 缺口数
    };

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalAlignments = 0;   ///< 总对齐次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit NeedlemanWunsch(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~NeedlemanWunsch() override;

    // ── 配置 ──

    /**
     * @brief 设置评分参数
     * @param match 匹配得分(默认+1)
     * @param mismatch 失配罚分(默认-1)
     * @param gap 缺口罚分(默认-2)
     */
    void setScores(int match, int mismatch, int gap);

    // ── 核心接口 ──

    /**
     * @brief 执行全局对齐
     * @param seq1 序列1
     * @param seq2 序列2
     * @return 对齐结果(含对齐字符串和统计)
     */
    AlignmentResult align(const QString& seq1, const QString& seq2);

    /**
     * @brief 仅计算对齐得分(不回溯, 更快)
     * @param seq1 序列1
     * @param seq2 序列2
     * @return 对齐得分
     */
    int score(const QString& seq1, const QString& seq2);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 对齐完成信号 @param score 对齐得分 */
    void alignmentCompleted(int score);

private:
    /** @brief 匹配得分 */
    int m_matchScore = 1;

    /** @brief 失配罚分 */
    int m_mismatchScore = -1;

    /** @brief 缺口罚分 */
    int m_gapScore = -2;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // NEEDLEMANWUNSCH_H
