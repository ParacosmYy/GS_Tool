/**
 * @file NihilistCode.h
 * @brief Nihilist密码(Polybius方阵+数字密钥加法+统计密码分析) — Nihilist Cipher with Polybius Key Square, Numeric Key Addition and Statistical Cryptanalysis
 *
 * 功能: 实现Nihilist密码，支持Polybius方阵生成、数字密钥加法加密、
 *       频率统计分析和已知明文攻击。
 *
 * 协作: VigenereCipher3(维吉尼亚) / PlayfairCipher2(Playfair) / AffineCipher1(仿射)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief Nihilist密码处理器(Polybius方阵+数字密钥)
 */
class NihilistCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int textSize = 0;
        int keyLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NihilistCode(QObject *parent = nullptr);
    ~NihilistCode() override;

    void setKeyPhrase(const QString& phrase);

    /** @brief 加密文本 */
    QVector<int> encrypt(const QString& plaintext);

    /** @brief 解密密文 */
    QString decrypt(const QVector<int>& ciphertext);

    /** @brief 生成Polybius方阵(5x5, J=I合并) */
    QVector<QVector<int>> polybiusSquare() const;

    /** @brief 统计密码分析: 推断密钥长度 */
    int analyzeKeyLength(const QVector<int>& ciphertext) const;

    /** @brief 频率分析攻击 */
    QVector<double> frequencyAnalysis(const QVector<int>& ciphertext) const;

    /** @brief 已知明文攻击: 恢复部分密钥 */
    QVector<int> knownPlaintextAttack(const QVector<int>& ciphertext,
                                      const QString& known) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int size);

private:
    QString m_keyPhrase;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build 5x5 Polybius square from key */
    void buildSquare(QVector<QVector<QChar>>& square,
                     QVector<int>& charToNum) const;

    /** @brief Convert char to Polybius number (11-55) */
    int charToNum(QChar c, const QVector<QVector<QChar>>& square) const;

    /** @brief Convert Polybius number back to char */
    QChar numToChar(int num, const QVector<QVector<QChar>>& square) const;

    /** @brief Expand key to numeric sequence */
    QVector<int> expandKey(int length,
                           const QVector<QVector<QChar>>& square) const;

    /** @brief Index of coincidence for key length detection */
    double indexOfCoincidence(const QVector<int>& ct, int period) const;
};
