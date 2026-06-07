/**
 * @file DoubleTranspositionCode.h
 * @brief 双重列置换密码(乘积密码组合+已知明文攻击) — Double Columnar Transposition Cipher with Product Cipher Composition and Crib-Based Attack
 *
 * 功能: 实现双重列置换密码，支持单/双列置换加密解密、
 *       乘积密码组合和基于已知明文(crib)的密钥恢复攻击。
 *
 * 协作: AesEngine3(AES) / RsaCodec5(RSA) / XorCipher4(异或密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 双重列置换密码(乘积组合+已知明文攻击)
 */
class DoubleTranspositionCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int encryptCount = 0;
        int decryptCount = 0;
        int attackCount = 0;
        int key1Length = 0;
        int key2Length = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DoubleTranspositionCode(QObject *parent = nullptr);
    ~DoubleTranspositionCode() override;

    /** @brief Set first and second transposition keys */
    void setKeys(const QString& key1, const QString& key2);

    /** @brief Encrypt plaintext using double columnar transposition */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using double columnar transposition */
    QString decrypt(const QString& ciphertext);

    /** @brief Single columnar transposition encrypt */
    QString singleEncrypt(const QString& text, const QString& key) const;

    /** @brief Single columnar transposition decrypt */
    QString singleDecrypt(const QString& text, const QString& key) const;

    /** @brief Crib-based attack: recover key lengths and partial keys */
    QVector<QPair<QString, QString>> cribAttack(const QString& ciphertext,
                                                 const QString& crib,
                                                 int maxKeyLen = 10);

    /** @brief Derive column order from keyword */
    QVector<int> keyOrder(const QString& key) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int textLen, double timeMs);

private:
    QString m_key1;
    QString m_key2;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Read off columns in key order */
    QString readOffColumns(const QVector<QVector<QChar>>& grid,
                           const QVector<int>& order) const;

    /** @brief Fill grid row-wise */
    QVector<QVector<QChar>> fillRowWise(const QString& text, int cols) const;

    /** @brief Score a candidate key by comparing bigram frequencies */
    double scoreCandidate(const QString& candidate, const QString& crib) const;
};
