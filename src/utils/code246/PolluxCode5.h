/**
 * @file PolluxCode5.h
 * @brief Pollux密码(扩展莫尔斯元素组合+n元频率概率密钥评分) — Pollux Cipher with Extended Morse Element Combinations and Probabilistic Key Scoring via N-Gram Frequency Analysis
 *
 * 功能: 实现Pollux密码(Pollux Cipher)，通过扩展莫尔斯元素组合(extended
 *       Morse element combinations)将密文数字映射到dot/dash/separator三态，
 *       使用n元频率分析(n-gram frequency analysis)的概率密钥评分(probabilistic
 *       key scoring)自动恢复最优密钥。
 *
 * 协作: CaesarCipher3(凯撒密码) / VigenereCipher4(维吉尼亚密码) / PlayfairCipher3(普莱费尔密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief Pollux密码(扩展莫尔斯元素组合+n元频率密钥评分)
 */
class PolluxCode5 : public QObject {
    Q_OBJECT

public:
    /** @brief Morse element type */
    enum MorseElement { Dot = 0, Dash = 1, Separator = 2 };

    /** @brief Key candidate with score */
    struct KeyCandidate {
        QVector<MorseElement> mapping;  // 10-element mapping for digits 0-9
        double score = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int cipherLength = 0;
        int numCandidatesEvaluated = 0;
        double bestScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolluxCode5(QObject *parent = nullptr);
    ~PolluxCode5() override;

    /** @brief Set n-gram size for frequency analysis (2 or 3) */
    void setNgramSize(int n);

    /** @brief Set number of top key candidates to evaluate */
    void setMaxCandidates(int count);

    /** @brief Encrypt plaintext with given key mapping */
    QString encrypt(const QString& plaintext, const QVector<MorseElement>& key) const;

    /** @brief Decrypt ciphertext with given key mapping */
    QString decrypt(const QString& ciphertext, const QVector<MorseElement>& key) const;

    /** @brief Auto-crack: find best key via n-gram scoring */
    KeyCandidate crack(const QString& ciphertext);

    /** @brief Get English n-gram frequency table */
    QMap<QString, double> ngramFrequencies() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void crackProgress(int candidatesEvaluated, double bestScore);
    void crackCompleted(double bestScore, double timeMs);

private:
    int m_ngramSize = 2;
    int m_maxCandidates = 500;

    QMap<QString, double> m_ngramFreq;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Morse code lookup table */
    QMap<QChar, QString> m_morseTable;

    /** @brief Initialize English letter n-gram frequencies */
    void initNgramFrequencies();

    /** @brief Initialize Morse code table */
    void initMorseTable();

    /** @brief Score plaintext using n-gram log-probability */
    double scoreText(const QString& text) const;

    /** @brief Generate candidate mappings from ciphertext digits */
    QVector<QVector<MorseElement>> generateCandidates(const QString& ciphertext) const;

    /** @brief Decode morse string to letters */
    QString morseToText(const QString& morse) const;
};
