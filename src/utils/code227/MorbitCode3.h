/**
 * @file MorbitCode3.h
 * @brief 莫尔斯码(扩展摩尔斯符号频率分析+迭代爬山随机扰动) — Morbit Code with Extended Morse-symbol Frequency Analysis and Iterative Hill-climbing with Random Perturbation
 *
 * 功能: 实现Morbit密码编解码，基于扩展摩尔斯符号频率分析，
 *       使用迭代爬山法(hill-climbing)结合随机扰动进行密钥搜索与自动解码。
 *
 * 协作: FrequencyAnalyzer3(频率分析) / SubstitutionCipher2(替换密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief 莫尔斯码(扩展频率分析+爬山随机扰动解码)
 */
class MorbitCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief Decryption result with score */
    struct DecodeResult {
        QString plaintext;
        double score = 0.0;
        QString key;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        int bestScoreIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MorbitCode3(QObject *parent = nullptr);
    ~MorbitCode3() override;

    /** @brief Set the 9-digit Morbit key (digits 1-9, no repeats) */
    bool setKey(const QString& key);

    /** @brief Encrypt plaintext to Morbit ciphertext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext with current key */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Auto-crack: hill-climbing with random perturbation */
    DecodeResult crack(const QString& ciphertext, int maxIter = 5000);

    /** @brief Analyze Morse symbol frequency in ciphertext */
    QMap<QChar, int> frequencyAnalysis(const QString& ciphertext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int length, double timeMs);
    void crackProgress(int iteration, double score);
    void crackCompleted(double bestScore, int iterations);

private:
    QString m_key;
    QMap<QChar, int> m_morseSymbolMap;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Morse code table
    QMap<QChar, QString> m_morseTable;
    QMap<QString, QChar> m_morseReverse;

    /** @brief Build Morse code lookup table */
    void buildMorseTable();

    /** @brief Convert text to Morse dots/dashes */
    QString textToMorse(const QString& text) const;

    /** @brief Score plaintext using English quadgram frequency */
    double scorePlaintext(const QString& text) const;

    /** @brief Generate random Morbit key */
    QString randomKey() const;

    /** @brief Perturb key by swapping two positions */
    QString perturbKey(const QString& key) const;
};
