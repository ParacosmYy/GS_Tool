/**
 * @file FoursquareCode11.h
 * @brief 四方密码(Polybius坐标交织与对角转置增强空间编码) — Foursquare Cipher with Polybius Coordinate Interleaving and Diagonal Transposition for Enhanced Spatial Encoding
 *
 * 功能: 实现四方密码(Foursquare cipher)，采用Polybius坐标交织(Polybius coordinate
 *       interleaving)和对角转置(diagonal transposition)实现增强空间编码
 *       (enhanced spatial encoding)。
 *
 * 协作: PlayfairCipher10(Playfair密码) / HillCipher8(Hill密码) / VigenereCipher9(Vigenere密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 四方密码(Polybius坐标交织与对角转置增强空间编码)
 */
class FoursquareCode11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode11(QObject *parent = nullptr);
    ~FoursquareCode11() override;

    /** @brief Set the four keys for the four squares (TL, TR, BL, BR) */
    void setKeys(const QString& key1, const QString& key2);

    /** @brief Encrypt plaintext using foursquare cipher */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using foursquare cipher */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Get the current square configuration */
    QVector<QVector<int>> squares() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherComputed(int inputLen, int outputLen, double timeMs);

private:
    /** @brief 5x5 Polybius square (0-24 mapping) */
    struct Square {
        int grid[5][5] = {};
        int pos[25][2] = {};  // Reverse lookup: char -> (row, col)
    };

    Square m_squareTL;  // Top-left (standard alphabet)
    Square m_squareTR;  // Top-right (key1-based)
    Square m_squareBL;  // Bottom-left (key2-based)
    Square m_squareBR;  // Bottom-right (standard alphabet)

    Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Build a Polybius square from keyword */
    void buildSquare(Square& sq, const QString& key) const;

    /** @brief Build standard alphabet square (I/J merged) */
    void buildStandardSquare(Square& sq) const;

    /** @brief Map character to index 0-24 (J=I) */
    static int charToIndex(QChar c);

    /** @brief Map index 0-24 back to character */
    static QChar indexToChar(int idx);

    /** @brief Prepare text: uppercase, remove non-alpha, replace J->I */
    QString prepareText(const QString& text) const;

    /** @brief Apply diagonal transposition for extra mixing */
    QVector<QPair<int,int>> diagonalTranspose(
        const QVector<QPair<int,int>>& coords) const;

    /** @brief Reverse diagonal transposition */
    QVector<QPair<int,int>> inverseDiagonalTranspose(
        const QVector<QPair<int,int>>& coords) const;
};
