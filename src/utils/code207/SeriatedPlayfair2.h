/**
 * @file SeriatedPlayfair2.h
 * @brief 序列化Playfair密码(双字母频率分析+模拟退火密钥恢复) — Seriated Playfair Cipher with Digraph Frequency Analysis and Simulated Annealing Key Recovery
 *
 * 功能: 实现序列化Playfair密码，支持双字母频率分析、
 *       模拟退火密钥恢复和序列化矩形加密/解密。
 *
 * 协作: HillCipher3(Hill密码) / VigenereCipher4(Vigenere密码) / Aes256Encrypt5(AES加密)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 序列化Playfair密码(双字母频率分析+模拟退火密钥恢复)
 */
class SeriatedPlayfair2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int keyLength = 0;
        int saIterations = 0;
        double bestScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SeriatedPlayfair2(QObject *parent = nullptr);
    ~SeriatedPlayfair2() override;

    void setPeriod(int period);
    void setMaxSAIterations(int maxIter);
    void setCoolingRate(double rate);
    void setInitialTemp(double temp);

    /** @brief Encrypt plaintext using seriated Playfair */
    QString encrypt(const QString& plaintext, const QString& key) const;

    /** @brief Decrypt ciphertext using seriated Playfair */
    QString decrypt(const QString& ciphertext, const QString& key) const;

    /** @brief Build 5x5 Playfair matrix from key */
    QVector<QVector<int>> buildMatrix(const QString& key) const;

    /** @brief Compute English digraph frequency score */
    double digraphScore(const QString& text) const;

    /** @brief Crack cipher via simulated annealing key recovery */
    QString crackSA(const QString& ciphertext);

    /** @brief Get best recovered key */
    QString getBestKey() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void crackProgress(int iteration, double score, double temp);
    void operationCompleted(const QString& op, double timeMs);

private:
    int m_period = 7;
    int m_maxSAIter = 10000;
    double m_coolingRate = 0.999;
    double m_initTemp = 20.0;

    QString m_bestKey;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Standard English digraph frequencies (top 25x25) */
    static QVector<QVector<double>> s_digraphFreq;

    /** @brief Initialize digraph frequency table */
    static void initDigraphFreq();

    /** @brief Playfair digraph encrypt/decrypt pair */
    QPair<QChar, QChar> processPair(QChar a, QChar b,
                                     const QVector<QVector<int>>& matrix,
                                     bool encrypt) const;

    /** @brief Find position of char in matrix */
    QPair<int, int> findInMatrix(int ch, const QVector<QVector<int>>& matrix) const;

    /** @brief Generate random key */
    QString randomKey() const;

    /** @brief Perturb key for SA neighbour */
    QString perturbKey(const QString& key) const;

    /** @brief Preprocess text: uppercase, I/J merge, remove non-alpha */
    QString preprocess(const QString& text) const;
};
