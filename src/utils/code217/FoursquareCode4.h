/**
 * @file FoursquareCode4.h
 * @brief 四方密码(8x8扩展Polybius网格+坐标转置+关键字派生轴) — Foursquare Cipher with Extended 8x8 Polybius Grid and Coordinate Transposition with Keyword-Derived Axis
 *
 * 功能: 实现四方密码加密算法，采用扩展的8x8 Polybius网格，
 *       坐标转置混合和关键字派生轴排序增强安全性。
 *
 * 协作: AffineCipher3(仿射密码) / PlayfairCipher3(Playfair密码) / VigenereCipher3(维吉尼亚密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 四方密码(8x8 Polybius网格+坐标转置)
 */
class FoursquareCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int gridSize = 8;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode4(QObject *parent = nullptr);
    ~FoursquareCode4() override;

    /** @brief Set keyword pair for the four squares */
    void setKeywords(const QString& keyword1, const QString& keyword2);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    /** @brief Get current grid state */
    QVector<QVector<int>> gridState() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);

private:
    QString m_keyword1;
    QString m_keyword2;

    // Four 8x8 grids: TL, TR, BL, BR
    QVector<QVector<int>> m_gridTL;
    QVector<QVector<int>> m_gridTR;
    QVector<QVector<int>> m_gridBL;
    QVector<QVector<int>> m_gridBR;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build 8x8 Polybius grid from keyword */
    QVector<QVector<int>> buildGrid(const QString& keyword) const;

    /** @brief Find position (row,col) of char in grid */
    QPair<int, int> findInGrid(const QVector<QVector<int>>& grid, int val) const;

    /** @brief Encode bigram via foursquare rules */
    QPair<int, int> encodeBigram(int r1, int c1, int r2, int c2) const;

    /** @brief Decode bigram via foursquare rules */
    QPair<int, int> decodeBigram(int r1, int c1, int r2, int c2) const;
};
