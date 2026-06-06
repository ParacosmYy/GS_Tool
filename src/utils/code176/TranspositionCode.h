/**
 * @file TranspositionCode.h
 * @brief 列置换密码(密钥列序+重合指数密码分析) — Columnar Transposition Cipher with Key-based Column Ordering and Cryptanalysis via Index of Coincidence
 *
 * 功能: 实现列置换密码的加密与解密，支持密钥驱动的列排列顺序、
 *       多轮置换和基于重合指数的密码分析攻击。
 *
 * 协作: HuffmanCodec12(哈夫曼) / LzwCodec8(LZW) / AesCodec4(AES)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 列置换密码处理器
 */
class TranspositionCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncryptions = 0;
        quint64 totalDecryptions = 0;
        int lastKeyLength = 0;
        int lastMessageLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TranspositionCode(QObject *parent = nullptr);
    ~TranspositionCode() override;

    /** @brief 设置密钥字符串(字符顺序决定列序) */
    void setKey(const QString& key);

    /** @brief 加密明文 */
    QString encrypt(const QString& plaintext);

    /** @brief 解密密文 */
    QString decrypt(const QString& ciphertext);

    /** @brief 设置加密轮数 */
    void setRounds(int rounds);

    /**
     * @brief 密码分析: 推测密钥长度
     * @param ciphertext 密文
     * @param maxKeyLen 最大候选密钥长度
     * @return 按可能性排序的{密钥长度, 重合指数得分}
     */
    QVector<QPair<int, double>> analyzeKeyLength(const QString& ciphertext,
                                                  int maxKeyLen = 20) const;

    /**
     * @brief 重合指数计算
     * @param text 输入文本
     * @return IC值
     */
    double indexCoincidence(const QString& text) const;

    /**
     * @brief 尝试自动破解
     * @param ciphertext 密文
     * @param maxKeyLen 最大密钥长度
     * @return 候选明文列表
     */
    QVector<QString> crack(const QString& ciphertext, int maxKeyLen = 10) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int messageLength, int keyLength);
    void decryptionCompleted(int messageLength);
    void analysisCompleted(int suggestedKeyLength, double icScore);

private:
    QString m_key;
    int m_rounds = 1;

    /** @brief 从密钥生成列排列顺序 */
    QVector<int> keyOrder() const;

    /** @brief 单轮加密 */
    QString encryptSingle(const QString& text, const QVector<int>& order) const;

    /** @brief 单轮解密 */
    QString decryptSingle(const QString& text, const QVector<int>& order) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
