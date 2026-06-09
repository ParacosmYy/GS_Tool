/**
 * @file RouteCipher4.h
 * @brief 路由密码(螺旋/对角/锯齿可配置路径矩形网格转置) — Route Cipher with Configurable Path Patterns (Spiral/Diagonal/Zigzag) on Rectangular Grid Transposition
 *
 * 功能: 实现路由密码(Route cipher)，在矩形网格(rectangular grid)上按可配置
 *       路径模式(螺旋spiral / 对角diagonal / 锯齿zigzag)进行转置加密与解密。
 *
 * 协作: HillCipher6(希尔密码) / AffineCipher3(仿射密码) / VigenereCipher5(维吉尼亚)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 路由密码(螺旋/对角/锯齿可配置路径矩形网格转置)
 */
class RouteCipher4 : public QObject {
    Q_OBJECT

public:
    /** @brief Path pattern for reading the grid */
    enum PathPattern {
        Spiral = 0,     // Clockwise inward spiral
        Diagonal = 1,   // Top-left to bottom-right diagonals
        Zigzag = 2      // Alternating row direction zigzag
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridRows = 0;
        int gridCols = 0;
        int inputLength = 0;
        PathPattern pattern = Spiral;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RouteCipher4(QObject *parent = nullptr);
    ~RouteCipher4() override;

    /** @brief Set grid dimensions */
    void setGridSize(int rows, int cols);

    /** @brief Set path pattern */
    void setPathPattern(PathPattern p);

    /** @brief Encrypt plaintext via route transposition */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext back to original */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(bool encrypt, int inputLen, int gridRows, int gridCols, double timeMs);

private:
    int m_rows = 4;
    int m_cols = 4;
    PathPattern m_pattern = Spiral;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Fill grid row-major and read via pattern */
    QVector<QChar> readByPattern(const QVector<QChar>& rowMajor, int rows, int cols) const;

    /** @brief Reverse: given pattern-ordered chars, recover row-major */
    QVector<QChar> reversePattern(const QVector<QChar>& ordered, int rows, int cols) const;

    /** @brief Spiral order indices */
    QVector<int> spiralOrder(int rows, int cols) const;

    /** @brief Diagonal order indices */
    QVector<int> diagonalOrder(int rows, int cols) const;

    /** @brief Zigzag order indices */
    QVector<int> zigzagOrder(int rows, int cols) const;

    /** @brief Get pattern indices */
    QVector<int> patternOrder(int rows, int cols) const;

    /** @brief Invert a permutation */
    QVector<int> invertPermutation(const QVector<int>& perm) const;
};
