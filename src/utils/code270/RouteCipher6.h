/**
 * @file RouteCipher6.h
 * @brief 路线密码(可配置网格路径模式的螺旋/对角/锯齿矩阵几何转置加密) — Route Cipher with Configurable Grid Path Pattern (Spiral/Diagonal/Zigzag) for Matrix-Based Geometric Transposition
 *
 * 功能: 实现路线密码(Route cipher)，采用可配置网格路径模式(configurable grid path pattern)
 *       包括螺旋(spiral)/对角(diagonal)/锯齿(zigzag)实现矩阵几何转置加密(matrix-based geometric transposition)。
 *
 * 协作: TranspositionCipher4(转位置换) / ColumnarTransposition3(列置换) / RailFence3(栅栏密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 路线密码(可配置网格路径模式的螺旋/对角/锯齿矩阵几何转置加密)
 */
class RouteCipher6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridRows = 0;
        int gridCols = 0;
        int textSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Grid path pattern */
    enum class PathPattern {
        Spiral,     ///< Clockwise spiral from outer to inner
        Diagonal,   ///< Diagonal zigzag from top-left
        Zigzag      ///< Horizontal zigzag row-by-row
    };

    explicit RouteCipher6(QObject *parent = nullptr);
    ~RouteCipher6() override;

    /** @brief Set grid dimensions (auto-computed if rows=0 or cols=0) */
    void setGridSize(int rows, int cols);

    /** @brief Set path pattern for reading order */
    void setPathPattern(PathPattern pattern);

    /** @brief Encrypt plaintext via route cipher */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext via inverse route */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherDone(int textSize, int rows, int cols, double timeMs);

private:
    int m_rows = 0;
    int m_cols = 0;
    PathPattern m_pattern = PathPattern::Spiral;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Fill grid row-major and compute auto dimensions */
    QPair<int, int> computeGrid(int textLen) const;

    /** @brief Generate spiral traversal indices */
    QVector<int> spiralOrder(int rows, int cols) const;

    /** @brief Generate diagonal traversal indices */
    QVector<int> diagonalOrder(int rows, int cols) const;

    /** @brief Generate zigzag traversal indices */
    QVector<int> zigzagOrder(int rows, int cols) const;

    /** @brief Get traversal order based on current pattern */
    QVector<int> traversalOrder(int rows, int cols) const;

    /** @brief Build row-major grid from text */
    QVector<QVector<QChar>> buildGrid(const QString& text, int rows, int cols) const;
};
