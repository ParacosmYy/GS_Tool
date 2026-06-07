/**
 * @file DoubleTranspositionCode2.h
 * @brief 双重置换密码(列行乘积密码+基于已知明文的密钥重建) — Double Transposition Cipher with Columnar-Row Product Cipher and Crib-Based Key Reconstruction
 *
 * 功能: 实现双重置换密码，支持列-行乘积加密、
 *       基于已知明文的密钥重建和解密验证。
 *
 * 协作: HillCipher6(Hill密码) / AesEngine8(AES引擎) / XteaCipher5(XTEA密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 双重置换密码(列行乘积密码+基于已知明文的密钥重建)
 */
class DoubleTranspositionCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastInputLen = 0;
        int lastColKeyLen = 0;
        int lastRowKeyLen = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DoubleTranspositionCode2(QObject *parent = nullptr);
    ~DoubleTranspositionCode2() override;

    /** @brief Encrypt using column-then-row transposition */
    QString encrypt(const QString& plaintext,
                    const QVector<int>& colKey,
                    const QVector<int>& rowKey) const;

    /** @brief Decrypt using row-then-column inverse transposition */
    QString decrypt(const QString& ciphertext,
                    const QVector<int>& colKey,
                    const QVector<int>& rowKey) const;

    /** @brief Attempt crib-based key reconstruction */
    QVector<QPair<QVector<int>, QVector<int>>> reconstructKeys(
        const QString& plaintext, const QString& ciphertext,
        int maxColKey, int maxRowKey) const;

    /** @brief Validate that keys are valid permutations */
    static bool validateKey(const QVector<int>& key);

    /** @brief Invert a permutation key */
    static QVector<int> invertKey(const QVector<int>& key);

    /** @brief Score how well a candidate decrypt matches expected */
    double scoreDecryption(const QString& candidate,
                           const QString& expected) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int inputLen, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Single column transposition */
    QString columnTransposition(const QString& text,
                                 const QVector<int>& key, bool encrypt) const;

    /** @brief Single row transposition */
    QString rowTransposition(const QString& text,
                              const QVector<int>& key, bool encrypt) const;

    /** @brief Generate all permutations of given length */
    QVector<QVector<int>> generatePermutations(int len, int maxLen) const;

    /** @brief N-gram frequency scoring for key quality */
    double ngramScore(const QString& text) const;
};
