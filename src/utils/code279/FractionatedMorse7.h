/**
 * @file FractionatedMorse7.h
 * @brief 分裂摩尔斯密码(扩展三字符字母表与可逆摩尔斯-符号映射的双向编码) — Fractionated Morse with Extended Trigraph Alphabet and Reversible Morse-to-Symbol Mapping for Bidirectional Encoding
 *
 * 功能: 实现分裂摩尔斯密码(Fractionated Morse)，采用扩展三字符字母表(extended trigraph alphabet)
 *       与可逆摩尔斯-符号映射(reversible Morse-to-symbol mapping)实现双向编码(bidirectional encoding)。
 *
 * 协作: HuffmanCoder6(哈夫曼编码) / AminoAcid6(氨基酸编码) / BaconCipher3(培根密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief 分裂摩尔斯密码(扩展三字符字母表与可逆摩尔斯-符号映射)
 */
class FractionatedMorse7 : public QObject {
    Q_OBJECT

public:
    /** @brief Encoding/decoding result */
    struct MorseResult {
        QString text;
        bool success = false;
        int inputLength = 0;
        int outputLength = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int encodeCount = 0;
        int decodeCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FractionatedMorse7(QObject *parent = nullptr);
    ~FractionatedMorse7() override;

    /** @brief Encode plaintext to fractionated Morse */
    MorseResult encode(const QString& plaintext) const;

    /** @brief Decode fractionated Morse back to plaintext */
    MorseResult decode(const QString& cipher) const;

    /** @brief Get Morse representation of a character */
    QString toMorse(QChar ch) const;

    /** @brief Get character from Morse code */
    QChar fromMorse(const QString& morse) const;

    /** @brief Validate cipher text format */
    bool isValidCipher(const QString& cipher) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeDone(int inLen, int outLen, double timeMs);
    void decodeDone(int inLen, int outLen, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Morse code table: letter -> dit-dah string */
    QMap<QChar, QString> m_morseTable;

    /** @brief Reverse Morse table: dit-dah string -> letter */
    QMap<QString, QChar> m_reverseMorse;

    /** @brief Trigram to symbol mapping */
    QMap<QString, QChar> m_trigramToSymbol;

    /** @brief Symbol to trigram mapping */
    QMap<QChar, QString> m_symbolToTrigram;

    /** @brief Extended alphabet for trigraph mapping (26 letters + extras) */
    static constexpr int TRIGRAPH_ALPHABET_SIZE = 26;

    /** @brief Initialize Morse code table */
    void initMorseTable();

    /** @brief Initialize trigraph-to-symbol mapping */
    void initTrigrahMapping();

    /** @brief Pad Morse element to exactly 3 symbols using 'x' separator */
    QString padTrigraph(const QString& morse) const;
};
