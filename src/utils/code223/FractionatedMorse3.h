/**
 * @file FractionatedMorse3.h
 * @brief 分数摩尔斯密码(扩展三字符表+爬山随机重启密钥搜索) — Fractionated Morse Cipher with Extended Trigraph Table and Hill-Climbing Random Restart Key Search
 *
 * 功能: 实现分数摩尔斯加密/解密，使用扩展三字符(Trigraph)映射表，
 *       集成爬山算法与随机重启进行自动密钥恢复。
 *
 * 协作: HillCipher8(希尔密码) / VigenereCipher5(维吉尼亚) / ADFG VX4(ADFGVX)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @brief 分数摩尔斯密码(扩展Trigraph+爬山密钥搜索)
 */
class FractionatedMorse3 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher result */
    struct CipherResult {
        QString text;
        QString key;
        double score = 0.0;
        int restarts = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int encryptOps = 0;
        int decryptOps = 0;
        int crackOps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FractionatedMorse3(QObject *parent = nullptr);
    ~FractionatedMorse3() override;

    /** @brief Set alphabet key for trigraph substitution (26 letters) */
    void setKey(const QString& key);

    /** @brief Encrypt plaintext to fractionated Morse ciphertext */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext back to plaintext */
    CipherResult decrypt(const QString& ciphertext);

    /** @brief Crack cipher via hill-climbing with random restarts */
    CipherResult crack(const QString& ciphertext, int maxRestarts = 50,
                       int hillClimbSteps = 1000);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, double timeMs);

private:
    QString m_key;
    QMap<QString, QChar> m_morseToLetter;
    QMap<QChar, QString> m_letterToMorse;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build standard Morse code mapping */
    void buildMorseTable();

    /** @brief Build trigraph substitution from current key */
    void buildTrigraphTable();

    /** @brief Convert text to Morse with 'x' separator */
    QString textToMorse(const QString& text) const;

    /** @brief Convert Morse (with 'x' separator) back to text */
    QString morseToText(const QString& morse) const;

    /** @brief Score plaintext using English quadgram fitness */
    double englishFitness(const QString& text) const;

    /** @brief Mutate key by swapping two random positions */
    QString mutateKey(const QString& key) const;

    /** @brief Generate random 26-letter key */
    QString randomKey() const;
};
