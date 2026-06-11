/**
 * @file FractionatedMorse8.h
 * @brief 分组摩尔斯密码(三字母置换表与变长摩尔斯符号分组实现分层分组密码) — Fractionated Morse with Trigraph Permutation Table and Variable-length Morse Symbol Grouping for Layered Fractionation Cipher
 *
 * 功能: 实现分组摩尔斯密码(Fractionated Morse cipher)，采用三字母置换表(trigraph permutation table)
 *       与变长摩尔斯符号分组(variable-length Morse symbol grouping)实现分层分组密码(layered fractionation cipher)。
 *
 * 协作: AffineCipher5(仿射密码) / PlayfairCipher6(Playfair密码) / VigenereCipher7(Vigenere密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

class FractionatedMorse8 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher result */
    struct CipherResult {
        QString output;
        int inputLength = 0;
        int outputLength = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncrypts = 0;
        int numDecrypts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FractionatedMorse8(QObject *parent = nullptr);
    ~FractionatedMorse8() override;

    /** @brief Set the 26-character permutation key */
    void setKey(const QString& key);

    /** @brief Encrypt plaintext using fractionated Morse */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext back to plaintext */
    CipherResult decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int inLen, int outLen, double timeMs);

private:
    QString m_key;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Morse code mapping: letter -> morse string */
    QMap<QChar, QString> m_morseTable;

    /** @brief Reverse Morse mapping: morse string -> letter */
    QMap<QString, QChar> m_reverseMorse;

    /** @brief Trigraph to letter mapping (for encryption) */
    QMap<QString, QChar> m_trigramToLetter;

    /** @brief Letter to trigraph mapping (for decryption) */
    QMap<QChar, QString> m_letterToTrigram;

    /** @brief Standard 26-character alphabet for trigraph generation */
    static const QString ALPHABET;

    /** @brief Initialize Morse code table */
    void initMorseTable();

    /** @brief Build trigram permutation mappings from key */
    void buildTrigramMappings();

    /** @brief Generate all 26 Morse trigrams in standard order */
    QVector<QString> generateStandardTrigrams() const;

    /** @brief Convert plaintext to Morse string with separators */
    QString textToMorse(const QString& text) const;

    /** @brief Convert Morse string back to text */
    QString morseToText(const QString& morse) const;
};
