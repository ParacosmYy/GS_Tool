/**
 * @file FractionatedMorse2.h
 * @brief 分组摩尔斯密码(三元频率分析+束搜索密钥恢复) — Fractionated Morse Cipher with Trigraphic Frequency Analysis and Beam Search Key Recovery
 *
 * 功能: 实现分组摩尔斯密码，支持三元频率分析、
 *       束搜索密钥恢复和统计密文攻击。
 *
 * 协作: Enigma5(恩尼格玛) / Vigenere3(维吉尼亚) / HuffmanCoder4(哈夫曼编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 分组摩尔斯密码(三元频率分析+束搜索密钥恢复)
 */
class FractionatedMorse2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int keyLength = 0;
        int beamWidth = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FractionatedMorse2(QObject *parent = nullptr);
    ~FractionatedMorse2() override;

    void setBeamWidth(int width);

    /** @brief Encrypt plaintext with given key (26-char alphabet permutation) */
    QString encrypt(const QString& plaintext, const QString& key) const;

    /** @brief Decrypt ciphertext with given key */
    QString decrypt(const QString& ciphertext, const QString& key) const;

    /** @brief Convert text to Morse code string (. = - separated by x) */
    static QString textToMorse(const QString& text);

    /** @brief Convert Morse string back to text */
    static QString morseToText(const QString& morse);

    /** @brief Split Morse stream into trigraphs */
    static QVector<QString> morseToTrigraphs(const QString& morse);

    /** @brief Compute English trigraphic frequency score for a candidate key */
    double frequencyScore(const QString& ciphertext,
                          const QString& candidateKey) const;

    /** @brief Beam search key recovery from ciphertext */
    QString crackKey(const QString& ciphertext) const;

    /** @brief Full auto-decrypt: crack key and return plaintext */
    QString autoDecrypt(const QString& ciphertext) const;

    /** @brief Score a candidate plaintext using English bigram frequencies */
    double englishScore(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void keyCracked(const QString& key, double score, double timeMs);

private:
    int m_beamWidth = 100;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Standard Morse code lookup table */
    static QVector<QPair<QChar, QString>> morseTable();

    /** @brief Get trigraph index (0-25) from a 3-symbol Morse group */
    static int trigraphIndex(const QString& trigraph);

    /** @brief Load English letter frequency distribution */
    static QVector<double> englishFreqs();

    /** @brief Load English bigram log-probabilities */
    static QVector<QVector<double>> englishBigramLogProbs();
};
