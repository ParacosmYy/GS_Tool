/**
 * @file StraddlingCheckerboard3.h
 * @brief 跨越棋盘格编码(变宽行+模拟退火布局优化) — Straddling Checkerboard Coding with Variable-Width Rows and Simulated Annealing Layout Optimization
 *
 * 功能: 实现跨越棋盘格编码/解码，支持变宽行配置，
 *       通过模拟退火算法优化编码布局以最小化最大游程长度。
 *
 * 协作: ConvolutionalCode5(卷积码) / ReedSolomon7(RS码) / TurboCode4(Turbo码)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 跨越棋盘格编码(变宽行+模拟退火优化)
 */
class StraddlingCheckerboard3 : public QObject {
    Q_OBJECT

public:
    /** @brief Layout row configuration */
    struct RowConfig {
        int offset = 0;       // Starting position offset
        int width = 0;        // Number of cells in this row
        QVector<int> symbolMap; // Symbol assignment for each cell
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int numRows = 0;
        double compressionRatio = 0.0;
        int saIterations = 0;
        double saTemperature = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StraddlingCheckerboard3(QObject *parent = nullptr);
    ~StraddlingCheckerboard3() override;

    /** @brief Set number of checkerboard rows and alphabet size */
    void setParameters(int numRows = 3, int alphabetSize = 26);

    /** @brief Optimize layout using simulated annealing */
    void optimizeLayout(int maxIterations = 5000, double initTemp = 10.0, double coolingRate = 0.995);

    /** @brief Encode input symbols to checkerboard positions */
    QVector<int> encode(const QVector<int>& symbols) const;

    /** @brief Decode checkerboard positions back to symbols */
    QVector<int> decode(const QVector<int>& positions) const;

    /** @brief Get current layout configuration */
    QVector<RowConfig> layout() const;

    /** @brief Compute max run length of current layout */
    int maxRunLength() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingCompleted(int inputLen, int outputLen, double timeMs);
    void layoutOptimized(int iterations, double finalCost);

private:
    int m_numRows = 3;
    int m_alphabetSize = 26;
    int m_cellsPerRow = 0;

    QVector<RowConfig> m_rows;
    QVector<int> m_symbolToRow;  // Maps symbol -> row index
    QVector<int> m_decodeTable;  // Maps flat cell index -> symbol

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build default variable-width layout */
    void buildDefaultLayout();

    /** @brief Build decode lookup table */
    void buildDecodeTable();

    /** @brief Compute cost function (max run length + balance penalty) */
    double computeCost(const QVector<RowConfig>& config) const;

    /** @brief Generate neighbor solution for SA by swapping two cells */
    QVector<RowConfig> neighborSolution(const QVector<RowConfig>& config) const;

    /** @brief Count total cells across all rows */
    int totalCells() const;

    /** @brief Validate layout completeness */
    bool validateLayout() const;
};
