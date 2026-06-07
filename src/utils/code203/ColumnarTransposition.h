/**
 * @file ColumnarTransposition.h
 * @brief 列置换密码(关键词列排序+爬山法密钥恢复) — Columnar Transposition with Keyword-Based Column Ordering and Hill-Climbing Key Recovery
 *
 * 功能: 实现列置换密码，支持关键词驱动的列排序加密解密、
 *       爬山法自动密钥恢复和N-gram频率分析。
 *
 * 协作: VigenereCipher7(维吉尼亚密码) / AesEncryptor5(AES加密) / HashUtils5(哈希工具)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 列置换密码(关键词列排序+爬山法密钥恢复)
 */
class ColumnarTransposition : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int textLength = 0;
        int keyLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ColumnarTransposition(QObject *parent = nullptr);
    ~ColumnarTransposition() override;

    /** @brief Encrypt plaintext using keyword */
    QString encrypt(const QString& plaintext, const QString& keyword) const;

    /** @brief Decrypt ciphertext using keyword */
    QString decrypt(const QString& ciphertext, const QString& keyword) const;

    /** @brief Derive column order from keyword (stable alphabetical sort) */
    QVector<int> columnOrder(const QString& keyword) const;

    /** @brief Recover key via hill-climbing with N-gram scoring */
    QString recoverKey(const QString& ciphertext, int maxKeyLen = 10) const;

    /** @brief Score text fitness using quadgram statistics */
    double quadgramScore(const QString& text) const;

    /** @brief Generate random permutation key of given length */
    QVector<int> randomKey(int length) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int textLen, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    // Precomputed English quadgram log-probabilities (top 200 entries)
    QVector<QPair<QString, double>> m_quadgrams;

    /** @brief Build quadgram frequency table */
    void initQuadgrams();

    /** @brief Permute columns during encryption */
    QString applyColumnPermutation(const QString& text, const QVector<int>& order, bool encrypt) const;

    /** @brief Swap two positions in a key and return new key */
    static QVector<int> swapKey(const QVector<int>& key, int i, int j);

    /** @brief Convert permutation to keyword representation */
    static QString permutationToString(const QVector<int>& perm);
};
