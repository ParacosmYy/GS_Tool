/**
 * @file LcsSolver.h
 * @brief 最长公共子序列求解器 — LCS长度/diff/相似度
 *
 * 功能: 求解两个字节序列的最长公共子序列(LCS)，
 *       生成diff操作序列(match/insert/delete)，
 *       计算相似度[0,1]，统计比较次数与平均耗时。
 */
#ifndef LCSSOLVER_H
#define LCSSOLVER_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 最长公共子序列求解器
 */
class LcsSolver : public QObject {
    Q_OBJECT

public:
    /** @brief diff操作类型 */
    enum DiffOp {
        Match  = 0,  ///< 匹配(公共子序列元素)
        Insert = 1,  ///< 插入(仅在B中存在)
        Delete = 2   ///< 删除(仅在A中存在)
    };
    Q_ENUM(DiffOp)

    /** @brief diff操作条目 */
    struct DiffEntry {
        DiffOp op;      ///< 操作类型
        char    value;  ///< 对应字节值
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComparisons = 0;    ///< 累计比较次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(毫秒)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit LcsSolver(QObject* parent = nullptr);

    /**
     * @brief 求解LCS长度
     * @param a 序列A
     * @param b 序列B
     * @return LCS长度
     */
    int solve(const QByteArray& a, const QByteArray& b);

    /**
     * @brief 生成diff操作序列
     * @param a 序列A
     * @param b 序列B
     * @return 操作序列(match/insert/delete)
     */
    QVector<DiffEntry> diff(const QByteArray& a, const QByteArray& b);

    /**
     * @brief 计算相似度(基于LCS长度)
     * @param a 序列A
     * @param b 序列B
     * @return 相似度[0.0, 1.0]
     */
    double similarity(const QByteArray& a, const QByteArray& b);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 比较完成 @param lcsLength LCS长度 */
    void comparisonCompleted(int lcsLength);

private:
    /**
     * @brief 构建LCS动态规划表
     * @param a 序列A
     * @param b 序列B
     * @return DP表(m+1) x (n+1)
     */
    QVector<QVector<int>> buildTable(const QByteArray& a,
                                     const QByteArray& b) const;

    /**
     * @brief 从DP表回溯diff操作序列
     * @param table DP表
     * @param a 序列A
     * @param b 序列B
     * @return 操作序列(逆序)
     */
    QVector<DiffEntry> backtrack(const QVector<QVector<int>>& table,
                                 const QByteArray& a,
                                 const QByteArray& b) const;

    mutable Stats m_stats;             ///< 统计信息(mutable支持const方法)
    double m_timeSum;                  ///< 累计处理时间
};

#endif // LCSSOLVER_H
