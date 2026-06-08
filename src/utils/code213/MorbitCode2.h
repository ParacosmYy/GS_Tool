/**
 * @file MorbitCode2.h
 * @brief Morbit密码(摩尔斯数字映射+频率直方图密钥枚举) — Morbit Code with Morse-Based Digit Mapping and Frequency Histogram Analysis for Key Enumeration
 *
 * 功能: 实现Morbit密码编解码，支持摩尔斯电码数字映射、
 *       频率直方图分析和密钥空间枚举攻击。
 *
 * 协作: HuffmanCodec5(哈夫曼编解码) / RSACrypto3(RSA加密) / Checksum8(校验和)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief Morbit密码(摩尔斯映射+频率直方图)
 */
class MorbitCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int keyLength = 0;
        int messageLength = 0;
        int candidatesTried = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Frequency histogram entry */
    struct HistogramEntry {
        QChar symbol;
        int count = 0;
        double frequency = 0.0;
    };

    explicit MorbitCode2(QObject *parent = nullptr);
    ~MorbitCode2() override;

    /** @brief Set the 9-digit key (digits 1-9, no repeats) */
    void setKey(const QString& key);

    /** @brief Encode plaintext to Morbit ciphertext */
    QString encode(const QString& plaintext) const;

    /** @brief Decode Morbit ciphertext to plaintext */
    QString decode(const QString& ciphertext) const;

    /** @brief Convert text to Morse code string */
    QString textToMorse(const QString& text) const;

    /** @brief Convert Morse code string back to text */
    QString morseToText(const QString& morse) const;

    /** @brief Analyze frequency histogram of cipher symbols */
    QVector<HistogramEntry> frequencyHistogram(const QString& ciphertext) const;

    /** @brief Enumerate key candidates via frequency analysis */
    QVector<QString> enumerateKeys(const QString& ciphertext,
                                    int maxCandidates = 100) const;

    /** @brief Score a key candidate against ciphertext */
    double scoreKeyCandidate(const QString& key,
                             const QString& ciphertext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);
    void keyEnumerationProgress(int tried, int maxCandidates);

private:
    QString m_key;
    QVector<int> m_keyMap;  // Maps digit position to Morse pair index

    // Morse code table: A-Z, 0-9
    QVector<QString> m_morseTable;
    QVector<QChar> m_charTable;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Morse code lookup table */
    void buildMorseTable();

    /** @brief Map Morse pair (di-dah combinations) to digit */
    int morsePairToIndex(QChar a, QChar b) const;

    /** @brief Get Morse pair for a given index */
    QPair<QChar, QChar> indexToMorsePair(int idx) const;

    /** @brief Validate key format */
    bool validateKey(const QString& key) const;
};
