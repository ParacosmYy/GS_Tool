/**
 * @file BazeleriesCode6.h
 * @brief Bazeleries密码(对角转置与双列置换增强多表编码) — Bazeleries Cipher with Diagonal Transposition and Double Columnar Rearrangement for Enhanced Polyalphabetic Encoding
 *
 * 功能: 实现Bazeleries密码(Bazeleries cipher)，采用对角转置(diagonal transposition)
 *       与双列置换(double columnar rearrangement)实现增强多表编码(enhanced polyalphabetic encoding)。
 *
 * 协作: VigenereCipher5(维吉尼亚) / PlayfairCipher4(Playfair) / RailFenceCipher3(栅栏密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Bazeleries密码(对角转置与双列置换增强多表编码)
 */
class BazeleriesCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int keyLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BazeleriesCode6(QObject *parent = nullptr);
    ~BazeleriesCode6() override;

    /** @brief Set the keyword for columnar transposition */
    void setKey(const QString& key);

    /** @brief Set secondary key for double columnar phase */
    void setSecondKey(const QString& key);

    /** @brief Encrypt plaintext using Bazeleries cipher */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using Bazeleries cipher */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherDone(int inputLen, int keyLen, double timeMs);

private:
    QString m_key;
    QString m_secondKey;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate alphabetical order from key (for column ordering) */
    QVector<int> keyOrder(const QString& key) const;

    /** @brief Inverse key order for decryption */
    QVector<int> inverseOrder(const QVector<int>& order) const;

    /** @brief Diagonal transposition write into grid */
    QVector<QVector<QChar>> diagonalWrite(const QString& text, int cols) const;

    /** @brief Diagonal transposition read from grid */
    QString diagonalRead(const QVector<QVector<QChar>>& grid, int cols) const;

    /** @brief Columnar transposition with given column order */
    QString columnarTransposition(const QString& text, const QVector<int>& order) const;

    /** @brief Inverse columnar transposition */
    QString columnarInverse(const QString& text, const QVector<int>& order) const;

    /** @brief Remove non-alpha and convert to uppercase */
    QString normalize(const QString& text) const;
};
