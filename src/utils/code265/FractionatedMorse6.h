/**
 * @file FractionatedMorse6.h
 * @brief 分式摩尔斯密码(三字符替换与变长摩尔斯-三字符映射分式编码) — Fractionated Morse with Trigraph Substitution and Variable-Length Morse-to-Trigraph Mapping for Fractionated Encoding
 *
 * 功能: 实现分式摩尔斯密码(Fractionated Morse)，采用三字符替换(trigraph substitution)
 *       和变长摩尔斯-三字符映射(variable-length Morse-to-trigraph mapping)实现分式编码。
 *
 * 协作: PlayfairCipher5(Playfair密码) / VigenereCipher4(Vigenere密码) / ADFGCipher5(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief 分式摩尔斯密码(三字符替换与变长摩尔斯-三字符映射分式编码)
 */
class FractionatedMorse6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int trigraphCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FractionatedMorse6(QObject *parent = nullptr);
    ~FractionatedMorse6() override;

    /** @brief Set substitution key (26 unique uppercase letters) */
    void setKey(const QString& key);

    /** @brief Encode plaintext to fractionated Morse cipher */
    QString encode(const QString& plaintext) const;

    /** @brief Decode fractionated Morse cipher to plaintext */
    QString decode(const QString& ciphertext) const;

    /** @brief Convert text to Morse code (dot=., dash=-, separator=/) */
    QString textToMorse(const QString& text) const;

    /** @brief Convert Morse code back to text */
    QString morseToText(const QString& morse) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherComputed(int inputLen, int outputLen, int trigraphs, double timeMs);

private:
    QString m_key;
    QMap<QChar, QString> m_morseTable;     // Letter -> Morse
    QMap<QString, QChar> m_reverseMorse;   // Morse -> Letter
    QMap<QString, QChar> m_trigramTable;   // Trigram -> Cipher letter
    QMap<QChar, QString> m_reverseTrigram; // Cipher letter -> Trigram

    Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Build standard Morse code table */
    void buildMorseTable();

    /** @brief Build trigram substitution table from key */
    void buildTrigramTable();

    /** @brief Pad Morse string to multiple of 3 with 'x' (separator) */
    QString padMorse(const QString& morse) const;

    /** @brief Split Morse into trigrams */
    QVector<QString> splitTrigrams(const QString& morse) const;
};
