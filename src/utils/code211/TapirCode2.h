/**
 * @file TapirCode2.h
 * @brief 袋鼠码(扩展字母替换+频率偏向爬山密钥优化) — Tapir Code with Extended Alphabet Substitution and Frequency-Biased Hill Climbing for Key Optimization
 *
 * 功能: 实现袋鼠码编解码，支持扩展字母表替换、
 *       频率偏向爬山密钥优化和N-gram评分。
 *
 * 协作: HuffmanCode3(哈夫曼编码) / ArithmeticCode2(算术编码) / LZWCode2(LZW压缩)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 袋鼠码(扩展字母替换+频率偏向爬山密钥优化)
 */
class TapirCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int alphabetSize = 0;
        double bestScore = 0.0;
        int hillClimbIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TapirCode2(QObject *parent = nullptr);
    ~TapirCode2() override;

    /** @brief Set alphabet for encoding */
    void setAlphabet(const QString& alphabet);

    /** @brief Encode text using key */
    QByteArray encode(const QByteArray& input, const QByteArray& key) const;

    /** @brief Decode text using key */
    QByteArray decode(const QByteArray& input, const QByteArray& key) const;

    /** @brief Break cipher using frequency-biased hill climbing */
    QByteArray crackWithHillClimb(const QByteArray& cipher,
                                  int maxIterations = 1000) const;

    /** @brief Score plaintext using N-gram frequency analysis */
    double ngramScore(const QByteArray& text, int n = 2) const;

    /** @brief Generate random substitution key */
    QByteArray generateRandomKey() const;

    /** @brief Set expected language frequencies for cracking */
    void setExpectedFrequencies(const QVector<QPair<QChar, double>>& freqs);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingCompleted(int inputLen, int outputLen, double timeMs);
    void crackingProgress(int iteration, double score);

private:
    QString m_alphabet;
    QVector<QPair<QChar, double>> m_expectedFreqs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build substitution map from key */
    QMap<QChar, QChar> buildSubstitutionMap(const QByteArray& key, bool invert) const;

    /** @brief Swap two positions in key and return modified key */
    static QByteArray swapKeyPositions(const QByteArray& key, int i, int j);

    /** @brief Compute character frequency distribution */
    QVector<QPair<QChar, double>> computeFrequencies(const QByteArray& text) const;

    /** @brief Fitness score based on frequency correlation */
    double frequencyFitness(const QByteArray& text) const;
};
