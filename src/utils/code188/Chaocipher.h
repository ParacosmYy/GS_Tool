/**
 * @file Chaocipher.h
 * @brief Chaocipher密码(双旋转字母表+位置驱动置换) — Chaocipher with Two Rotating Alphabets and Position-Driven Permutation
 *
 * 功能: 实现Chaocipher加密/解密算法，支持左/右双旋转字母表、
 *       明文/密文字母位置驱动置换、中间字母切分和批量文本处理。
 *
 * 协作: EnigmaMachine3(转子密码) / VigenereCipher2(维吉尼亚) / AesEngine4(AES)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Chaocipher双旋转字母表密码
 */
class Chaocipher : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOps = 0;
        int charsEncrypted = 0;
        int charsDecrypted = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chaocipher(QObject *parent = nullptr);
    ~Chaocipher() override;

    /** @brief 设置左字母表(26字符, 必须是字母排列) */
    void setLeftAlphabet(const QString& alpha);

    /** @brief 设置右字母表(26字符, 必须是字母排列) */
    void setRightAlphabet(const QString& alpha);

    /** @brief 加密文本(仅处理A-Z, 忽略其他字符) */
    QString encrypt(const QString& plaintext);

    /** @brief 解密文本(仅处理A-Z, 忽略其他字符) */
    QString decrypt(const QString& ciphertext);

    /** @brief 获取当前左字母表状态 */
    QString leftAlphabet() const { return m_left; }

    /** @brief 获取当前右字母表状态 */
    QString rightAlphabet() const { return m_right; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void resetAlphabets();

signals:
    void operationCompleted(const QString& op, int numChars, double timeMs);

private:
    QString m_left;
    QString m_right;
    QString m_initialLeft;
    QString m_initialRight;
    Stats m_stats;
    double m_timeSum = 0.0;

    static constexpr int ALPHA_SIZE = 26;
    static constexpr int PVT_POS = 13; // PVT position (zenith)

    /** @brief Validate alphabet is a permutation of A-Z */
    bool validateAlphabet(const QString& alpha) const;

    /** @brief Permute left alphabet after encryption step */
    void permuteLeft(int pos);

    /** @brief Permute right alphabet after encryption step */
    void permuteRight(int pos);

    /** @brief Find character position in string */
    int findPos(const QString& alpha, QChar ch) const;
};
