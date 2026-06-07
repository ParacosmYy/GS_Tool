/**
 * @file MorbitCode.h
 * @brief Morbit密码(3x3莫尔斯网格映射+频率网格恢复) — Morbit Cipher with 3x3 Morse Grid Mapping and Frequency-Based Grid Recovery
 *
 * 功能: 实现Morbit密码编解码，支持3x3莫尔斯网格映射、
 *       基于字符频率分析的网格恢复和自动密钥破解。
 *
 * 协作: HuffmanCodec3(哈夫曼编解码) / BaseNCodec5(Base-N编解码) / ADFGVX5(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief Morbit密码(3x3莫尔斯网格+频率恢复)
 */
class MorbitCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;
        quint64 totalDecodes = 0;
        int lastInputLength = 0;
        int gridKey = 0;
        bool autoRecovered = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MorbitCode(QObject *parent = nullptr);
    ~MorbitCode() override;

    /** @brief Set 9-digit grid key (digits 1-9 permutation) */
    void setGridKey(const QString& key);

    /** @brief Encode plaintext to Morbit ciphertext */
    QString encode(const QString& plaintext) const;

    /** @brief Decode Morbit ciphertext with known key */
    QString decode(const QString& ciphertext) const;

    /** @brief Attempt frequency-based grid recovery from ciphertext */
    QVector<QPair<QString, double>> crackKey(const QString& ciphertext) const;

    /** @brief Convert text to Morse code */
    QString toMorse(const QString& text) const;

    /** @brief Convert Morse code to text */
    QString fromMorse(const QString& morse) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int length, double timeMs);
    void decodeCompleted(int length, double timeMs);

private:
    QString m_gridKey = QStringLiteral("123456789");

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Morse alphabet lookup */
    static QVector<QPair<QChar, QString>> morseTable();

    /** @brief Build reverse lookup: morse -> char */
    static QVector<QPair<QString, QChar>> reverseMorse();

    /** @brief Convert 3x3 grid to digit pairs */
    QVector<QPair<int, int>> morseToDigitPairs(const QString& morse) const;

    /** @brief Score a key candidate using English frequency */
    double scoreKey(const QString& ciphertext, const QString& keyCandidate) const;
};
