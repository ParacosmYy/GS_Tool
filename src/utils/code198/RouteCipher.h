/**
 * @file RouteCipher.h
 * @brief 路径密码(可配置网格路径螺旋/对角/锯齿+路径恢复密码分析) — Route Cipher with Configurable Grid Path and Path-Recovery Cryptanalysis
 *
 * 功能: 实现路径密码算法，支持螺旋/对角/锯齿网格路径、
 *       路径恢复密码分析和自定义路径模式。
 *
 * 协作: AesEngine3(AES加密) / XorCipher1(XOR密码) / Base64Codec2(Base64编解码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief 路径密码(可配置网格路径)
 */
class RouteCipher : public QObject {
    Q_OBJECT

public:
    /** @brief Grid traversal path type */
    enum class PathMode { Spiral, Diagonal, Zigzag };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastRows = 0;
        int lastCols = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RouteCipher(QObject *parent = nullptr);
    ~RouteCipher() override;

    void setGridSize(int rows, int cols);
    void setPathMode(PathMode mode);

    /** @brief Encrypt plaintext using route cipher */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using route cipher */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Generate spiral traversal order for rows x cols grid */
    QVector<QPair<int,int>> spiralPath(int rows, int cols) const;

    /** @brief Generate diagonal traversal order */
    QVector<QPair<int,int>> diagonalPath(int rows, int cols) const;

    /** @brief Generate zigzag traversal order */
    QVector<QPair<int,int>> zigzagPath(int rows, int cols) const;

    /** @brief Build grid from text, filling row-major */
    QVector<QVector<QChar>> buildGrid(const QString& text, int rows, int cols) const;

    /** @brief Read grid cells following given path */
    QString readPath(const QVector<QVector<QChar>>& grid,
                     const QVector<QPair<int,int>>& path) const;

    /** @brief Write characters along path into empty grid */
    void writePath(QVector<QVector<QChar>>& grid,
                   const QVector<QPair<int,int>>& path, const QString& text) const;

    /** @brief Auto-detect best grid dimensions for text length */
    QPair<int,int> autoGridSize(int textLen) const;

    /** @brief Brute-force cryptanalysis: try all small grids and score results */
    QVector<QPair<int,int>> cryptanalyze(const QString& ciphertext, int maxDim = 10) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int rows, int cols, double timeMs);

private:
    int m_rows = 4;
    int m_cols = 4;
    PathMode m_mode = PathMode::Spiral;

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Get traversal path based on current mode */
    QVector<QPair<int,int>> currentPath(int rows, int cols) const;

    /** @brief Score text for likelihood of being readable English */
    static double englishScore(const QString& text);
};
