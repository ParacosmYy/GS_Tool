/**
 * @file SubstitutionCode.h
 * @brief 单表替换密码(频率分析+爬山自动求解) — Monoalphabetic Substitution Cipher with Frequency Analysis and Hill-Climbing Auto-Solve
 *
 * 功能: 实现单表替换密码的加密/解密，支持英文字母频率分析、
 *       爬山算法自动密钥搜索和评分。
 *
 * 协作: VigenereCipher(维吉尼亚) / AesEngine(AES) / XorCipher(XOR)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @brief 单表替换密码处理器
 */
class SubstitutionCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncrypts = 0;          ///< 累计加密次数
        quint64 totalDecrypts = 0;          ///< 累计解密次数
        quint64 totalAutoSolves = 0;        ///< 累计自动求解次数
        double lastScore = 0.0;             ///< 最近评分
        int lastIterations = 0;             ///< 最近迭代次数
        double avgProcessingTimeMs = 0.0;   ///< 平均耗时(ms)
    };

    explicit SubstitutionCode(QObject *parent = nullptr);
    ~SubstitutionCode() override;

    /** @brief 设置加密密钥(26字母排列) */
    void setKey(const QString& key);

    /** @brief 加密文本 */
    QString encrypt(const QString& plaintext) const;

    /** @brief 解密文本 */
    QString decrypt(const QString& ciphertext) const;

    /** @brief 英文字母频率分析 */
    QMap<QChar, double> frequencyAnalysis(const QString& text) const;

    /** @brief 爬山算法自动求解密钥 */
    QString autoSolve(const QString& ciphertext, int maxIterations = 5000);

    /** @brief 评估解密质量(quadgram评分) */
    double scoreText(const QString& text) const;

    /** @brief 生成随机密钥 */
    QString generateRandomKey() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptCompleted(int length);
    void decryptCompleted(int length);
    void autoSolveProgress(int iteration, double score);

private:
    /** @brief 应用替换表 */
    QString applySubstitution(const QString& text,
                              const QVector<int>& table,
                              bool encrypt) const;

    /** @brief 生成初始频率猜测密钥 */
    QVector<int> frequencyGuess(const QString& ciphertext) const;

    /** @brief 随机交换密钥中两个位置 */
    static void swapKeyPositions(QVector<int>& key);

    /** @brief 标准英文字母频率 */
    static const double s_englishFreq[26];

    /** @brief 四元组评分表(简化版) */
    static double bigramScore(const QString& text);

    QVector<int> m_encryptTable;    ///< 加密替换表 [0..25]->[0..25]
    QVector<int> m_decryptTable;    ///< 解密替换表

    Stats m_stats;
    double m_timeSum = 0.0;
};
