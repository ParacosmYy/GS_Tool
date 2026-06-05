/**
 * @file BitapMatcher.h
 * @brief Bitap近似字符串匹配引擎 — 位移掩码模糊搜索
 *
 * 功能: 实现Bitap(Shift-Or)算法，支持精确匹配和允许指定
 *       编辑距离的近似匹配，统计搜索/匹配次数及平均耗时。
 *
 * 协作: BytePatternAnalyzer(模式匹配) / DataPatternDetector(数据检测)
 */
#ifndef BITAPMATCHER_H
#define BITAPMATCHER_H

#include <QObject>
#include <QString>
#include <QVector>

/**
 * @brief Bitap近似字符串匹配引擎
 */
class BitapMatcher : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSearches = 0;          ///< 累计搜索次数
        quint64 totalMatches = 0;           ///< 累计匹配数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit BitapMatcher(QObject* parent = nullptr);

    /**
     * @brief 近似字符串搜索(允许编辑距离)
     * @param text 文本
     * @param pattern 模式
     * @param maxErrors 最大允许编辑距离(0=精确)
     * @return 匹配起始位置列表
     */
    QVector<int> search(const QString& text, const QString& pattern,
                        int maxErrors = 0);

    /**
     * @brief 精确字符串搜索(Shift-Or优化)
     * @param text 文本
     * @param pattern 模式
     * @return 匹配起始位置列表
     */
    QVector<int> exactSearch(const QString& text, const QString& pattern);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 @param matchCount 匹配数 */
    void searchCompleted(int matchCount);

private:
    double m_timeSum;           ///< 累计耗时(ms)
    mutable Stats m_stats;      ///< 可变统计
};

#endif // BITAPMATCHER_H
