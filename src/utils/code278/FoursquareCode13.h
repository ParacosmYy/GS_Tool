/**
 * @file FoursquareCode13.h
 * @brief 四方密码(扩展22x22网格与三元组替换的空间多图编码) — Foursquare Cipher with Extended 22x22 Grid and Trigram-based Substitution for Enhanced Spatial Polygraphic Encoding
 *
 * 功能: 实现四方密码(Foursquare cipher)，采用扩展22x22网格(extended 22x22 grid)
 *       与三元组替换(trigram-based substitution)实现增强空间多图编码(enhanced spatial polygraphic encoding)。
 *
 * 协作: PlayfairCode8(Playfair密码) / HillCode10(Hill密码) / VigenereCode6(Vigenere密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 四方密码(扩展22x22网格与三元组替换)
 */
class FoursquareCode13 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher configuration */
    struct Config {
        QString key1;
        QString key2;
        int gridSize = 22;
        bool trigramMode = true;
    };

    /** @brief Cipher result */
    struct CipherResult {
        QString output;
        int inputLength = 0;
        int outputLength = 0;
        int numBlocks = 0;
        double processingTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 22;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode13(QObject *parent = nullptr);
    ~FoursquareCode13() override;

    /** @brief Set cipher keys and configuration */
    void setConfig(const Config& config);

    /** @brief Encrypt plaintext using extended foursquare */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using extended foursquare */
    CipherResult decrypt(const QString& ciphertext);

    /** @brief Build 22x22 substitution grid from key */
    QVector<QVector<int>> buildGrid(const QString& key) const;

    /** @brief Map character to grid index */
    int charToIndex(QChar c) const;

    /** @brief Map grid index back to character */
    QChar indexToChar(int idx) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherDone(const QString& op, int blocks, double timeMs);

private:
    Config m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief The four substitution grids */
    QVector<QVector<int>> m_gridTL;    // Top-left (standard alphabet)
    QVector<QVector<int>> m_gridTR;    // Top-right (key1)
    QVector<QVector<int>> m_gridBL;    // Bottom-left (key2)
    QVector<QVector<int>> m_gridBR;    // Bottom-right (standard alphabet)

    /** @brief Standard alphabet ordering for 22x22 grid */
    QString m_alphabet;

    /** @brief Precompute all four grids */
    void precomputeGrids();

    /** @brief Encode a single bigram using foursquare rule */
    QPair<QChar, QChar> encodeBigram(QChar a, QChar b) const;

    /** @brief Decode a single bigram using foursquare rule */
    QPair<QChar, QChar> decodeBigram(QChar a, QChar b) const;

    /** @brief Encode a trigram using extended substitution */
    QString encodeTrigram(const QString& trig) const;

    /** @brief Decode a trigram using extended substitution */
    QString decodeTrigram(const QString& trig) const;

    /** @brief Find row and column of character in grid */
    void findInGrid(const QVector<QVector<int>>& grid, int val, int& row, int& col) const;

    /** @brief Prepare text (pad, normalize) */
    QString prepareText(const QString& text) const;
};
