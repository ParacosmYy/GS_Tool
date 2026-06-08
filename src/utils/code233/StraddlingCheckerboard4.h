/**
 * @file StraddlingCheckerboard4.h
 * @brief 跨越棋盘密码(频率排序密钥网格+动态行宽选择) — Straddling Checkerboard Cipher with Frequency-Ranked Key Grid and Dynamic Row-Width Selection
 *
 * 功能: 实现跨越棋盘密码(straddling checkerboard cipher)，使用频率排序的密钥网格
 *       (frequency-ranked key grid)优化字符编码，并采用动态行宽选择(dynamic row-width
 *       selection)策略实现自适应编码效率。
 *
 * 协作: HomophonicSubstitution8(同音替换) / PolybiusSquare6(波利比奥斯方阵) / BaconCipher5(培根密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 跨越棋盘密码(频率排序密钥网格+动态行宽选择)
 */
class StraddlingCheckerboard4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        double compressionRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StraddlingCheckerboard4(QObject *parent = nullptr);
    ~StraddlingCheckerboard4() override;

    /** @brief Set the key phrase for grid generation */
    void setKeyPhrase(const QString& phrase);

    /** @brief Set row labels (two digits for straddling rows) */
    void setRowLabels(int row1, int row2);

    /** @brief Encode plaintext to digit string */
    QString encode(const QString& plaintext);

    /** @brief Decode digit string to plaintext */
    QString decode(const QString& ciphertext);

    /** @brief Get current checkerboard grid layout */
    QMap<QChar, QString> gridLayout() const;

    /** @brief Analyze frequency distribution of input */
    QVector<QPair<QChar, double>> frequencyAnalysis(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int inLen, int outLen, double ratio);
    void decodeCompleted(int inLen, int outLen);

private:
    QString m_keyPhrase;
    int m_rowLabel1 = 2;
    int m_rowLabel2 = 6;

    // Grid: char -> code string
    QMap<QChar, QString> m_encodeGrid;
    // Reverse: code string -> char
    QMap<QString, QChar> m_decodeGrid;

    // Row configuration: row widths for dynamic selection
    QVector<int> m_rowWidths;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build frequency-ranked grid from key phrase */
    void buildGrid();

    /** @brief Analyze character frequencies and sort descending */
    QVector<QPair<QChar, int>> analyzeFrequency(const QString& text) const;

    /** @brief Select optimal row widths based on frequency data */
    QVector<int> selectRowWidths(const QVector<QPair<QChar, int>>& freqs);

    /** @brief Generate alphabet with key-based permutation */
    QVector<QChar> generateAlphabet() const;
};
