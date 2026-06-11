/**
 * @file RouteCipher8.h
 * @brief 路线密码(螺旋路径枚举与对角遍历实现多模式几何置换加密) — Route Cipher with Spiral Path Enumeration and Diagonal Traversal Modes for Multi-Pattern Geometric Transposition Encryption
 *
 * 功能: 实现路线密码(Route cipher)，采用螺旋路径枚举(spiral path enumeration)
 *       与对角遍历模式(diagonal traversal modes)实现多模式几何置换加密(multi-pattern geometric transposition encryption)。
 *
 * 协作: HillCipher(矩阵密码) / PlayfairCipher(替换密码) / VigenereCipher(多表密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

class RouteCipher8 : public QObject {
    Q_OBJECT

public:
    /** @brief Traversal pattern for route cipher */
    enum class PathMode {
        SpiralCW,        ///< Clockwise spiral from outer to inner
        SpiralCCW,       ///< Counter-clockwise spiral
        DiagonalDown,    ///< Diagonal traversal top-left to bottom-right
        DiagonalUp,      ///< Diagonal traversal bottom-left to top-right
        ZigzagVertical,  ///< Alternating column direction
        ZigzagHorizontal ///< Alternating row direction
    };

    /** @brief Encryption/decryption result */
    struct CipherResult {
        QString text;
        int rows = 0;
        int cols = 0;
        PathMode mode = PathMode::SpiralCW;
        int paddingChars = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalEncrypts = 0;
        quint64 totalDecrypts = 0;
        PathMode lastMode = PathMode::SpiralCW;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RouteCipher8(QObject *parent = nullptr);
    ~RouteCipher8() override;

    void setPathMode(PathMode mode);
    void setGridSize(int rows, int cols);
    void setPaddingChar(QChar ch);

    /** @brief Encrypt plaintext using selected route traversal pattern */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext by reversing the route traversal */
    CipherResult decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int rows, int cols, int pathMode, double timeMs);
    void decryptDone(int rows, int cols, int pathMode, double timeMs);

private:
    PathMode m_mode = PathMode::SpiralCW;
    int m_rows = 0;
    int m_cols = 0;
    QChar m_padding = QChar('_');
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Fill grid row-major from text */
    QVector<QVector<QChar>> fillGrid(const QString& text, int rows, int cols) const;

    /** @brief Read grid along specified traversal path */
    QString readPath(const QVector<QVector<QChar>>& grid, PathMode mode) const;

    /** @brief Write text into grid along specified traversal path (inverse) */
    QVector<QVector<QChar>> writePath(const QString& text, int rows, int cols, PathMode mode) const;

    /** @brief Generate spiral clockwise index sequence */
    QVector<int> spiralCWIndices(int rows, int cols) const;

    /** @brief Generate spiral counter-clockwise index sequence */
    QVector<int> spiralCCWIndices(int rows, int cols) const;

    /** @brief Generate diagonal-down traversal indices */
    QVector<int> diagonalDownIndices(int rows, int cols) const;

    /** @brief Generate diagonal-up traversal indices */
    QVector<int> diagonalUpIndices(int rows, int cols) const;

    /** @brief Generate zigzag vertical traversal indices */
    QVector<int> zigzagVerticalIndices(int rows, int cols) const;

    /** @brief Generate zigzag horizontal traversal indices */
    QVector<int> zigzagHorizontalIndices(int rows, int cols) const;

    /** @brief Get traversal indices for given mode */
    QVector<int> getPathIndices(int rows, int cols, PathMode mode) const;

    /** @brief Compute grid dimensions from text length */
    void computeGridSize(int textLen);
};
