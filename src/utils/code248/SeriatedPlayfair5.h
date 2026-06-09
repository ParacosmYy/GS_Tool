/**
 * @file SeriatedPlayfair5.h
 * @brief 序列化普莱费尔密码(随机化Polybius方格填充+双字母有向图可逆性验证) — Seriated Playfair with Randomized Polybius Square Filling and Double-Letter Digraph with Reversibility Verification
 *
 * 功能: 实现序列化普莱费尔密码(Seriated Playfair Cipher)，使用随机化
 *       Polybius方格填充(randomized square filling)增强密钥空间，通过
 *       双字母有向图(double-letter digraph)编码并验证加密-解密可逆性。
 *
 * 协作: Aes256Cipher1(AES加密) / RsaSign8(RSA签名) / Base64Codec2(Base64编解码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QChar>

/**
 * @brief 序列化普莱费尔密码(随机方格+可逆性验证)
 */
class SeriatedPlayfair5 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher operation result */
    struct CipherResult {
        QString output;
        bool reversible = false;
        int numDigraphs = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        int numDigraphsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SeriatedPlayfair5(QObject *parent = nullptr);
    ~SeriatedPlayfair5() override;

    /** @brief Set keyword for Polybius square generation */
    void setKeyword(const QString& keyword);

    /** @brief Set seriation period (column group size) */
    void setSeriationPeriod(int period);

    /** @brief Encrypt plaintext with reversibility verification */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    CipherResult decrypt(const QString& ciphertext);

    /** @brief Verify encrypt-decrypt round trip */
    bool verifyReversibility(const QString& plaintext);

    /** @brief Get current 5x5 Polybius square */
    QVector<QVector<QChar>> square() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(const QString& operation, int numDigraphs, double timeMs);

private:
    QString m_keyword;
    int m_period = 5;

    QVector<QVector<QChar>> m_square; // 5x5 Polybius
    QVector<int> m_rowOf;             // Letter -> row in square
    QVector<int> m_colOf;             // Letter -> col in square

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build 5x5 Polybius square from keyword */
    void buildSquare();

    /** @brief Find position of a letter in the square */
    bool findPosition(QChar ch, int& row, int& col) const;

    /** @brief Encode a single digraph (encrypt mode) */
    QPair<QChar, QChar> encodeDigraph(QChar a, QChar b) const;

    /** @brief Decode a single digraph (decrypt mode) */
    QPair<QChar, QChar> decodeDigraph(QChar a, QChar b) const;

    /** @brief Prepare text: upper-case, replace J->I, pad double letters */
    QString prepareText(const QString& text) const;

    /** @brief Break prepared text into digraphs */
    QVector<QPair<QChar, QChar>> makeDigraphs(const QString& text) const;

    /** @brief Apply seriation columnar transposition */
    QString applySeriation(const QString& text, int period) const;

    /** @brief Reverse seriation columnar transposition */
    QString reverseSeriation(const QString& text, int period) const;

    /** @brief Map letter to index (A=0, B=1, ..., Z=25, J mapped to I) */
    int letterIndex(QChar ch) const;
};
