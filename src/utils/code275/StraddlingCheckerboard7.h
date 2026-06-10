/**
 * @file StraddlingCheckerboard7.h
 * @brief 跨越棋盘编码(扩展标签集与分数定位的增强型非标准编码) — Straddling Checkerboard with Extended Label Set and Fractional Positioning for Enhanced Non-standard Encoding
 *
 * 功能: 实现跨越棋盘编码(Straddling checkerboard)，采用扩展标签集(extended label set)
 *       与分数定位(fractional positioning)实现增强型非标准编码(enhanced non-standard encoding)。
 *
 * 协作: HuffmanTree9(哈夫曼编码) / ArithmeticCoder8(算术编码) / RangeCoder6(区间编码)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 跨越棋盘编码(扩展标签集与分数定位)
 */
class StraddlingCheckerboard7 : public QObject {
    Q_OBJECT

public:
    /** @brief Board configuration with extended label set */
    struct BoardConfig {
        int rows = 2;              // Number of board rows
        int cols = 10;             // Number of board columns
        QVector<int> rowSlots;     // Slots per row (stride pattern)
        QVector<int> labels;       // Extended label assignment (-1 = unassigned)
    };

    /** @brief Fractional position for fine-grained encoding */
    struct FractionalPos {
        int row = 0;
        int col = 0;
        double fraction = 0.0;     // Fractional offset within cell [0,1)
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncoded = 0;
        int numDecoded = 0;
        int boardSize = 0;
        double avgProcessingTimeMs = 0.0;
        double compressionRatio = 0.0;
    };

    explicit StraddlingCheckerboard7(QObject *parent = nullptr);
    ~StraddlingCheckerboard7() override;

    /** @brief Configure board with custom row slots */
    void setBoardConfig(const BoardConfig& config);

    /** @brief Set alphabet size for encoding */
    void setAlphabetSize(int size);

    /** @brief Encode a message into board positions */
    QVector<FractionalPos> encode(const QVector<int>& message);

    /** @brief Decode board positions back to message */
    QVector<int> decode(const QVector<FractionalPos>& positions);

    /** @brief Get current board configuration */
    BoardConfig boardConfig() const;

    /** @brief Optimize board layout for given frequency distribution */
    void optimizeLayout(const QVector<double>& frequencies);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingDone(int msgLen, int boardSize, double ratio, double timeMs);

private:
    BoardConfig m_config;
    int m_alphabetSize = 26;

    // Encoding lookup: symbol -> (row, col)
    QVector<QVector<int>> m_encodeTable;  // [symbol] = {row, col}

    // Decoding lookup: (row, col) -> symbol
    QVector<QVector<int>> m_decodeTable;  // [row][col] = symbol (-1 if gap)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build encode/decode tables from config */
    void buildTables();

    /** @brief Compute fractional position for a symbol occurrence */
    FractionalPos computeFractional(int symbol, int occurrence) const;

    /** @brief Resolve fractional position to nearest symbol */
    int resolveFractional(const FractionalPos& pos) const;

    /** @brief Count active cells in board */
    int countActiveCells() const;
};
