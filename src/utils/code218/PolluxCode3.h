/**
 * @file PolluxCode3.h
 * @brief Pollux密码变体(扩展莫尔斯符号集+贝叶斯密钥推断) — Pollux Cipher Variant with Extended Morse Symbol Set and Bayesian Key Inference from Ciphertext Statistics
 *
 * 功能: 实现Pollux密码的扩展变体，使用扩展莫尔斯符号集编码，
 *       通过密文统计特征进行贝叶斯推断恢复密钥。
 *
 * 协作: VigenereCipher2(维吉尼亚) / EnigmaMachine3(恩尼格玛) / ADFGVX2(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief Pollux密码变体(扩展莫尔斯+贝叶斯推断)
 */
class PolluxCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int keyLength = 0;
        double inferenceScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Morse symbol: dot, dash, letter-space, word-space */
    enum Symbol { Dot = 0, Dash = 1, LetterGap = 2, WordGap = 3 };

    explicit PolluxCode3(QObject *parent = nullptr);
    ~PolluxCode3() override;

    /** @brief Set encryption key (mapping digit->symbol) */
    void setKey(const QMap<int, Symbol>& keyMapping);

    /** @brief Encrypt plaintext to Pollux ciphertext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using current key */
    QString decrypt(const QString& ciphertext);

    /** @brief Bayesian key inference from ciphertext statistics */
    QMap<int, Symbol> inferKey(const QString& ciphertext,
                                const QMap<QChar, QString>& morseTable);

    /** @brief Get standard Morse code table */
    QMap<QChar, QString> standardMorseTable() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);
    void keyInferred(int keySize, double score, double timeMs);

private:
    QMap<int, Symbol> m_key;
    QMap<QChar, QString> m_morseTable;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build standard Morse code table */
    void buildMorseTable();

    /** @brief Convert plaintext to Morse sequence */
    QString textToMorse(const QString& text) const;

    /** @brief Count symbol frequencies in ciphertext */
    QVector<double> symbolFrequencies(const QString& ciphertext) const;

    /** @brief Compute Bayesian log-likelihood for a key hypothesis */
    double bayesianScore(const QMap<int, Symbol>& hypothesis,
                         const QString& ciphertext) const;
};
