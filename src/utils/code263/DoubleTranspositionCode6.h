/**
 * @file DoubleTranspositionCode6.h
 * @brief 双重置换密码(Nihilist复合列置换与关键字栅栏分层排列) — Double Transposition Cipher with Nihilist-style Compound Columnar and Keyword-driven Rail Fence for Layered Permutation
 *
 * 功能: 实现双重置换密码(Double transposition cipher)，采用Nihilist风格复合列置换
 *       (Nihilist-style compound columnar)和关键字驱动栅栏(keyword-driven rail fence)
 *       实现分层排列(layered permutation)。
 *
 * 协作: AesEncryption4(AES加密) / Rc4Cipher3(RC4流密码) / XorCipher2(XOR密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 双重置换密码(Nihilist复合列置换与关键字栅栏分层排列)
 */
class DoubleTranspositionCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int numRows = 0;
        int numCols = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DoubleTranspositionCode6(QObject *parent = nullptr);
    ~DoubleTranspositionCode6() override;

    /** @brief Set column keyword for first columnar transposition */
    void setColumnKeyword(const QString& keyword);

    /** @brief Set row keyword for second columnar transposition (rail fence layer) */
    void setRowKeyword(const QString& keyword);

    /** @brief Set rail fence rails for interleaving layer */
    void setRailFenceRails(int rails);

    /** @brief Encrypt plaintext via double transposition */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext via inverse double transposition */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherApplied(int inputLength, bool encrypt, double timeMs);

private:
    QString m_colKeyword;
    QString m_rowKeyword;
    int m_rails = 3;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Derive column order from keyword (stable sort indices) */
    QVector<int> keywordOrder(const QString& keyword) const;

    /** @brief Columnar transposition encrypt */
    QString columnarEncrypt(const QString& text, const QString& keyword) const;

    /** @brief Columnar transposition decrypt */
    QString columnarDecrypt(const QString& text, const QString& keyword) const;

    /** @brief Rail fence encrypt with given rails */
    QString railFenceEncrypt(const QString& text, int rails) const;

    /** @brief Rail fence decrypt with given rails */
    QString railFenceDecrypt(const QString& text, int rails) const;

    /** @brief Remove non-alpha and convert to uppercase */
    static QString normalize(const QString& text);
};
