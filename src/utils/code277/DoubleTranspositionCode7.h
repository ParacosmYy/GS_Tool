/**
 * @file DoubleTranspositionCode7.h
 * @brief 双重置换密码(Myszkowski变体与重复密钥列置换增强排列复杂度的双阶段加密) — Double Transposition Cipher with Myszkowski Variant and Repeated-key Columnar for Enhanced Permutation Complexity
 *
 * 功能: 实现双重置换密码(Double transposition cipher)，采用Myszkowski变体(Myszkowski variant)
 *       与重复密钥列置换(repeated-key columnar)实现增强排列复杂度的双阶段加密(enhanced permutation complexity)。
 *
 * 协作: CaesarCipher6(凯撒密码) / VigenereCipher7(Vigenere密码) / ADFGXCipher5(ADFGX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 双重置换密码(Myszkowski变体与重复密钥列置换)
 */
class DoubleTranspositionCode7 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher result */
    struct CipherResult {
        QString ciphertext;
        QString key1Order;
        QString key2Order;
        int numRows1 = 0;
        int numRows2 = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int textLength = 0;
        int key1Length = 0;
        int key2Length = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DoubleTranspositionCode7(QObject *parent = nullptr);
    ~DoubleTranspositionCode7() override;

    /** @brief Encrypt plaintext with two keys using Myszkowski + columnar */
    CipherResult encrypt(const QString& plaintext, const QString& key1, const QString& key2);

    /** @brief Decrypt ciphertext with two keys */
    QString decrypt(const QString& ciphertext, const QString& key1, const QString& key2);

    /** @brief Get column order from key (Myszkowski variant) */
    QVector<QVector<int>> myszkowskiOrder(const QString& key) const;

    /** @brief Get column order from key (standard alphabetical) */
    QVector<int> columnOrder(const QString& key) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int textLen, int key1Len, int key2Len, double timeMs);
    void decryptDone(int textLen, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Single transposition encrypt (Myszkowski variant) */
    QString transposeEncrypt(const QString& text, const QString& key) const;

    /** @brief Single transposition decrypt (Myszkowski variant) */
    QString transposeDecrypt(const QString& text, const QString& key) const;

    /** @brief Single transposition encrypt (standard columnar) */
    QString columnarEncrypt(const QString& text, const QString& key) const;

    /** @brief Single transposition decrypt (standard columnar) */
    QString columnarDecrypt(const QString& text, const QString& key) const;
};
