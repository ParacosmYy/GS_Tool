/**
 * @file PolluxCode.h
 * @brief Pollux密码(莫尔斯变长编码+点划频率密码分析) — Pollux Cipher with Morse-Based Variable-Length Encoding and Dot-Dash Frequency Cryptanalysis
 *
 * 功能: 实现Pollux密码的编码与解码，支持莫尔斯电码变长符号映射、
 *       点划频率统计分析和基于频率的密钥推断。
 *
 * 协作: Huffman9(哈夫曼编码) / Vigenere5(维吉尼亚) / FrequencyAnalyzer(频率分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QMap>
#include <QString>

/**
 * @brief Pollux密码(莫尔斯变长编码+频率分析)
 */
class PolluxCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOps = 0;
        int encodeOps = 0;
        int decodeOps = 0;
        int lastInputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolluxCode(QObject *parent = nullptr);
    ~PolluxCode() override;

    /** @brief Set custom key mapping (dot/dash/sep -> digits) */
    void setKey(const QMap<QChar, QVector<int>>& key);

    /** @brief Encode plaintext to Pollux cipher text */
    QString encode(const QString& plaintext);

    /** @brief Decode Pollux cipher text to plaintext */
    QString decode(const QString& ciphertext) const;

    /** @brief Analyze dot/dash frequency distribution in ciphertext */
    QMap<QChar, double> frequencyAnalysis(const QString& ciphertext) const;

    /** @brief Attempt key recovery from known plaintext-ciphertext pair */
    QMap<QChar, QVector<int>> crackKey(const QString& plain,
                                        const QString& cipher) const;

    /** @brief Convert text to Morse code string */
    QString toMorse(const QString& text) const;

    /** @brief Convert Morse string back to text */
    QString fromMorse(const QString& morse) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& type, int inputLen, double timeMs);

private:
    QMap<QChar, QVector<int>> m_key;     // '.'/'-'/' ' -> digit list
    QMap<QString, QChar> m_morseTable;   // morse -> char
    QMap<QChar, QString> m_charToMorse;  // char -> morse

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize standard Morse code table */
    void initMorseTable();

    /** @brief Initialize default Pollux key */
    void initDefaultKey();
};
