/**
 * @file SeriatedPlayfair6.h
 * @brief 序列化Playfair密码(Bifid分数化转置嵌入序列化增强有向图扰乱) — Seriated Playfair with Bifid Fractionation and Transposition-embedded Seriation for Enhanced Digraph Disruption
 *
 * 功能: 实现序列化Playfair密码(Seriated Playfair cipher)，采用Bifid分数化(bifid
 *       fractionation)和转置嵌入序列化(transposition-embedded seriation)增强有向图
 *       扰乱(digraph disruption)。
 *
 * 协作: HillCipher5(Hill密码) / VigenereCipher4(Vigenere密码) / ADFGXCipher4(ADFGX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 序列化Playfair密码(Bifid分数化转置嵌入序列化增强有向图扰乱)
 */
class SeriatedPlayfair6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int keyLength = 0;
        int period = 0;
        int textSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SeriatedPlayfair6(QObject *parent = nullptr);
    ~SeriatedPlayfair6() override;

    /** @brief Set keyword for Playfair square and seriation period */
    void setKey(const QString& keyword, int period = 5);

    /** @brief Encrypt plaintext using seriated Playfair */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using seriated Playfair */
    QString decrypt(const QString& ciphertext);

    /** @brief Get the current 5x5 key square as flat string */
    QString keySquare() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherUpdated(int textSize, int period, double timeMs);

private:
    int m_period = 5;
    QString m_square;   // 25-char key square (I/J merged)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build 5x5 key square from keyword */
    void buildSquare(const QString& keyword);

    /** @brief Find position of char in square, returns (row, col) */
    QPair<int, int> findChar(QChar c) const;

    /** @brief Encode a digraph using standard Playfair rules */
    QPair<QChar, QChar> encodeDigraph(QChar a, QChar b) const;

    /** @brief Decode a digraph using standard Playfair rules */
    QPair<QChar, QChar> decodeDigraph(QChar a, QChar b) const;

    /** @brief Prepare text: uppercase, replace J->I, pad */
    QString prepareText(const QString& text) const;

    /** @brief Apply bifid fractionation and seriation */
    QString applySeriation(const QString& text, bool encrypt) const;
};
