/**
 * @file TapirCode7.h
 * @brief Tapir密码(扩展同音表与上下文依赖符号选择的频率均衡加密) — Tapir Cipher with Extended Homophone Table and Context-dependent Symbol Selection for Frequency-equalized Encryption
 *
 * 功能: 实现Tapir密码(Tapir cipher)，采用扩展同音表(extended homophone table)
 *       与上下文依赖符号选择(context-dependent symbol selection)实现频率均衡加密(frequency-equalized encryption)。
 *
 * 协作: HuffmanCodec6(哈夫曼编解码) / RSACode5(RSA加密) / AESCipher4(AES加密)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief Tapir密码(扩展同音表与上下文依赖符号选择)
 */
class TapirCode7 : public QObject {
    Q_OBJECT

public:
    /** @brief Encryption result */
    struct EncResult {
        QVector<int> cipherSymbols;
        double frequencyVariance = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSymbols = 0;
        int tableSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TapirCode7(QObject *parent = nullptr);
    ~TapirCode7() override;

    /** @brief Set the alphabet size for homophone table */
    void setAlphabetSize(int size);

    /** @brief Set the number of homophones per symbol (expansion factor) */
    void setExpansionFactor(int factor);

    /** @brief Build homophone table from frequency analysis */
    void buildTable(const QMap<QChar, double>& freqMap);

    /** @brief Encrypt plaintext using context-dependent homophone selection */
    EncResult encrypt(const QString& plaintext);

    /** @brief Decrypt cipher symbols back to plaintext */
    QString decrypt(const QVector<int>& cipherSymbols);

    /** @brief Analyze frequency distribution of cipher symbols */
    QMap<int, double> analyzeCipherFrequency(const QVector<int>& cipherSymbols) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int plainLen, int cipherLen, double variance, double timeMs);
    void tableBuilt(int alphabetSize, int tableSize, double timeMs);

private:
    int m_alphabetSize = 26;
    int m_expansion = 4;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Homophone table: char -> list of (symbol, contextMask) pairs */
    QMap<QChar, QVector<QPair<int, quint32>>> m_homophoneTable;

    /** @brief Reverse lookup: symbol -> original character */
    QMap<int, QChar> m_reverseTable;

    /** @brief Context state for deterministic symbol cycling */
    QMap<QChar, int> m_contextCounter;

    /** @brief Next available cipher symbol ID */
    int m_nextSymbolId = 0;

    /** @brief Select homophone based on context (previous symbol) */
    int selectHomophone(QChar ch, int prevSymbol);

    /** @brief Generate context mask from previous symbol */
    quint32 contextMask(int prevSymbol) const;

    /** @brief Compute frequency variance of cipher symbols */
    double computeVariance(const QVector<int>& symbols) const;
};
