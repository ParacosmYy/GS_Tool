/**
 * @file AffineCode.h
 * @brief 仿射密码(ax+b mod 26模逆验证+穷举密钥分析) — Affine Cipher (ax+b mod 26) with Modular Inverse Validation and Exhaustive Key Cryptanalysis
 *
 * 功能: 实现仿射密码加密/解密(ax+b mod 26)，支持模逆元验证、
 *       密钥合法性检查和穷举法密钥分析。
 *
 * 协作: CaesarCode1(凯撒密码) / VigenereCode2(维吉尼亚密码) / HillCipher3(Hill密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief 仿射密码(ax+b mod 26)处理器
 */
class AffineCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numEncrypts = 0;
        int numDecrypts = 0;
        int numCryptanalysis = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 密钥结构 */
    struct Key {
        int a = 1; ///< Multiplicative coefficient (must be coprime to 26)
        int b = 0; ///< Additive shift
    };

    /** @brief 密码分析结果 */
    struct AnalysisResult {
        Key key;
        QString plaintext;
        double score = 0.0; ///< Fitness score based on frequency analysis
    };

    explicit AffineCode(QObject *parent = nullptr);
    ~AffineCode() override;

    /** @brief 验证密钥a是否与26互素 */
    bool validateKey(int a) const;

    /** @brief 计算a在mod 26下的模逆元 */
    int modInverse(int a) const;

    /** @brief 最大公约数 */
    int gcd(int a, int b) const;

    /** @brief 加密 */
    QString encrypt(const QString& plaintext, const Key& key) const;

    /** @brief 解密 */
    QString decrypt(const QString& ciphertext, const Key& key) const;

    /** @brief 穷举法密钥分析(基于频率评分) */
    QVector<AnalysisResult> exhaustiveCryptanalysis(const QString& ciphertext) const;

    /** @brief 频率分析评分(英文字母频率匹配度) */
    double frequencyScore(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int textLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief English letter frequencies (A-Z) */
    static constexpr double s_enFreq[26] = {
        0.0817, 0.0150, 0.0278, 0.0425, 0.1270, 0.0223, 0.0202,
        0.0609, 0.0697, 0.0015, 0.0077, 0.0403, 0.0241, 0.0675,
        0.0751, 0.0193, 0.0010, 0.0599, 0.0633, 0.0906, 0.0276,
        0.0098, 0.0236, 0.0015, 0.0197, 0.0007
    };

    /** @brief Valid a values coprime to 26 */
    static constexpr int s_validA[] = {1,3,5,7,9,11,15,17,19,21,23,25};

    /** @brief Normalize character to 0-25 */
    int charToIdx(QChar c) const;

    /** @brief Convert index back to character */
    QChar idxToChar(int idx, bool upper) const;
};
