/**
 * @file TapirCode3.h
 * @brief Tapir编码(Polybius混合编码+模拟退火N元组频率适应度) — Tapir Code with Polybius-Hybrid Encoding and Simulated Annealing with N-Gram Frequency Fitness
 *
 * 功能: 实现Polybius方阵混合编码与Tapir解码，利用模拟退火算法
 *       以N元组(N-gram)频率统计作为适应度函数进行密钥搜索与密码分析。
 *
 * 协作: FrequencyAnalyzer2(频率分析) / VigenereCipher1(维吉尼亚) / SubstitutionCipher4(替换密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief Tapir编码(Polybius混合+模拟退火N元组)
 */
class TapirCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief SA configuration */
    struct SAConfig {
        double initialTemp = 100.0;
        double coolingRate = 0.995;
        int maxIterations = 50000;
        int ngramSize = 3;
    };

    /** @brief Decoding result */
    struct DecodeResult {
        QString plaintext;
        QString bestKey;
        double fitness = 0.0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int encodeCount = 0;
        int decodeCount = 0;
        double avgEncodingTimeMs = 0.0;
        double avgDecodingTimeMs = 0.0;
        double bestFitness = 0.0;
    };

    explicit TapirCode3(QObject *parent = nullptr);
    ~TapirCode3() override;

    /** @brief Set N-gram frequency table from reference text */
    void loadNGramFrequencies(const QString& referenceText, int ngramSize = 3);

    /** @brief Set SA parameters */
    void setSAConfig(const SAConfig& config);

    /** @brief Encode text using Polybius-hybrid with given key */
    QString encode(const QString& plaintext, const QString& key) const;

    /** @brief Decode ciphertext via simulated annealing key search */
    DecodeResult decode(const QString& ciphertext,
                        const QString& initialKey = "") const;

    /** @brief Score text against loaded N-gram frequency model */
    double scoreText(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeProgress(int iteration, double fitness, double temperature);
    void operationCompleted(const QString& op, double timeMs);

private:
    SAConfig m_saConfig;
    Stats m_stats;
    double m_encodeTimeSum = 0.0;
    double m_decodeTimeSum = 0.0;

    // N-gram log-frequency table
    QMap<QString, double> m_ngramLogFreq;
    int m_ngramTotal = 0;

    /** @brief Build 5x5 Polybius square from keyword */
    QVector<QVector<QChar>> buildPolybiusSquare(const QString& key) const;

    /** @brief Map (row, col) pair to character */
    QChar polybiusLookup(const QVector<QVector<QChar>>& square,
                         int row, int col) const;

    /** @brief Reverse lookup char in Polybius square */
    bool polybiusReverse(const QVector<QVector<QChar>>& square,
                         QChar ch, int& row, int& col) const;

    /** @brief Generate random alphabetic key */
    QString randomKey() const;

    /** @brief Mutate key by swapping two characters */
    QString mutateKey(const QString& key) const;

    /** @brief Decode with specific key using Polybius-hybrid */
    QString decodeWithKey(const QString& ciphertext,
                          const QString& key) const;

    /** @brief Compute N-gram fitness score for text */
    double ngramFitness(const QString& text) const;

    /** @brief Preprocess text to uppercase letters only */
    QString preprocess(const QString& text) const;
};
