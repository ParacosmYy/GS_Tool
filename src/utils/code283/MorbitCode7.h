/**
 * @file MorbitCode7.h
 * @brief Morbit密码(扩展4x4网格与关键词驱动排列的增强紧凑视觉编码) — Morbit Cipher with Extended 4x4 Grid and Keyword-driven Permutation for Enhanced Compact Visual Encoding
 *
 * 功能: 实现Morbit密码(Morbit cipher)，采用扩展4x4网格(extended 4x4 grid)
 *       与关键词驱动排列(keyword-driven permutation)实现增强紧凑视觉编码(enhanced compact visual encoding)。
 *
 * 协作: FrequencyAnalyzer6(频率分析) / SubstitutionCipher5(替换密码) / TranspositionCipher4(置换密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Morbit密码(扩展4x4网格与关键词驱动排列)
 */
class MorbitCode7 : public QObject {
    Q_OBJECT

public:
    /** @brief Grid cell with encoded symbol */
    struct GridCell {
        int row = 0;
        int col = 0;
        QChar symbol;
        int permIndex = 0;
    };

    /** @brief Encoding/decoding result */
    struct MorbitResult {
        QString output;
        QVector<GridCell> gridLayout;
        int inputLength = 0;
        int gridCells = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 16;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MorbitCode7(QObject *parent = nullptr);
    ~MorbitCode7() override;

    void setKeyword(const QString& keyword);
    void setGridSize(int size);

    /** @brief Encode plaintext using Morbit cipher */
    MorbitResult encode(const QString& plaintext);

    /** @brief Decode ciphertext using Morbit cipher */
    MorbitResult decode(const QString& ciphertext);

    /** @brief Build 4x4 grid with keyword permutation */
    QVector<GridCell> buildGrid() const;

    /** @brief Generate permutation from keyword */
    QVector<int> keywordPermutation(const QString& kw) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeDone(int n, int cells, double timeMs);

private:
    QString m_keyword;
    int m_gridRows = 4;
    int m_gridCols = 4;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<QChar> m_symbols;

    /** @brief Initialize symbol set for grid */
    void initSymbols();

    /** @brief Map a pair of digits to a grid cell */
    GridCell mapToCell(int row, int col, int permIdx) const;

    /** @brief Convert text to digit pairs */
    QVector<int> textToDigits(const QString& text) const;

    /** @brief Convert digit pairs back to text */
    QString digitsToText(const QVector<int>& digits) const;
};
