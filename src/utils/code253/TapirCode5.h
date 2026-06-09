/**
 * @file TapirCode5.h
 * @brief Tapir密码(多字母周期密钥+扩展数字标点符号集) — Tapir Code with Polyalphabetic Periodic Key and Extended Symbol Set Including Numeric and Punctuation Characters
 *
 * 功能: 实现Tapir密码(Tapir Code)，使用多字母周期密钥(polyalphabetic
 *       periodic key)进行加密解密，扩展符号集(extended symbol set)支持
 *       数字(digit)和标点符号(punctuation)字符的完整映射。
 *
 * 协作: CaesarCipher2(凯撒密码) / VigenereCipher3(维吉尼亚密码) / XORCipher1(异或密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Tapir密码(多字母周期密钥+扩展符号集)
 */
class TapirCode5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int keyLength = 0;
        int alphabetSize = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TapirCode5(QObject *parent = nullptr);
    ~TapirCode5() override;

    /** @brief Set the periodic key string */
    void setKey(const QString& key);

    /** @brief Encrypt plaintext to ciphertext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext back to plaintext */
    QString decrypt(const QString& ciphertext);

    /** @brief Get the current alphabet */
    QString alphabet() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int length, double timeMs);
    void decryptionCompleted(int length, double timeMs);

private:
    QString m_key;
    QString m_alphabet;
    QVector<int> m_keyOffsets;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build extended alphabet: A-Z, 0-9, punctuation */
    void buildAlphabet();

    /** @brief Map key characters to shift offsets */
    void computeKeyOffsets();

    /** @brief Find index of char in alphabet, or -1 */
    int charIndex(QChar c) const;

    /** @brief Shift a character by offset (mod alphabet size) */
    QChar shiftChar(QChar c, int offset) const;
};
