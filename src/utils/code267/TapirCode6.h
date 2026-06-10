/**
 * @file TapirCode6.h
 * @brief Tapir密码(同音替换与概率符号分配频率平坦化加密) — Tapir Cipher with Homophonic Substitution and Probabilistic Symbol Assignment for Frequency Flattening Encryption
 *
 * 功能: 实现Tapir密码(Tapir cipher)，采用同音替换(homophonic substitution)与
 *       概率符号分配(probabilistic symbol assignment)实现频率平坦化加密(frequency flattening)。
 *
 * 协作: XORCipher8(异或密码) / CaesarCode9(凯撒密码) / VigenereCode10(维吉尼亚密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief Tapir密码(同音替换与概率符号分配频率平坦化加密)
 */
class TapirCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int alphabetSize = 0;
        int numHomophones = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Homophone table entry */
    struct Homophones {
        QChar baseChar;
        QVector<int> symbols;       // assigned homophone symbol codes
        QVector<double> weights;    // probability weights for each symbol
    };

    explicit TapirCode6(QObject *parent = nullptr);
    ~TapirCode6() override;

    /** @brief Build homophone table from key string */
    void setKey(const QString& key);

    /** @brief Set number of homophones per plaintext symbol */
    void setHomophonesPerSymbol(int count);

    /** @brief Encrypt plaintext using homophonic substitution */
    QVector<int> encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext symbol sequence back to plaintext */
    QString decrypt(const QVector<int>& ciphertext);

    /** @brief Get the current homophone table */
    QVector<Homophones> homophoneTable() const;

    /** @brief Analyze frequency distribution of ciphertext */
    QMap<int, int> frequencyAnalysis(const QVector<int>& ciphertext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inputLen, int outputLen, double timeMs);

private:
    int m_homoCount = 4;        // homophones per symbol
    int m_nextSymbol = 0;       // next available symbol code

    QVector<Homophones> m_table;
    QMap<QChar, int> m_charIndex;   // char -> index in m_table
    QMap<int, int> m_symbolToIndex; // symbol code -> char index

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build homophone table for given alphabet */
    void buildTable(const QString& alphabet);

    /** @brief Select a symbol for a character using weighted probability */
    int selectSymbol(int charIndex) const;

    /** @brief Normalize weights to sum to 1.0 */
    static void normalizeWeights(QVector<double>& weights);
};
