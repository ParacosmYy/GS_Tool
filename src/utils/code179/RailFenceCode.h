/**
 * @file RailFenceCode.h
 * @brief 栅栏密码(可配置轨道数+锯齿模式+暴力破解分析) — Rail Fence Cipher with Configurable Rail Count, Zigzag Pattern and Brute-force Rank Cryptanalysis
 *
 * 功能: 实现栅栏密码编解码，支持可配置轨道数、锯齿形(zigzag)读写模式、
 *       暴力穷举所有轨道数进行密码分析和频率评分。
 *
 * 协作: CaesarCipher3(凯撒密码) / VigenereCode3(维吉尼亚) / A51Cipher3(A5/1流密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 栅栏密码编解码器(可配置轨道+锯齿模式+暴力破解)
 */
class RailFenceCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncryptions = 0;
        quint64 totalDecryptions = 0;
        int lastRails = 0;
        int lastLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 破解结果条目 */
    struct CrackResult {
        int rails;              ///< Number of rails used
        QString plaintext;      ///< Decrypted candidate text
        double score;           ///< Frequency-based fitness score
    };

    explicit RailFenceCode(QObject *parent = nullptr);
    ~RailFenceCode() override;

    void setRails(int rails);

    /** @brief 加密明文 */
    QString encrypt(const QString& plaintext);

    /** @brief 解密密文(已知轨道数) */
    QString decrypt(const QString& ciphertext);

    /** @brief 暴力破解: 穷举轨道数并评分 */
    QVector<CrackResult> bruteForceCrack(const QString& ciphertext,
                                          int maxRails = 0);

    /** @brief 频率评分(英文字母频率匹配度) */
    double frequencyScore(const QString& text) const;

    /** @brief 生成锯齿轨道矩阵 */
    QVector<QVector<QChar>> buildZigzagMatrix(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int rails, int length);
    void decryptionCompleted(int rails, int length);

private:
    int m_rails = 3;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Read off rails row-by-row */
    QString readOffRails(const QVector<QVector<QChar>>& matrix) const;

    /** @brief Reconstruct zigzag pattern from rail-read ciphertext */
    QString decodeFromRails(const QString& ciphertext, int rails) const;

    /** @brief English letter frequency table */
    static const QVector<double> s_englishFreq;
};
