/**
 * @file DigrafidCode7.h
 * @brief 二图字密码(扩展二图矩阵与列键分数编码的多表替代) — Digrafid Cipher with Extended Digraph Matrix and Column-keyed Fractional Encoding for Polygraphic Substitution
 *
 * 功能: 实现二图字密码(Digrafid cipher)，采用扩展二图矩阵(extended digraph matrix)
 *       与列键分数编码(column-keyed fractional encoding)实现多表替代(polygraphic substitution)。
 *
 * 协作: Playfair6(Playfair密码) / ADFGVX5(ADFGVX密码) / Bifid5(Bifid密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 二图字密码(扩展二图矩阵与列键分数编码)
 */
class DigrafidCode7 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher result */
    struct CipherResult {
        QString text;
        int numDigraphs = 0;
        int periodUsed = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncoded = 0;
        int numDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DigrafidCode7(QObject *parent = nullptr);
    ~DigrafidCode7() override;

    void setKey(const QString& key);
    void setPeriod(int period);

    CipherResult encode(const QString& plaintext);
    CipherResult decode(const QString& ciphertext);

    /** @brief Get the current 9x9 digraph matrix */
    QVector<QVector<int>> digraphMatrix() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeDone(int n, int period, double timeMs);
    void decodeDone(int n, int period, double timeMs);

private:
    QString m_key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ#";
    int m_period = 5;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 9x9 digraph substitution table (row*9+col -> cipher index) */
    QVector<QVector<int>> m_matrix;

    /** @brief Character to index mapping (27 chars -> 0..26) */
    QVector<int> m_charToIdx;

    /** @brief Rebuild the matrix from key */
    void rebuildMatrix();

    /** @brief Map character to 0..26 */
    int charIndex(QChar c) const;

    /** @brief Map index back to character */
    QChar indexChar(int idx) const;

    /** @brief Prepare text: uppercase, replace J->I, pad to even length */
    QString prepare(const QString& text) const;

    /** @brief Fractional encode a pair of indices */
    int fractionate(int row, int col) const;

    /** @brief Defractionate a cipher index back to (row, col) */
    QPair<int, int> defractionate(int cipher) const;
};
