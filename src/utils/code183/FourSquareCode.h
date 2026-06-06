/**
 * @file FourSquareCode.h
 * @brief 四方密码(2×2双字母编码+配对Polybius方阵) — Four-square Cipher with 2×2 Digraph Encoding via Paired Polybius Squares
 *
 * 功能: 实现四方密码加密解密，支持双关键字生成配对Polybius方阵、
 *       2×2双字母编码/解码和自定义字母表。
 *
 * 协作: PlayfairCipher2(Playfair) / VigenereCipher3(Vigenere) / HillCipher4(Hill)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 四方密码处理器(配对Polybius方阵)
 */
class FourSquareCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FourSquareCode(QObject *parent = nullptr);
    ~FourSquareCode() override;

    void setKey1(const QString& key);
    void setKey2(const QString& key);
    void setAlphabet(const QString& alpha);

    /** @brief 加密明文 */
    QString encrypt(const QString& plaintext);

    /** @brief 解密密文 */
    QString decrypt(const QString& ciphertext);

    /** @brief 生成Polybius方阵 */
    QVector<QVector<QChar>> buildSquare(const QString& key) const;

    /** @brief 在方阵中查找字符位置 */
    QPair<int, int> findInSquare(const QVector<QVector<QChar>>& square,
                                  QChar ch) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length);

private:
    QString m_key1;
    QString m_key2;
    QString m_alphabet;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Standard 5x5 alphabet (I/J merged) */
    static QString defaultAlphabet();

    /** @brief Remove duplicates and map I->J */
    QString processKey(const QString& key) const;

    /** @brief Prepare text: uppercase, replace J->I, pad digraphs */
    QString prepareText(const QString& text) const;
};
