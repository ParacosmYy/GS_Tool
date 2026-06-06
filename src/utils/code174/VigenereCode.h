/**
 * @file VigenereCode.h
 * @brief 维吉尼亚密码(编码解码+Kasiski/Friedman密钥长度分析) — Vigenere Cipher Encoder/Decoder with Kasiski Examination and Friedman Test
 *
 * 功能: 实现维吉尼亚密码编解码，支持Kasiski测试和Friedman测试
 *       自动推测密钥长度、频率分析破解。
 *
 * 协作: CaesarCipher(凯撒密码) / AesCodec(AES编解码) / Base64Converter(Base64)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @brief 维吉尼亚密码编解码器(含密钥分析)
 */
class VigenereCode : public QObject {
    Q_OBJECT

public:
    /** @brief 密钥长度分析结果 */
    struct KeyLengthResult {
        int keyLength = 0;              ///< 推测的密钥长度
        double friedmanIC = 0.0;        ///< Friedman重合指数
        QVector<QPair<int, int>> kasiskiFactors; ///< Kasiski因子(因子,频次)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncryptions = 0;       ///< 累计加密次数
        quint64 totalDecryptions = 0;       ///< 累计解密次数
        quint64 totalAnalyses = 0;          ///< 累计分析次数
        double avgProcessingTimeMs = 0.0;   ///< 平均耗时(ms)
    };

    explicit VigenereCode(QObject *parent = nullptr);
    ~VigenereCode() override;

    /** @brief 设置密钥(仅支持A-Z字母) */
    void setKey(const QString& key);

    /** @brief 加密文本 */
    QString encrypt(const QString& plaintext) const;

    /** @brief 解密文本 */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Kasiski检验推测密钥长度 */
    KeyLengthResult kasiskiExamination(const QString& ciphertext) const;

    /** @brief Friedman检验计算重合指数 */
    double friedmanTest(const QString& text) const;

    /** @brief 综合分析: 推测密钥长度并尝试破解 */
    KeyLengthResult analyzeKeyLength(const QString& ciphertext) const;

    /** @brief 通过频率分析推测密钥 */
    QString crackKey(const QString& ciphertext, int keyLength) const;

    /** @brief 自动破解: 分析+推测密钥+解密 */
    QString autoCrack(const QString& ciphertext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int length);
    void decryptionCompleted(int length);
    void analysisCompleted(int keyLength, double ic);

private:
    /** @brief 将字母转为0-25数值 */
    static int charToIndex(QChar c);

    /** @brief 将0-25数值转为字母 */
    static QChar indexToChar(int idx);

    /** @brief 清理文本: 仅保留A-Z大写 */
    static QString cleanText(const QString& text);

    /** @brief 查找重复序列位置(Kasiski) */
    QVector<QPair<int, int>> findRepeats(const QString& text,
                                          int seqLen) const;

    /** @brief 计算GCD */
    static int gcd(int a, int b);

    /** @brief 因数分解 */
    static QVector<int> getFactors(int n);

    /** @brief 计算单字母频率分布 */
    static QMap<QChar, double> frequencyDistribution(const QString& text);

    /** @brief 英语字母频率(用于破解) */
    static const double s_englishFreq[26];

    QString m_key;

    Stats m_stats;
    double m_timeSum = 0.0;
};
