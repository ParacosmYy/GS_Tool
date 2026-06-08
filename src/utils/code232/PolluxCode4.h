/**
 * @file PolluxCode4.h
 * @brief Pollux密码(扩展摩尔斯三字组统计+Viterbi路径解码最优密钥序列) — Pollux Cipher with Extended Morse Trigraph Statistics and Viterbi Path Decoding for Optimal Key Sequence
 *
 * 功能: 实现Pollux密码(Pollux cipher)，利用扩展摩尔斯三字组统计(extended Morse
 *       trigraph statistics)分析密文，并采用Viterbi路径解码(Viterbi path decoding)
 *       算法搜索最优密钥序列(key sequence)进行解密。
 *
 * 协作: CaesarCipher3(凯撒密码) / VigenereCipher4(维吉尼亚密码) / AffineCipher2(仿射密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief Pollux密码(扩展摩尔斯三字组统计+Viterbi路径解码最优密钥序列)
 */
class PolluxCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief Morse symbol: dot, dash, or word-space */
    enum Symbol { Dot = 0, Dash = 1, Space = 2 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int cipherLength = 0;
        int plainLength = 0;
        int keyLength = 0;
        double trigraphScore = 0.0;
        double viterbiScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolluxCode4(QObject *parent = nullptr);
    ~PolluxCode4() override;

    /** @brief Set digit-to-symbol key mapping (0-9 -> Dot/Dash/Space) */
    void setKey(const QMap<int, Symbol>& key);

    /** @brief Encrypt plaintext to Pollux ciphertext digits */
    QVector<int> encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext digits to plaintext */
    QString decrypt(const QVector<int>& cipher) const;

    /** @brief Auto-break cipher using trigraph statistics + Viterbi */
    QString autoBreak(const QVector<int>& cipher);

    /** @brief Get current key mapping */
    QMap<int, Symbol> key() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptCompleted(int len, double timeMs);
    void decryptCompleted(int len, double timeMs);
    void autoBreakCompleted(double score, double timeMs);

private:
    QMap<int, Symbol> m_key;
    QMap<QChar, QString> m_morseTable;  // char -> morse string (e.g. ".-")

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build standard Morse code table */
    void buildMorseTable();

    /** @brief Convert plaintext to Morse symbols (dot/dash/space sequence) */
    QVector<Symbol> textToMorse(const QString& text) const;

    /** @brief Convert Morse symbols back to text */
    QString morseToText(const QVector<Symbol>& symbols) const;

    /** @brief Compute trigraph log-likelihood for candidate Morse sequence */
    double trigraphScore(const QVector<Symbol>& seq) const;

    /** @brief Viterbi decoder: find optimal digit-to-symbol mapping */
    QVector<Symbol> viterbiDecode(const QVector<int>& cipher) const;

    /** @brief Initialize trigraph frequency table (English) */
    void initTrigraphFreq();
};
