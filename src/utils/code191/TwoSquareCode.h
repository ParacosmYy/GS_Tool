/**
 * @file TwoSquareCode.h
 * @brief 双方阵密码(配对5x5网格+双字母坐标映射+互质周期分析) — Two-Square Cipher with Paired 5x5 Grids, Digraph Coordinate Mapping and Mutual-Prime Period Analysis
 *
 * 功能: 实现双方阵密码算法，支持配对5x5 Playfair网格、
 *       双字母(digraph)坐标映射和互质周期密钥分析。
 *
 * 协作: PlayfairCipher2(Playfair) / VigenereCipher3(维吉尼亚) / HillCipher4(Hill)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 双方阵密码(配对5x5网格+互质周期分析)
 */
class TwoSquareCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 5;
        int digraphCount = 0;
        int keyLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TwoSquareCode(QObject *parent = nullptr);
    ~TwoSquareCode() override;

    /** @brief Set keyword for grid generation, auto-removes duplicates */
    void setKeyword(const QString& keyword);

    /** @brief Encrypt plaintext using two-square cipher */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using two-square cipher */
    QString decrypt(const QString& ciphertext);

    /** @brief Crack cipher via mutual-prime period analysis */
    QVector<QString> periodAnalysis(const QString& ciphertext,
                                     int maxPeriod = 20) const;

    /** @brief Get current grid pair (for inspection) */
    QPair<QVector<QChar>, QVector<QChar>> grids() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);

private:
    QString m_keyword;
    QVector<QChar> m_grid1;     // 5x5 grid for first letter
    QVector<QChar> m_grid2;     // 5x5 grid for second letter
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build a 5x5 grid from keyword + alphabet (J merged into I) */
    QVector<QChar> buildGrid(const QString& key) const;

    /** @brief Find row,col of char in grid */
    QPair<int, int> findPosition(const QVector<QChar>& grid,
                                  QChar ch) const;

    /** @brief Prepare text: uppercase, J->I, even length */
    QString prepareText(const QString& text) const;

    /** @brief Encrypt a single digraph using coordinate mapping */
    QPair<QChar, QChar> encryptDigraph(QChar a, QChar b) const;

    /** @brief Decrypt a single digraph */
    QPair<QChar, QChar> decryptDigraph(QChar a, QChar b) const;

    /** @brief Greatest common divisor */
    int gcd(int a, int b) const;
};
