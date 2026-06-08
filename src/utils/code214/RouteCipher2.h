/**
 * @file RouteCipher2.h
 * @brief 路线密码(多路径遍历加密+遗传算法密钥搜索) — Route Cipher with Multi-Path Traversal and Genetic Algorithm Key Search for Route Recovery
 *
 * 功能: 实现路线密码，支持多路径网格遍历加密、
 *       遗传算法搜索最优路线密钥进行密码恢复。
 *
 * 协作: VigenereCipher1(维吉尼亚) / SubstitutionCipher3(替换密码) / HashChain4(哈希链)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief 路线密码(多路径遍历+遗传算法密钥恢复)
 */
class RouteCipher2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 0;
        int gridRows = 0;
        int gridCols = 0;
        int populationSize = 0;
        int generations = 0;
        double bestFitness = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Traversal path type */
    enum PathType {
        SpiralInward = 0,
        ZigzagRow = 1,
        ZigzagCol = 2,
        Diagonal = 3
    };

    explicit RouteCipher2(QObject *parent = nullptr);
    ~RouteCipher2() override;

    /** @brief Set grid dimensions and GA parameters */
    void setParameters(int rows, int cols, int populationSize = 50,
                       int generations = 200, double mutationRate = 0.1);

    /** @brief Encrypt plaintext using specified path */
    QString encrypt(const QString& plaintext, PathType path) const;

    /** @brief Decrypt ciphertext using specified path */
    QString decrypt(const QString& ciphertext, PathType path) const;

    /** @brief Generate traversal indices for given path type */
    QVector<int> generatePath(int rows, int cols, PathType path) const;

    /** @brief Recover route key via genetic algorithm */
    PathType recoverKey(const QString& ciphertext,
                        const QString& languageModel = "en") const;

    /** @brief Score decrypted text using language frequency analysis */
    double fitness(const QString& text, const QString& lang) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int pathType, double timeMs);
    void keyRecovered(int bestPath, double fitness, double timeMs);

private:
    int m_rows = 4;
    int m_cols = 4;
    int m_popSize = 50;
    int m_gens = 200;
    double m_mutRate = 0.1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Spiral inward traversal indices */
    QVector<int> spiralPath(int rows, int cols) const;

    /** @brief Zigzag row traversal indices */
    QVector<int> zigzagRowPath(int rows, int cols) const;

    /** @brief Zigzag column traversal indices */
    QVector<int> zigzagColPath(int rows, int cols) const;

    /** @brief Diagonal traversal indices */
    QVector<int> diagonalPath(int rows, int cols) const;

    /** @brief Compute English bigram frequency score */
    double englishScore(const QString& text) const;

    /** @brief Tournament selection for GA */
    int tournamentSelect(const QVector<double>& fitnesses) const;

    /** @brief Crossover two path type parents */
    PathType crossover(PathType a, PathType b) const;
};
