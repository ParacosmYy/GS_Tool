/**
 * @file PlayfairCode.h
 * @brief Playfair密码(二连字替换+密钥方阵+频率分析) — Playfair Cipher with Digraph Substitution, Key Square Generation and Frequency-based Cryptanalysis
 *
 * 功能: 实现Playfair密码，支持5×5密钥方阵生成、二连字替换加解密、
 *       频率分析和基于n-gram统计的密码破解。
 *
 * 协作: AesCipher3(AES) / Rc4Code3(RC4) / RsaCode3(RSA)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief Playfair密码处理器(二连字替换+频率分析)
 */
class PlayfairCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncrypts = 0;
        quint64 totalDecrypts = 0;
        int keyLength = 0;
        int textSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PlayfairCode(QObject *parent = nullptr);
    ~PlayfairCode() override;

    /** @brief 设置密钥并生成5×5方阵(J合并到I) */
    void setKey(const QString& key);

    /** @brief 加密明文 */
    QString encrypt(const QString& plaintext);

    /** @brief 解密密文 */
    QString decrypt(const QString& ciphertext);

    /** @brief 从密文频率分析推测密钥(简化模拟退火) */
    QString analyzeFrequency(const QString& ciphertext, int maxIterations = 5000);

    /** @brief 计算文本与英文bigram频率的适配度 */
    double fitnessScore(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptCompleted(int textSize);
    void decryptCompleted(int textSize);

private:
    QChar m_square[5][5];       ///< 5×5 key square
    bool m_keySet = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 预处理文本: 大写、去非字母、J→I、插入X */
    QString preprocess(const QString& text) const;

    /** @brief 在方阵中查找字符位置 */
    QPair<int, int> findChar(QChar c) const;

    /** @brief 加密/解密一对字符 */
    QPair<QChar, QChar> processPair(QChar a, QChar b, bool encrypt) const;

    /** @brief 生成打乱方阵(用于频率分析) */
    void generateRandomSquare();

    /** @brief 从方阵生成对应的字母密钥 */
    QString squareToKey() const;

    /** @brief 交换方阵中两行/两列/两字符 */
    void swapInSquare(int r1, int c1, int r2, int c2);
};
