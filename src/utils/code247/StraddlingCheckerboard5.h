/**
 * @file StraddlingCheckerboard5.h
 * @brief 跨界棋盘编码(莫尔斯码启发网格布局+频率优化空单元放置) — Straddling Checkerboard with Morse-Code-Inspired Grid Layout and Frequency-Optimized Empty Cell Placement
 *
 * 功能: 实现跨界棋盘密码(Straddling Checkerboard Cipher)，使用莫尔斯码
 *       启发的网格布局(Morse-code-inspired grid layout)和频率优化的空单元
 *       放置(frequency-optimized empty cell placement)提升编码效率。
 *
 * 协作: BaconCipher5(培根密码) / PlayfairCipher5(Playfair密码) / SubstitutionCipher5(替换密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 跨界棋盘编码(莫尔斯码网格+频率优化)
 */
class StraddlingCheckerboard5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncodes = 0;
        int numDecodes = 0;
        int inputLength = 0;
        int outputLength = 0;
        double compressionRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StraddlingCheckerboard5(QObject *parent = nullptr);
    ~StraddlingCheckerboard5() override;

    /** @brief Set the key phrase for alphabet permutation */
    void setKeyPhrase(const QString& phrase);

    /** @brief Set two row indices for straddling rows (0-9, distinct) */
    void setStraddleRows(int row1, int row2);

    /** @brief Encode plaintext to digit string */
    QString encode(const QString& plaintext);

    /** @brief Decode digit string back to plaintext */
    QString decode(const QString& ciphertext);

    /** @brief Get current board layout (10x10 grid) */
    QVector<QString> boardLayout() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_keyPhrase;
    int m_straddleRow1 = 2;
    int m_straddleRow2 = 6;

    /** @brief The 10x3 checkerboard: row label -> char mapping */
    QVector<QVector<QChar>> m_board;

    /** @brief Reverse lookup: char -> (row, col) */
    QVector<int> m_charRow;  // 256 slots, -1 if not on board
    QVector<int> m_charCol;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build frequency-optimized checkerboard */
    void buildBoard();

    /** @brief Generate permuted alphabet from key phrase */
    QVector<QChar> permuteAlphabet() const;

    /** @brief Compute English letter frequency ordering */
    QVector<int> frequencyOrder() const;

    /** @brief Place high-frequency chars in single-digit cells */
    void placeHighFrequency(const QVector<QChar>& alpha);

    /** @brief Place remaining chars in double-digit cells */
    void placeRemaining(const QVector<QChar>& alpha, int startIdx);
};
