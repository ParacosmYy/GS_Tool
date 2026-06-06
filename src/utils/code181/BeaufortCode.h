/**
 * @file BeaufortCode.h
 * @brief 博福特密码(互惠替代密码+重合指数法周期密钥检测) — Beaufort Cipher (Reciprocal Substitution) with Periodic Key Detection via Index of Coincidence
 *
 * 功能: 实现博福特密码编解码、基于重合指数法的密钥周期检测、
 *       自动密钥长度推测和密钥恢复。
 *
 * 协作: VigenereCipher5(维吉尼亚) / CaesarCipher3(凯撒) / AutokeyCipher4(自动密钥)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief 博福特密码处理器(互惠替代+重合指数法)
 */
class BeaufortCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int detectedKeyLength = 0;
        double indexOfCoincidence = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BeaufortCode(QObject *parent = nullptr);
    ~BeaufortCode() override;

    /** @brief 加密文本 */
    QString encrypt(const QString& plaintext, const QString& key) const;

    /** @brief 解密文本(与加密相同，互惠密码) */
    QString decrypt(const QString& ciphertext, const QString& key) const;

    /** @brief 计算重合指数(IC) */
    double indexOfCoincidence(const QString& text) const;

    /** @brief 检测密钥长度(重合指数法) */
    QVector<QPair<int, double>> detectKeyLength(const QString& ciphertext, int maxKeyLen = 20) const;

    /** @brief 自动分析密钥 */
    QString autoAnalyzeKey(const QString& ciphertext, int keyLength) const;

    /** @brief 字母频率分析 */
    QVector<QPair<QChar, double>> frequencyAnalysis(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int textLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief English letter frequencies for key recovery */
    static const double s_englishFreq[26];

    /** @brief Apply Beaufort transform to single character */
    QChar beaufortChar(QChar plain, QChar key) const;

    /** @brief Extract alphabetic characters to upper-case */
    QString toAlphaUpper(const QString& text) const;

    /** @brief Chi-squared score against English frequencies */
    double chiSquared(const QVector<int>& counts, int total) const;
};
