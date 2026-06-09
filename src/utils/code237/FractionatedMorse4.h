/**
 * @file FractionatedMorse4.h
 * @brief 分组摩尔斯密码(三字母代换表+可配置码元时序) — Fractionated Morse with Trigram-to-Letter Substitution Table and Configurable Morse Element Timing
 *
 * 功能: 实现分组摩尔斯密码(Fractionated Morse cipher)，采用三字母代换表(trigram-to-letter
 *       substitution table)将摩尔斯三码元组映射为字母，支持可配置摩尔斯码元时序(configurable
 *       Morse element timing)用于精确编码解码。
 *
 * 协作: BaconCipher3(培根密码) / PlayfairCipher6(Playfair密码) / VigenereCipher5(维吉尼亚密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief 分组摩尔斯密码(三字母代换表+可配置码元时序)
 */
class FractionatedMorse4 : public QObject {
    Q_OBJECT

public:
    /** @brief Morse element timing configuration */
    struct Timing {
        double ditDuration = 1.0;     // dit = 1 unit
        double dahDuration = 3.0;     // dah = 3 units
        double intraCharGap = 1.0;    // gap within character
        double interCharGap = 3.0;    // gap between characters
        double wordGap = 7.0;         // gap between words
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncoded = 0;
        int numDecoded = 0;
        int totalCharacters = 0;
        int totalTrigrams = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FractionatedMorse4(QObject *parent = nullptr);
    ~FractionatedMorse4() override;

    /** @brief Set custom Morse element timing */
    void setTiming(const Timing& t);

    /** @brief Set keyword for substitution alphabet generation */
    void setKeyword(const QString& keyword);

    /** @brief Encode plaintext to fractionated Morse ciphertext */
    QString encode(const QString& plaintext);

    /** @brief Decode fractionated Morse ciphertext to plaintext */
    QString decode(const QString& ciphertext);

    /** @brief Convert text to Morse code string with configured timing */
    QString textToMorse(const QString& text) const;

    /** @brief Convert Morse code string back to text */
    QString morseToText(const QString& morse) const;

    /** @brief Get current substitution table */
    QMap<QString, QChar> substitutionTable() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int chars, int trigrams, double timeMs);
    void decodeCompleted(int chars, int trigrams, double timeMs);

private:
    Timing m_timing;
    QString m_keyword;
    QMap<QChar, QString> m_charToMorse;      // letter -> morse string
    QMap<QString, QChar> m_morseToChar;      // morse string -> letter
    QMap<QString, QChar> m_trigramToLetter;  // trigram -> cipher letter
    QMap<QChar, QString> m_letterToTrigram;  // cipher letter -> trigram

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize standard International Morse table */
    void initMorseTable();

    /** @brief Build fractionated substitution table from keyword */
    void buildSubstitutionTable();

    /** @brief Pad Morse to trigram boundaries and split */
    QVector<QString> splitTrigrams(const QString& morseStream) const;
};
