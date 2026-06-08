/**
 * @file RouteCipher3.h
 * @brief 路线密码(螺旋与对角遍历变体+模拟退火路线模式恢复) — Route Cipher with Spiral and Diagonal Traversal Variants and Simulated Annealing for Route Pattern Recovery
 *
 * 功能: 实现路线密码(Route Cipher)，支持螺旋(spiral)和对角(diagonal)遍历变体，
 *       并采用模拟退火(simulated annealing)算法进行路线模式恢复与密文破解。
 *
 * 协作: SubstitutionCipher2(替换密码) / TranspositionCipher1(转置密码) / AECipher5(AE密码)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 路线密码(螺旋+对角遍历+模拟退火恢复)
 */
class RouteCipher3 : public QObject {
    Q_OBJECT

public:
    /** @brief Traversal pattern type */
    enum Pattern {
        SpiralCW = 0,
        SpiralCCW = 1,
        DiagonalDown = 2,
        DiagonalUp = 3
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int rows = 0;
        int cols = 0;
        int textLength = 0;
        int annealIterations = 0;
        double bestScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RouteCipher3(QObject *parent = nullptr);
    ~RouteCipher3() override;

    /** @brief Set grid dimensions */
    void setGridSize(int rows, int cols);

    /** @brief Encrypt plaintext using specified route pattern */
    QString encrypt(const QString& plaintext, Pattern pattern) const;

    /** @brief Decrypt ciphertext using specified route pattern */
    QString decrypt(const QString& ciphertext, Pattern pattern) const;

    /** @brief Recover route pattern via simulated annealing (returns best pattern + decrypted) */
    QString annealRecover(const QString& ciphertext,
                          const QVector<QString>& dictionary,
                          int maxIter = 5000);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptCompleted(int length, double timeMs);
    void annealProgress(int iter, double score);

private:
    int m_rows = 4;
    int m_cols = 4;
    int m_seed = 42;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build 2D grid from text, filling row-major */
    QVector<QVector<QChar>> buildGrid(const QString& text) const;

    /** @brief Read grid using spiral clockwise traversal */
    QVector<QChar> readSpiralCW(const QVector<QVector<QChar>>& grid) const;

    /** @brief Read grid using spiral counter-clockwise */
    QVector<QChar> readSpiralCCW(const QVector<QVector<QChar>>& grid) const;

    /** @brief Read grid using diagonal downward zigzag */
    QVector<QChar> readDiagonalDown(const QVector<QVector<QChar>>& grid) const;

    /** @brief Read grid using diagonal upward zigzag */
    QVector<QChar> readDiagonalUp(const QVector<QVector<QChar>>& grid) const;

    /** @brief Place chars into grid by pattern (inverse of read) */
    QVector<QVector<QChar>> writeByPattern(
        const QVector<QChar>& chars, Pattern pattern) const;

    /** @brief Score text against dictionary (bigram frequency) */
    double scoreText(const QString& text,
                     const QVector<QString>& dict) const;

    /** @brief LCG random generator */
    double randUniform();
};

