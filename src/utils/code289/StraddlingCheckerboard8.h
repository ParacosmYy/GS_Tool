/**
 * @file StraddlingCheckerboard8.h
 * @brief 跨骑棋盘密码(变宽行分区与位置依赖单表混合实现不规则分数化) — Straddling Checkerboard with Variable-width Row Partitions and Position-dependent Monoalphabetic Mixing for Irregular Fractionation
 *
 * 功能: 实现跨骑棋盘密码(Straddling checkerboard cipher)，采用变宽行分区(variable-width row partitions)
 *       与位置依赖单表混合(position-dependent monoalphabetic mixing)实现不规则分数化(irregular fractionation)。
 *
 * 协作: PlayfairCipher6(Playfair密码) / BifidCipher6(Bifid密码) / NihilistCipher5(Nihilist密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 跨骑棋盘密码(变宽行分区与位置依赖单表混合实现不规则分数化)
 */
class StraddlingCheckerboard8 : public QObject {
    Q_OBJECT

public:
    /** @brief Checkerboard configuration */
    struct BoardConfig {
        QString alphabet;                   // Full alphabet (typically 28+ chars incl. space/digits)
        QVector<int> rowWidths;             // Width of each row (variable)
        QVector<int> blankPositions;        // Positions that start new rows
        QVector<QVector<int>> shiftKeys;    // Position-dependent shift per column
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StraddlingCheckerboard8(QObject *parent = nullptr);
    ~StraddlingCheckerboard8() override;

    /** @brief Set the checkerboard layout from keyword */
    void setKeyword(const QString& keyword);

    /** @brief Set row widths for variable-width partitioning */
    void setRowWidths(const QVector<int>& widths);

    /** @brief Set position-dependent shift keys */
    void setShiftKeys(const QVector<QVector<int>>& keys);

    /** @brief Encode plaintext to digit stream */
    QString encode(const QString& plaintext) const;

    /** @brief Decode digit stream back to plaintext */
    QString decode(const QString& ciphertext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeDone(int inLen, int outLen, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_keyword;
    QVector<int> m_rowWidths;
    QVector<QVector<int>> m_shiftKeys;

    // Internal board: char -> (row, col) mapping
    QVector<QVector<QChar>> m_board;   // m_board[row][col] = character
    QVector<int> m_blankCols;          // Column indices that are blank in row 0
    int m_totalCols = 10;

    /** @brief Build the checkerboard from current config */
    void buildBoard();

    /** @brief Apply position-dependent shift to a digit */
    int applyShift(int digit, int position) const;

    /** @brief Reverse the position-dependent shift */
    int reverseShift(int digit, int position) const;

    /** @brief Expand keyword into full alphabet */
    QString expandAlphabet(const QString& keyword) const;
};
