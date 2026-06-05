#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief WaveletTree4 - 小波树数据结构
 *
 * 基于小波变换的压缩索引结构，支持rank/select
 * 操作和字符频率统计，适用于全文索引和压缩。
 */
class WaveletTree4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTreesBuilt = 0;
        int totalQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletTree4(QObject* parent = nullptr);

    /** @brief 从整数序列构建小波树 */
    void build(const QVector<int>& sequence, int alphabetSize = 0);

    /** @brief Rank查询: 序列[0..pos]中值val出现的次数 */
    int rank(int pos, int val) const;

    /** @brief Select查询: 值val第k次出现的位置 */
    int select(int k, int val) const;

    /** @brief 访问指定位置的值 */
    int access(int pos) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeBuilt(int sequenceLength, int alphabetSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_alphabetSize = 0;
    int m_sequenceLength = 0;
    struct WTNode;
    WTNode* m_root = nullptr;
};
