/**
 * @file FoursquareCode12.h
 * @brief 四方密码(扩展20x20网格与自动密钥驱动渐进替换增强多表编码) — Foursquare Cipher with Extended 20x20 Grid and Autokey-driven Progressive Substitution for Enhanced Polygraphic Encoding
 *
 * 功能: 实现四方密码(Foursquare cipher)，采用扩展20x20网格(extended 20x20 grid)
 *       与自动密钥驱动渐进替换(autokey-driven progressive substitution)实现增强多表编码(polygraphic encoding)。
 *
 * 协作: PlayfairCode8(Playfair密码) / BifidCode7(Bifid密码) / HillCode6(Hill密码)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 四方密码(扩展20x20网格与自动密钥驱动渐进替换)
 */
class FoursquareCode12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 20;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode12(QObject *parent = nullptr);
    ~FoursquareCode12() override;

    /** @brief Set primary key for top-right square */
    void setKey1(const QString& key);

    /** @brief Set secondary key for bottom-left square */
    void setKey2(const QString& key);

    /** @brief Set autokey seed for progressive substitution */
    void setAutokeySeed(const QString& seed);

    /** @brief Encode plaintext using foursquare with autokey */
    QString encode(const QString& plaintext);

    /** @brief Decode ciphertext using foursquare with autokey */
    QString decode(const QString& ciphertext);

    /** @brief Get current grid state for square at index (0-3) */
    QVector<QVector<int>> grid(int squareIndex) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingDone(int inputLen, int outputLen, double timeMs);

private:
    QString m_key1;
    QString m_key2;
    QString m_autokeySeed;

    // Four 20x20 substitution grids stored as flat arrays
    QVector<QVector<int>> m_grids;          // 4 grids, each 20x20
    QVector<QVector<int>> m_reverseGrids;   // Reverse lookup tables

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build all four squares from keys */
    void buildGrids();

    /** @brief Fill a grid using a keyword-driven mixed alphabet */
    void fillGrid(int gridIdx, const QString& key);

    /** @brief Generate autokey sequence for progressive substitution */
    QString generateAutokey(const QString& input, const QString& seed) const;

    /** @brief Map character to grid coordinates */
    void charToCoords(int gridIdx, int ch, int& row, int& col) const;

    /** @brief Map grid coordinates back to character */
    int coordsToChar(int gridIdx, int row, int col) const;

    /** @brief Apply progressive autokey shift to a character */
    int applyAutokeyShift(int ch, int shift, bool forward) const;
};
