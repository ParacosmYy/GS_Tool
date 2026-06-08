/**
 * @file DoubleTranspositionCode3.h
 * @brief 双重置换密码(字谜搜索+杆相交模式分析) — Double Transposition Cipher with Anagramming Search and Rod-Based Intersection Pattern Analysis
 *
 * 功能: 实现双重置换密码的加密与解密，结合字谜搜索技术和
 *       基于杆的相交模式分析进行已知明文攻击和密钥恢复。
 *
 * 协作: AesEncryptor(加密) / RsaSigner(签名) / Base64Tool(编码)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 双重置换密码(字谜搜索+杆相交分析)
 */
class DoubleTranspositionCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief Analysis result from cryptanalysis */
    struct AnalysisResult {
        QVector<int> columnKey;
        QVector<int> rowKey;
        QString plaintext;
        double score = 0.0;
        bool valid = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int messageLength = 0;
        int numColumns = 0;
        int numRows = 0;
        int searchDepth = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DoubleTranspositionCode3(QObject *parent = nullptr);
    ~DoubleTranspositionCode3() override;

    /** @brief Set column and row permutation keys */
    void setKeys(const QVector<int>& columnKey, const QVector<int>& rowKey);

    /** @brief Encrypt plaintext using double transposition */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using double transposition */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Perform anagramming search with known plaintext crib */
    AnalysisResult anagrammingSearch(const QString& ciphertext,
                                      const QString& crib) const;

    /** @brief Rod-based intersection pattern analysis */
    QVector<QVector<int>> rodIntersectionAnalysis(
        const QString& ciphertext, int cols) const;

    /** @brief Score a candidate key based on n-gram frequency */
    double scorePlaintext(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int length, double timeMs);
    void analysisCompleted(double score, double timeMs);

private:
    QVector<int> m_columnKey;
    QVector<int> m_rowKey;
    int m_cols = 0;
    int m_rows = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Single transposition with given key */
    QString singleTranspose(const QString& text, const QVector<int>& key,
                             int cols, bool encrypt) const;

    /** @brief Generate all permutations of length n (limited) */
    void generatePermutations(int n, int maxCount,
                               QVector<QVector<int>>& perms) const;

    /** @brief Validate key permutation (0-indexed, all unique) */
    bool validateKey(const QVector<int>& key) const;

    /** @brief Build rod diagram from ciphertext */
    QVector<QString> buildRods(const QString& ciphertext,
                                int cols) const;

    /** @brief English bigram frequency score */
    double bigramScore(const QString& text) const;
};
