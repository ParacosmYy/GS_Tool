/**
 * @file ShannonCoding.h
 * @brief Shannon-Fano编码 — 带Huffman对比的熵编码实现
 *
 * 功能:
 *   - Shannon-Fano编码: 基于概率排序的前缀编码
 *   - Huffman编码: 基于贪心合并的最优前缀编码
 *   - 两种编码的对比分析(码率、效率、冗余度)
 *   - 编码/解码操作
 *   - 统计编码次数、符号数、压缩率
 */

#pragma once

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QByteArray>

/**
 * @class ShannonCoding
 * @brief Shannon-Fano编码器 — 含Huffman对比分析
 *
 * Shannon-Fano编码通过递归二分概率表构建编码树，
 * 理论效率略低于Huffman编码，但实现更简单。
 * 本模块同时实现Huffman编码用于对比分析。
 */
class ShannonCoding : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEncodings = 0;      /**< 总编码次数 */
        int totalSymbols = 0;        /**< 总处理符号数 */
        int totalBitsEncoded = 0;    /**< 总编码比特数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 编码表条目 */
    struct CodeEntry {
        QChar symbol;          /**< 符号 */
        double probability;    /**< 概率 */
        QString code;          /**< 编码(二进制串) */
        int codeLength;        /**< 码长 */
    };

    /** @brief 编码结果 */
    struct EncodingResult {
        QMap<QChar, QString> codeTable;  /**< 编码表 */
        QByteArray encodedData;          /**< 编码后数据 */
        int originalBits;                /**< 原始比特数 */
        int encodedBits;                 /**< 编码后比特数 */
        double compressionRatio;         /**< 压缩比 */
        double entropy;                  /**< 信源熵 */
        double efficiency;               /**< 编码效率 */
        double redundancy;               /**< 冗余度 */
    };

    /** @brief 编码对比结果 */
    struct ComparisonResult {
        EncodingResult shannon;      /**< Shannon-Fano结果 */
        EncodingResult huffman;      /**< Huffman结果 */
        double shannonCodeLength;    /**< Shannon平均码长 */
        double huffmanCodeLength;    /**< Huffman平均码长 */
        double lengthDifference;     /**< 码长差 */
    };

    /** @brief 构造函数 */
    explicit ShannonCoding(QObject* parent = nullptr);

    /**
     * @brief Shannon-Fano编码
     * @param data 待编码文本
     * @return 编码结果
     */
    EncodingResult encodeShannon(const QString& data) const;

    /**
     * @brief Huffman编码
     * @param data 待编码文本
     * @return 编码结果
     */
    EncodingResult encodeHuffman(const QString& data) const;

    /**
     * @brief 对比Shannon-Fano与Huffman编码
     * @param data 待编码文本
     * @return 对比结果
     */
    ComparisonResult compare(const QString& data) const;

    /**
     * @brief 从概率表构建Shannon-Fano编码
     * @param probabilities 符号概率映射
     * @return 编码表
     */
    QVector<CodeEntry> buildShannonCode(
        const QMap<QChar, double>& probabilities) const;

    /**
     * @brief 从概率表构建Huffman编码
     * @param probabilities 符号概率映射
     * @return 编码表
     */
    QVector<CodeEntry> buildHuffmanCode(
        const QMap<QChar, double>& probabilities) const;

    /**
     * @brief 计算信源熵
     * @param probabilities 符号概率映射
     * @return 熵值(比特/符号)
     */
    double calculateEntropy(const QMap<QChar, double>& probabilities) const;

    /**
     * @brief 统计文本中符号频率
     * @param data 输入文本
     * @return 符号概率映射
     */
    QMap<QChar, double> calculateFrequencies(const QString& data) const;

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 */
    void encodingCompleted(const QString& method, int symbolCount,
                           double compressionRatio);

private:
    /** @brief Shannon-Fano递归分表 */
    void shannonSplit(QVector<CodeEntry>& entries, int start, int end) const;

    /** @brief Huffman树节点 */
    struct HuffNode {
        QChar symbol;
        double probability;
        HuffNode* left;
        HuffNode* right;
        HuffNode(QChar s, double p)
            : symbol(s), probability(p), left(nullptr), right(nullptr) {}
    };

    /** @brief 构建Huffman树 */
    HuffNode* buildHuffmanTree(
        const QMap<QChar, double>& probabilities) const;

    /** @brief 递归生成Huffman编码 */
    void generateHuffCodes(HuffNode* node, const QString& prefix,
                           QVector<CodeEntry>& entries) const;

    /** @brief 释放Huffman树 */
    void freeHuffTree(HuffNode* node) const;

    /** @brief 执行编码(内部) */
    EncodingResult performEncoding(
        const QString& data, const QVector<CodeEntry>& codeTable,
        const QMap<QChar, double>& probs) const;

    mutable Stats m_stats;       /**< 统计信息 */
    mutable double m_timeSum = 0.0; /**< 累计时间 */
};
