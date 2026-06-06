/**
 * @file ADFGVXCode.h
 * @brief ADFGVX分馏密码(Polybius方阵+列置换+已知明文攻击) — ADFGVX Fractionation Cipher with Polybius Square, Columnar Transposition and Known-Plaintext Attack
 *
 * 功能: 实现ADFGVX密码，支持6×6 Polybius方阵替换、列置换加密/解密
 *       和基于已知明文的密钥恢复攻击。
 *
 * 协作: PlayfairCipher6(Playfair) / EnigmaMachine7(Enigma) / VigenereCipher5(Vigenere)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief ADFGVX分馏密码处理器
 */
class ADFGVXCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncryptions = 0;
        quint64 totalDecryptions = 0;
        int polybiusSize = 6;
        int transpositionKeyLen = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ADFGVXCode(QObject *parent = nullptr);
    ~ADFGVXCode() override;

    /** @brief 设置Polybius方阵填充字符(A-Z + 0-9 = 36) */
    void setPolybiusKey(const QString& key);

    /** @brief 设置列置换密钥 */
    void setTranspositionKey(const QString& key);

    /** @brief 加密: Polybius替换 + 列置换 */
    QString encrypt(const QString& plaintext);

    /** @brief 解密: 逆列置换 + 逆Polybius */
    QString decrypt(const QString& ciphertext);

    /** @brief 已知明文攻击: 推断置换密钥 */
    QVector<QString> knownPlaintextAttack(const QString& ciphertext,
                                            const QString& knownPt) const;

    /** @brief 暴力搜索: 尝试所有短密钥长度 */
    QPair<QString, QString> bruteForceKey(const QString& ciphertext,
                                            const QString& knownPt,
                                            int maxKeyLen) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int length);
    void decryptionCompleted(int length);

private:
    QString m_polybiusKey;
    QString m_transKey;

    /** @brief 6x6 Polybius square grid */
    QVector<QVector<QChar>> m_grid;

    /** @brief ADFGVX coordinate labels */
    static constexpr const char* LABELS = "ADFGVX";

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build 6x6 Polybius square from key */
    void buildGrid();

    /** @brief Columnar transposition encrypt */
    QString columnarEncrypt(const QString& fractionated) const;

    /** @brief Columnar transposition decrypt */
    QString columnarDecrypt(const QString& ct, int keyLen) const;

    /** @brief Read columns in key-sorted order */
    QVector<int> keyOrder(const QString& key) const;

    /** @brief Score a candidate decryption against known plaintext */
    double scoreDecryption(const QString& candidate,
                            const QString& known) const;
};
