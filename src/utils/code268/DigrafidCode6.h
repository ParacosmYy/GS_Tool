/**
 * @file DigrafidCode6.h
 * @brief 二元组密码(二合字母列映射与分数位置编码多表替换) — Digrafid Cipher with Digraph-to-Column Mapping and Fractional Position Encoding for Polygraphic Substitution
 *
 * 功能: 实现二元组密码(Digrafid cipher)，采用二合字母列映射(digraph-to-column mapping)
 *       与分数位置编码(fractional position encoding)实现多表替换(polygraphic substitution)。
 *
 * 协作: PlayfairCipher4(Playfair密码) / BifidCipher5(Bifid密码) / TrifidCipher6(Trifid密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QChar>

/**
 * @brief 二元组密码(二合字母列映射与分数位置编码多表替换)
 */
class DigrafidCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int gridWidth = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DigrafidCode6(QObject *parent = nullptr);
    ~DigrafidCode6() override;

    /** @brief Set the alphabet (must contain unique characters) */
    void setAlphabet(const QString& alphabet);

    /** @brief Set grid width for fractional encoding */
    void setGridWidth(int width);

    /** @brief Set period (0 = no period, full text) */
    void setPeriod(int period);

    /** @brief Encrypt plaintext using Digrafid cipher */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using Digrafid cipher */
    QString decrypt(const QString& ciphertext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(bool encrypt, int inputLen, int outputLen, double timeMs);

private:
    QString m_alphabet = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ#");
    int m_gridWidth = 3;
    int m_period = 0;
    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Map character to fractional (row, col) position */
    QPair<int, int> charToPosition(QChar c) const;

    /** @brief Map (row, col) position back to character */
    QChar positionToChar(int row, int col) const;

    /** @brief Convert text to position pairs for digraph processing */
    QVector<QPair<QPair<int,int>, QPair<int,int>>> textToDigraphPositions(
        const QString& text) const;

    /** @brief Apply fractional transposition on position pairs */
    QVector<QPair<int,int>> fractionate(
        const QVector<QPair<QPair<int,int>, QPair<int,int>>>& digraphs) const;

    /** @brief Reverse fractional transposition for decryption */
    QVector<QPair<QPair<int,int>, QPair<int,int>>> defractionate(
        const QVector<QPair<int,int>>& positions) const;
};
