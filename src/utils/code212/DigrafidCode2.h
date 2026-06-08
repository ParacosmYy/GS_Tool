/**
 * @file DigrafidCode2.h
 * @brief Digrafid密码(二重分数化+双密钥空间列置换) — Digrafid Cipher with Digraphic Fractionation and Columnar Transposition with Dual Key Space
 *
 * 功能: 实现Digrafid密码，支持双图分数化、
 *       3×3表格映射和双密钥空间列置换。
 *
 * 协作: PlayfairCipher1(Playfair密码) / BifidCipher2(Bifid密码) / ADFGVX3(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief Digrafid密码(二重分数化+双密钥列置换)
 */
class DigrafidCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int period = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DigrafidCode2(QObject *parent = nullptr);
    ~DigrafidCode2() override;

    /** @brief Set keys and period. Keys must be 27-char alphabet permutations */
    void setKeys(const QString& horizontalKey, const QString& verticalKey,
                 int period);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Fractionate a digraph into two trits each */
    QPair<int, int> fractionate(QChar a, QChar b) const;

    /** @brief Defractionate two trits back to a digraph */
    QPair<QChar, QChar> defractionate(int h, int v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_hKey;   // Horizontal key (27 chars)
    QString m_vKey;   // Vertical key (27 chars)
    int m_period = 7;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build 3x3x3 coordinate from key position */
    static QVector<int> keyPosition(QChar c, const QString& key);

    /** @brief Columnar transposition on trit sequence */
    QVector<int> columnarTranspose(const QVector<int>& trits,
                                   const QString& key, bool encrypt) const;

    /** @brief Normalize text: uppercase, replace J with I, pad */
    static QString normalize(const QString& text);
};
