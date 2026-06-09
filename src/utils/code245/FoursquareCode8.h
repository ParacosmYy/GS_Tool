/**
 * @file FoursquareCode8.h
 * @brief 四方密码(16x16扩展网格+双关键字行列置换编码) — Foursquare Cipher with 16x16 Extended Grid and Double-Keyword Driven Row/Column Permutation Encoding
 *
 * 功能: 实现四方密码(Foursquare Cipher)，采用16x16扩展网格(16x16 extended
 *       grid)替代传统5x5方阵，支持双关键字(double-keyword)驱动的行列置换
 *       (row/column permutation)进行编码与解码。
 *
 * 协作: PlayfairCipher5(Playfair密码) / VigenereCipher6(Vigenere密码) / AffineCipher4(仿射密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 四方密码(16x16扩展网格+双关键字行列置换编码)
 */
class FoursquareCode8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 16;
        int numEncryptions = 0;
        int numDecryptions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode8(QObject *parent = nullptr);
    ~FoursquareCode8() override;

    /** @brief Set keyword for square 1 (top-right) */
    void setKeyword1(const QString& keyword);

    /** @brief Set keyword for square 2 (bottom-left) */
    void setKeyword2(const QString& keyword);

    /** @brief Encrypt plaintext using foursquare cipher */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using foursquare cipher */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Get current grid for a square (0=TL, 1=TR, 2=BL, 3=BR) */
    QVector<QVector<int>> grid(int squareIndex) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int length, double timeMs);
    void decryptionCompleted(int length, double timeMs);

private:
    QString m_keyword1;
    QString m_keyword2;

    // Four 16x16 grids, each storing character codes 0-255
    QVector<QVector<int>> m_gridTL;  // Top-left: standard order
    QVector<QVector<int>> m_gridTR;  // Top-right: keyword1 permuted
    QVector<QVector<int>> m_gridBL;  // Bottom-left: keyword2 permuted
    QVector<QVector<int>> m_gridBR;  // Bottom-right: standard order

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build a 16x16 grid from keyword-driven permutation */
    QVector<QVector<int>> buildGrid(const QString& keyword, bool standard) const;

    /** @brief Find row/col of a character code in a grid */
    QPair<int, int> findInGrid(const QVector<QVector<int>>& grid, int code) const;

    /** @brief Generate permutation order from keyword */
    QVector<int> keywordPermutation(const QString& keyword) const;

    /** @brief Convert string to padded pairs of character codes */
    QVector<QPair<int, int>> textToPairs(const QString& text) const;
};
