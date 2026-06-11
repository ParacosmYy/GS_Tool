/**
 * @file DigrafidCode8.h
 * @brief 二合字密码(二合字到列映射与双分坐标网格分数替换实现多图加密) — Digrafid Cipher with Digraph-to-column Mapping and Fractional Substitution via Bifurcated Coordinate Grid for Polygraphic Encryption
 *
 * 功能: 实现二合字密码(Digrafid cipher)，采用二合字到列映射(digraph-to-column mapping)
 *       与双分坐标网格分数替换(fractional substitution via bifurcated coordinate grid)实现多图加密(polygraphic encryption)。
 *
 * 协作: PlayfairCipher(Playfair密码) / BifidCipher(Bifid密码) / ADFGVXCipher(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

class DigrafidCode8 : public QObject {
    Q_OBJECT

public:
    /** @brief Encryption result */
    struct EncryptResult {
        QString ciphertext;
        int textLength = 0;
        int numDigraphs = 0;
    };

    /** @brief Decryption result */
    struct DecryptResult {
        QString plaintext;
        int textLength = 0;
        int numDigraphs = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalEncrypts = 0;
        quint64 totalDecrypts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DigrafidCode8(QObject *parent = nullptr);
    ~DigrafidCode8() override;

    /** @brief Set the 3x3 Polybius key (9 unique chars) */
    void setKey(const QString& key);

    /** @brief Set the column transposition keyword */
    void setColumnKey(const QString& colKey);

    /** @brief Encrypt plaintext using Digrafid cipher */
    EncryptResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using Digrafid cipher */
    DecryptResult decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int len, double timeMs);
    void decryptDone(int len, double timeMs);

private:
    QString m_key = QStringLiteral("ABCDEFGHI");
    QString m_columnKey = QStringLiteral("KEY");
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 3x3 Polybius grid for row/column coordinates */
    QVector<QVector<QChar>> m_grid;

    /** @brief Build the 3x3 substitution grid from key */
    void buildGrid();

    /** @brief Find row and column of a character in the grid */
    QPair<int, int> findInGrid(QChar c) const;

    /** @brief Get column key permutation order */
    QVector<int> columnPermutation() const;

    /** @brief Prepare text: uppercase, filter valid chars, pad if needed */
    QString prepareText(const QString& text) const;
};
