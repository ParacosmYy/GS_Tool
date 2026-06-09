/**
 * @file MorbitCode5.h
 * @brief Morbit密码(穷举密钥搜索剪枝+摩尔斯三元组频率评分自动解密) — Morbit Code with Exhaustive Key Search Pruning and Morse Trigram Frequency Scoring for Automated Decryption
 *
 * 功能: 实现Morbit密码(Morbit cipher)解密，通过穷举密钥搜索剪枝
 *       (exhaustive key search pruning)结合摩尔斯三元组频率评分
 *       (Morse trigram frequency scoring)实现自动解密。
 *
 * 协作: SubstitutionCipher3(替换密码) / VigenereCipher2(Vigenere密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief Morbit密码(穷举密钥搜索剪枝+摩尔斯三元组评分)
 */
class MorbitCode5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int keyLength = 0;
        int numCandidates = 0;
        double bestScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Decryption result with score */
    struct Result {
        QString plaintext;
        QString key;
        double score = 0.0;
    };

    explicit MorbitCode5(QObject *parent = nullptr);
    ~MorbitCode5() override;

    /** @brief Set ciphertext for decryption */
    void setCiphertext(const QString& cipher);

    /** @brief Set maximum candidates to evaluate */
    void setMaxCandidates(int max);

    /** @brief Set minimum score threshold for pruning */
    void setScoreThreshold(double threshold);

    /** @brief Run exhaustive key search with pruning */
    Result decrypt();

    /** @brief Decrypt with a known key */
    QString decryptWithKey(const QString& key) const;

    /** @brief Score plaintext using Morse trigram frequencies */
    double scorePlaintext(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decryptionCompleted(double bestScore, double timeMs);

private:
    QString m_cipher;
    int m_maxCandidates = 10000;
    double m_scoreThreshold = 0.0;

    // Morse code table (letter -> morse string)
    QMap<QChar, QString> m_morseTable;
    // Morse trigram frequency map
    QMap<QString, double> m_trigramFreq;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Morse code lookup table */
    void buildMorseTable();

    /** @brief Build English Morse trigram frequency table */
    void buildTrigramFreq();

    /** @brief Generate all permutations of key digits */
    QVector<QString> generateKeys() const;

    /** @brief Convert digit pair to Morse substring */
    QString digitPairToMorse(const QString& pair) const;

    /** @brief Convert Morse string to plaintext */
    QString morseToText(const QString& morse) const;

    /** @brief Prune candidates by partial score */
    QVector<QString> pruneKeys(const QVector<QString>& keys) const;
};
