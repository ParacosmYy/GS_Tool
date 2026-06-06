/**
 * @file BifidCode.h
 * @brief 双分密码(Polybius方格分数化+坐标转置) — Bifid Cipher with Polybius Fractionation and Coordinate Transposition for Periodic Key
 *
 * 功能: 实现Bifid密码，支持Polybius方格分数化、周期密钥坐标转置、
 *       加密解密和自定义字母表配置。
 *
 * 协作: VigenereCipher3(维吉尼亚) / PlayfairCipher1(Playfair) / ADFGXCipher2(ADFGX)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief Bifid密码处理器(Polybius分数化+坐标转置)
 */
class BifidCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int inputLength = 0;
        int period = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BifidCode(QObject *parent = nullptr);
    ~BifidCode() override;

    void setPeriod(int period);
    void setAlphabet(const QString& alpha);

    /** @brief 加密明文 */
    QString encrypt(const QString& plaintext, const QString& key);

    /** @brief 解密密文 */
    QString decrypt(const QString& ciphertext, const QString& key);

    /** @brief 生成Polybius方格 */
    QVector<QVector<QChar>> buildPolybiusSquare(const QString& key) const;

    /** @brief 字符转坐标 */
    QPair<int, int> charToCoord(QChar c, const QVector<QVector<QChar>>& square) const;

    /** @brief 坐标转字符 */
    QChar coordToChar(int row, int col, const QVector<QVector<QChar>>& square) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length);

private:
    int m_period = 5;
    QString m_alphabet;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Prepare text: remove non-alpha, replace J with I */
    QString prepareText(const QString& text) const;

    /** @brief Filter coordinates through period blocks */
    QVector<int> fractionate(const QVector<int>& coords, int period) const;

    /** @brief Defractionate coordinates through period blocks */
    QVector<int> defractionate(const QVector<int>& coords, int period) const;
};
