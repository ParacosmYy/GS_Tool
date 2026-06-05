/**
 * @file HuffmanTreeBuilder.h
 * @brief 通用霍夫曼树构建器 — 从频率表生成变长编码
 *
 * 功能: 从符号频率构建霍夫曼树，生成规范编码表，
 *       支持编解码、码本序列化、统计树深度/耗时。
 */
#ifndef HUFFMANTREEBUILDER_H
#define HUFFMANTREEBUILDER_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QHash>
#include <QMap>

class HuffmanTreeBuilder : public QObject {
    Q_OBJECT
public:
    /** 编码条目: 符号 → (码字, 码长) */
    struct CodeEntry {
        quint32 code = 0;
        int     length = 0;
    };

    /** 统计 */
    struct Stats {
        quint64 totalTreesBuilt = 0;
        quint64 totalSymbolsEncoded = 0;
        int     maxTreeDepth = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit HuffmanTreeBuilder(QObject* parent = nullptr);
    ~HuffmanTreeBuilder();

    /** @brief 从频率表构建树 @param freq 符号→频率 @return 是否成功 */
    bool buildFromFrequency(const QMap<int, quint64>& freq);

    /** @brief 生成规范编码 @return 符号→编码条目 */
    QMap<int, CodeEntry> canonicalCodes() const;

    /** @brief 编码数据 @param symbols 符号列表 @return 比特流(QByteArray) */
    QByteArray encodeSymbols(const QVector<int>& symbols) const;

    /** @brief 解码数据 @param data 比特流 @param count 符号数 @return 符号列表 */
    QVector<int> decodeSymbols(const QByteArray& data, int count) const;

    /** @brief 获取码表 */
    const QMap<int, CodeEntry>& codeTable() const { return m_codeTable; }

    /** @brief 码表是否已构建 */
    bool hasCodeTable() const { return !m_codeTable.isEmpty(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeBuilt(int symbolCount, int maxDepth);
    void symbolsEncoded(int count, int outputBytes);

private:
    struct HuffNode {
        int     symbol;     ///< -1 表示内部节点
        quint64 freq;
        HuffNode* left;
        HuffNode* right;
    };

    void deleteTree(HuffNode* node);
    int computeDepth(HuffNode* node, int depth);

    QMap<int, CodeEntry> m_codeTable;
    HuffNode* m_root;
    Stats m_stats;
    double m_timeSum;
};

#endif // HUFFMANTREEBUILDER_H
