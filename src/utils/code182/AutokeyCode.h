/**
 * @file AutokeyCode.h
 * @brief Autokey密码(运行密钥+已知明文攻击) — Autokey Cipher with Running Key from Plaintext and Known-Plaintext Attack via Key Offset Discovery
 *
 * 功能: 实现Autokey密码，支持基于明文的运行密钥生成、
 *       已知明文攻击(密钥偏移发现)、加密解密和自动破解。
 *
 * 协作: VigenereCipher3(Vigenere) / HillCipher2(Hill) / PlayfairCipher4(Playfair)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief Autokey密码处理器(运行密钥+已知明文攻击)
 */
class AutokeyCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int inputLength = 0;
        int keyLength = 0;
        bool attackSuccess = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AutokeyCode(QObject *parent = nullptr);
    ~AutokeyCode() override;

    void setAlphabet(const QString& alpha);

    /** @brief 加密明文 */
    QString encrypt(const QString& plaintext, const QString& key) const;

    /** @brief 解密密文 */
    QString decrypt(const QString& ciphertext, const QString& key) const;

    /** @brief 已知明文攻击: 发现密钥 */
    QPair<bool, QString> knownPlaintextAttack(const QString& ciphertext,
                                               const QString& knownPlaintext,
                                               int keyOffset = 0) const;

    /** @brief 自动尝试发现密钥偏移 */
    QVector<QPair<int, QString>> autoDiscoverKey(const QString& ciphertext,
                                                  const QString& knownPlaintext) const;

    /** @brief 频率分析评分(英语) */
    double frequencyScore(const QString& text) const;

    /** @brief 猜测密钥长度 */
    QVector<int> guessKeyLength(const QString& ciphertext, int maxLen = 20) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length);
    void attackFinished(bool success, const QString& discoveredKey);

private:
    QString m_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Map char to index in alphabet */
    int charToIndex(QChar c) const;

    /** @brief Map index to char */
    QChar indexToChar(int idx) const;

    /** @brief Build running key from initial key + plaintext */
    QString buildRunningKey(const QString& key, int textLength) const;

    /** @brief Extract initial key from running key */
    QString extractInitialKey(const QString& runningKey, int textLength) const;

    /** @brief Index of coincidence for period detection */
    double indexOfCoincidence(const QString& text) const;

    /** @brief Chi-squared statistic vs English */
    double chiSquared(const QString& text) const;
};
