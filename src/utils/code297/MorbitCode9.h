/**
 * @file MorbitCode9.h
 * @brief Morbit密码(自适应符号表重排与频率相关替换实现动态手动密码加密) — Morbit Cipher with Adaptive Symbol Table Reordering and Frequency-dependent Substitution for Dynamic Hand Cipher Encryption
 *
 * 功能: 实现Morbit密码(Morbit cipher)，采用自适应符号表重排(adaptive symbol table reordering)
 *       与频率相关替换(frequency-dependent substitution)实现动态手动密码加密(dynamic hand cipher encryption)。
 *
 * 协作: VigenereCipher(维吉尼亚密码) / PlayfairCipher(普莱费尔密码) / SubstitutionCipher(替换密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

class MorbitCode9 : public QObject {
    Q_OBJECT

public:
    /** @brief Symbol table entry with frequency tracking */
    struct SymbolEntry {
        QChar symbol;
        int frequency = 0;
        double normFrequency = 0.0;
    };

    /** @brief Encryption result */
    struct EncResult {
        QString cipherText;
        QVector<int> keyIndices;
        int inputLength = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalEncryptions = 0;
        quint64 totalDecryptions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MorbitCode9(QObject *parent = nullptr);
    ~MorbitCode9() override;

    /** @brief Set the Morse-based key pattern (digits 1-9) */
    void setKey(const QString& key);

    /** @brief Set custom substitution alphabet */
    void setAlphabet(const QString& alphabet);

    /** @brief Encrypt plaintext using Morbit cipher with adaptive reordering */
    EncResult encrypt(const QString& plainText);

    /** @brief Decrypt ciphertext using Morbit cipher */
    QString decrypt(const QString& cipherText);

    /** @brief Get current symbol frequency table */
    QVector<SymbolEntry> frequencyTable() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int len, double timeMs);
    void decryptDone(int len, double timeMs);

private:
    QString m_key;
    QString m_alphabet;
    QVector<SymbolEntry> m_symbolTable;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Morse code lookup for a character */
    QString toMorse(QChar ch) const;

    /** @brief Convert Morse dots/dashes to digit pairs via key */
    int morsePairToIndex(const QString& morsePair) const;

    /** @brief Reorder symbol table by descending frequency */
    void adaptSymbolTable();

    /** @brief Frequency-dependent substitution map */
    QChar substitute(QChar input, int keyOffset);

    /** @brief Reverse substitution */
    QChar reverseSubstitute(QChar cipher, int keyOffset);
};
