/**
 * @file SeriatedPlayfair3.h
 * @brief 序列化Playfair密码(扩展6x6网格+模拟退火双字母组频率评分) — Seriated Playfair Cipher with Extended 6x6 Grid and Simulated Annealing with Bigram Frequency Scoring
 *
 * 功能: 实现序列化Playfair密码，使用扩展6x6网格支持数字和字母，
 *       通过模拟退火算法和双字母组频率评分进行密钥优化。
 *
 * 协作: AesDecrypt3(AES) / VigenereCipher2(维吉尼亚) / Enigma4(恩尼格玛)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 序列化Playfair密码(6x6网格+模拟退火)
 */
class SeriatedPlayfair3 : public QObject {
    Q_OBJECT

public:
    /** @brief Decryption result with fitness score */
    struct DecryptResult {
        QString plaintext;
        double fitness = 0.0;
        int iterations = 0;
        double temperature = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 6;
        int keyLength = 0;
        int numDecryptions = 0;
        double bestFitness = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SeriatedPlayfair3(QObject *parent = nullptr);
    ~SeriatedPlayfair3() override;

    /** @brief Set initial temperature and cooling rate for SA */
    void setSAParameters(double initTemp = 20.0, double coolingRate = 0.997,
                         int maxIter = 10000);

    /** @brief Encrypt plaintext with given key */
    QString encrypt(const QString& plaintext, const QString& key) const;

    /** @brief Decrypt ciphertext with given key */
    QString decrypt(const QString& ciphertext, const QString& key) const;

    /** @brief Crack ciphertext using simulated annealing with bigram scoring */
    DecryptResult crack(const QString& ciphertext,
                        int seriationWidth = 1);

    /** @brief Load bigram frequency table from English text */
    void loadBigramFrequencies(const QVector<QVector<double>>& freq);

    /** @brief Build 6x6 grid from key */
    QVector<QVector<int>> buildGrid(const QString& key) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decryptionProgress(int iteration, double fitness, double temperature);
    void crackingCompleted(double fitness, int iterations, double timeMs);

private:
    double m_initTemp = 20.0;
    double m_coolingRate = 0.997;
    int m_maxIter = 10000;

    // 36-char alphabet: A-Z + 0-9
    QString m_alphabet;
    QVector<QVector<double>> m_bigramFreq; // [36][36]

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Normalize key to 36-char alphabet */
    QString normalizeKey(const QString& key) const;

    /** @brief Find position of character in grid */
    void findPosition(const QVector<QVector<int>>& grid, int ch, int& row, int& col) const;

    /** @brief Score text using bigram frequency */
    double scoreText(const QString& text) const;

    /** @brief Generate random permutation key */
    QString randomKey() const;

    /** @brief Mutate key by swapping two positions */
    QString mutateKey(const QString& key) const;

    /** @brief Apply Playfair pair transform (encrypt/decrypt) */
    QString transformPairs(const QString& text,
                           const QVector<QVector<int>>& grid, bool enc) const;
};
