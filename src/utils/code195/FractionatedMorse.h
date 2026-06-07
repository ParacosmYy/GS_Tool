/**
 * @file FractionatedMorse.h
 * @brief 分体摩尔斯密码(三字母列置换+摩尔斯统计分析) — Fractionated Morse Cipher with Trigraph Columnar Transposition and Morse Statistical Analysis
 *
 * 功能: 实现分体摩尔斯密码，支持摩尔斯编码/解码、
 *       三字母列置换和摩尔斯统计分析。
 *
 * 协作: SubstitutionCipher2(替换密码) / TranspositionCipher3(置换密码) / FrequencyAnalyzer4(频率分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @brief 分体摩尔斯密码(三字母列置换)
 */
class FractionatedMorse : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncryptions = 0;
        quint64 totalDecryptions = 0;
        int lastInputLength = 0;
        int lastOutputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FractionatedMorse(QObject *parent = nullptr);
    ~FractionatedMorse() override;

    /** @brief Set the keyword for columnar transposition */
    void setKeyword(const QString& keyword);

    /** @brief Encrypt plaintext using fractionated Morse */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext back to plaintext */
    QString decrypt(const QString& ciphertext);

    /** @brief Perform Morse statistical analysis on ciphertext */
    QMap<QChar, double> analyzeMorseFrequency(const QString& ciphertext) const;

    /** @brief Crack cipher using frequency analysis */
    QString crack(const QString& ciphertext) const;

    /** @brief Get the Morse code table */
    QMap<QChar, QString> morseTable() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inLen, int outLen, double timeMs);
    void decryptionCompleted(int inLen, int outLen, double timeMs);

private:
    QString m_keyword;

    // Morse code mapping: A-Z, 0-9
    QMap<QChar, QString> m_morseEncode;
    QMap<QString, QChar> m_morseDecode;

    // Fractionated Morse trigraph table: 26 entries for 26 letters
    QVector<QString> m_trigraphTable;

    // Columnar transposition order derived from keyword
    QVector<int> m_colOrder;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Morse code table */
    void buildMorseTable();

    /** @brief Build fractionated trigraph table */
    void buildTrigraphTable();

    /** @brief Derive column order from keyword */
    void deriveColumnOrder();

    /** @brief Convert plaintext to Morse string (dots, dashes, x-separators) */
    QString toMorseString(const QString& text) const;

    /** @brief Convert Morse string back to plaintext */
    QString fromMorseString(const QString& morse) const;

    /** @brief Pad Morse string to multiple of 3 and encode to trigraphs */
    QString morseToTrigraphs(const QString& morse) const;

    /** @brief Decode trigraphs back to Morse string */
    QString trigraphsToMorse(const QString& trigraphs) const;

    /** @brief Columnar transposition encrypt */
    QString columnarEncrypt(const QString& text) const;

    /** @brief Columnar transposition decrypt */
    QString columnarDecrypt(const QString& text) const;
};
