/**
 * @file DoubleTranspositionCode4.h
 * @brief 双重置换密码(双独立关键字列置换+位置扰乱) — Double Transposition Cipher with Two Independent Keyword Columnar Transpositions and Positional Scrambling
 *
 * 功能: 实现双重置换密码(Double transposition cipher)，使用两个独立关键字(keyword)
 *       的列置换(columnar transposition)进行双重加密，结合位置扰乱(positional scrambling)
 *       增强密文安全性，支持加密与解密操作。
 *
 * 协作: HillCipher3(Hill密码) / VigenereCipher5(Vigenere密码) / Aes256Codec6(AES编解码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 双重置换密码(双独立关键字列置换+位置扰乱)
 */
class DoubleTranspositionCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher result */
    struct CipherResult {
        QString text;
        int key1OrderLen = 0;
        int key2OrderLen = 0;
        int paddingAdded = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        int totalCharsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DoubleTranspositionCode4(QObject *parent = nullptr);
    ~DoubleTranspositionCode4() override;

    /** @brief Set first keyword */
    void setKeyword1(const QString& key);

    /** @brief Set second keyword */
    void setKeyword2(const QString& key);

    /** @brief Set padding character */
    void setPaddingChar(QChar ch);

    /** @brief Enable positional scrambling */
    void setScrambleEnabled(bool enable);

    /** @brief Encrypt plaintext */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    CipherResult decrypt(const QString& ciphertext);

    /** @brief Get column order from keyword */
    QVector<int> columnOrder(const QString& keyword) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int length, double timeMs);
    void decryptionCompleted(int length, double timeMs);

private:
    QString m_keyword1;
    QString m_keyword2;
    QChar m_paddingChar = QLatin1Char('X');
    bool m_scrambleEnabled = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Single columnar transposition encrypt */
    QString columnEncrypt(const QString& text, const QVector<int>& order) const;

    /** @brief Single columnar transposition decrypt */
    QString columnDecrypt(const QString& text, const QVector<int>& order) const;

    /** @brief Apply positional scramble */
    QString scramble(const QString& text) const;

    /** @brief Reverse positional scramble */
    QString unscramble(const QString& text) const;

    /** @brief Generate scramble pattern from key */
    QVector<int> scramblePattern(int length) const;
};
