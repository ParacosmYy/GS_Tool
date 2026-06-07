/**
 * @file GronsfeldCode.h
 * @brief Gronsfeld密码(数字密钥Vigenere变体+叠合攻击) — Gronsfeld Cipher with Numeric-Key Vigenere Variant and Superimposition Cryptanalysis
 *
 * 功能: 实现Gronsfeld密码的加密解密，支持数字密钥多表替换、
 *       叠合分析密钥恢复、Kasiski密钥长度推断和频率分析。
 *
 * 协作: PlayfairCipher3(Playfair密码) / HillCipher5(希尔密码) / VigenereCode6(Vigenere)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Gronsfeld密码处理器(数字密钥+叠合攻击)
 */
class GronsfeldCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int keyLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GronsfeldCode(QObject *parent = nullptr);
    ~GronsfeldCode() override;

    /** @brief 设置数字密钥(0-9序列) */
    void setKey(const QVector<int>& key);

    /** @brief 加密文本 */
    QString encrypt(const QString& plaintext) const;

    /** @brief 解密文本 */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Kasiski检验推断密钥长度 */
    QVector<QPair<int, int>> kasiskiExamination(const QString& ciphertext) const;

    /** @brief 叠合攻击恢复密钥 */
    QVector<int> superimpositionAttack(const QString& ciphertext, int keyLen) const;

    /** @brief 频率分析(按偏移位置) */
    QVector<QVector<double>> frequencyAnalysis(const QString& text, int keyLen) const;

    /** @brief 计算文本重合指数 */
    double indexOfCoincidence(const QString& text) const;

    const QVector<int>& key() const { return m_key; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, double timeMs);

private:
    QVector<int> m_key;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief English letter frequency table (A-Z) */
    static const QVector<double> s_englishFreq;

    /** @brief Shift a single character */
    QChar shiftChar(QChar c, int shift, bool encrypt) const;

    /** @brief Normalize text: uppercase, letters only */
    QString normalize(const QString& text) const;

    /** @brief Chi-squared test against English frequencies */
    double chiSquared(const QVector<int>& counts) const;
};
