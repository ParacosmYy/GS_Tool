/**
 * @file StraddlingCheckerboard.h
 * @brief 跨棋盘密码(混合宽度数字编码+助记关键词优化) — Straddling Checkerboard Cipher with Mixed-Width Digit Encoding and Mnemonic Keyword Optimization
 *
 * 功能: 实现跨棋盘密码，支持混合宽度数字编码(1位/2位)、
 *       助记关键词驱动的棋盘布局优化和空位选择策略。
 *
 * 协作: Playfair2(Playfair) / Vigenere3(维吉尼亚) / ADFGVX2(ADFGVX)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 跨棋盘密码(混合宽度编码+助记优化)
 */
class StraddlingCheckerboard : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int lastInputLen = 0;
        int lastOutputLen = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StraddlingCheckerboard(QObject *parent = nullptr);
    ~StraddlingCheckerboard() override;

    /** @brief Set keyword for board layout optimization */
    void setKeyword(const QString& keyword);

    /** @brief Set blank positions (two distinct digits 0-9) */
    void setBlankPositions(int blank1, int blank2);

    /** @brief Encode plaintext to digit string */
    QString encode(const QString& plaintext) const;

    /** @brief Decode digit string back to plaintext */
    QString decode(const QString& digits) const;

    /** @brief Get current board layout for inspection */
    QVector<QVector<QChar>> boardLayout() const;

    /** @brief Build optimized board from keyword */
    void buildBoard();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int inputLen, int outputLen);

private:
    QString m_keyword;
    int m_blank1 = 2;
    int m_blank2 = 6;

    // Board: 3 rows x 10 cols; row 0 direct, rows 1-2 via blanks
    QVector<QVector<QChar>> m_board;

    // Reverse lookup: char -> (row, col)
    // row=-1 means invalid
    QMap<QChar, QPair<int, int>> m_lookup;

    Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Initialize default board */
    void initDefaultBoard();

    /** @brief Rebuild reverse lookup from board */
    void rebuildLookup();
};
