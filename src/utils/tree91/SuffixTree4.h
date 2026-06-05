#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 后缀树工具类
 *
 * 提供后缀树的构建与模式搜索功能，支持线性时间
 * 构建和高效子串匹配查询。
 */
class SuffixTree4 : public QObject {
    Q_OBJECT
public:
    /// 构建统计信息
    struct Stats {
        int totalBuilds = 0;        ///< 总构建次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SuffixTree4(QObject* parent = nullptr);

    /** @brief 根据输入字符串构建后缀树 */
    void build(const QString& text);

    /** @brief 在后缀树中搜索模式串，返回是否匹配 */
    bool search(const QString& pattern);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 构建完成信号，返回字符串长度 */
    void buildCompleted(int textLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_text;
};
