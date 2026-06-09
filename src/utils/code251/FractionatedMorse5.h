/**
 * @file FractionatedMorse5.h
 * @brief 分数摩尔斯密码(变长摩尔斯元素组+维特比网格概率解密) — Fractionated Morse with Variable-Length Morse Element Groups and Probabilistic Decryption via Viterbi Trellis
 *
 * 功能: 实现分数摩尔斯密码(Fractionated Morse Cipher)，支持变长摩尔斯
 *       元素组(variable-length Morse element groups)将摩尔斯码映射为
 *       固定长度组，维特比网格(Viterbi trellis)实现概率解密，
 *       处理不确定和噪声密文。
 *
 * 协作: HuffmanCodec4(哈夫曼编解码) / Base85Codec3(Base85编解码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief 分数摩尔斯密码(变长组+维特比解密)
 */
class FractionatedMorse5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FractionatedMorse5(QObject *parent = nullptr);
    ~FractionatedMorse5() override;

    /** @brief Set ciphertext alphabet (default: 26 letters) */
    void setAlphabet(const QString& alphabet);

    /** @brief Set transition probability for Viterbi (0..1) */
    void setTransitionProb(double p);

    /** @brief Encrypt plaintext to fractionated Morse ciphertext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using Viterbi trellis */
    QString decrypt(const QString& ciphertext);

    /** @brief Convert text to Morse code string (dots, dashes, x-separator) */
    QString toMorse(const QString& text) const;

    /** @brief Convert Morse string back to text */
    QString fromMorse(const QString& morse) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptCompleted(int inputLen, int outputLen, double timeMs);
    void decryptCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_alphabet;
    double m_transProb = 0.8;

    /** @brief Morse code lookup table */
    QMap<QChar, QString> m_morseTable;

    /** @brief Reverse Morse lookup */
    QMap<QString, QChar> m_reverseMorse;

    /** @brief Fractionated Morse trigram to letter mapping */
    QMap<QString, QChar> m_trigramMap;

    /** @brief Letter to trigram mapping */
    QMap<QChar, QString> m_reverseTrigram;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize Morse code table */
    void initMorseTable();

    /** @brief Build fractionated trigram mapping */
    void buildTrigramMap();

    /** @brief Pad Morse to groups of 3 and map to letters */
    QString morseToTrigramLetters(const QString& morse) const;

    /** @brief Viterbi decode: find most likely plaintext */
    QString viterbiDecode(const QString& ciphertext) const;

    /** @brief Compute emission log-probability */
    double emissionLogProb(QChar observed, QChar expected) const;
};
