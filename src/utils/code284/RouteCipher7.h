/**
 * @file RouteCipher7.h
 * @brief 路径密码(可配置矩阵路径的几何换位加密) — Route Cipher with Configurable Matrix Path (Inward Spiral/Outward Spiral/Split Columns) for Flexible Geometric Transposition
 *
 * 功能: 实现路径密码(Route cipher)，采用可配置矩阵路径(inward spiral/outward spiral/split columns)
 *       实现灵活几何换位加密(flexible geometric transposition)。
 *
 * 协作: ColumnarTransposition6(列换位) / RailFence5(栅栏密码) / HillCipher8(希尔密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 路径密码(可配置矩阵路径)
 */
class RouteCipher7 : public QObject {
    Q_OBJECT

public:
    /** @brief Route path type */
    enum class PathType {
        InwardSpiral,      // Clockwise inward spiral
        OutwardSpiral,     // Center-outward spiral
        SplitColumns,      // Alternating column direction
        ZigzagRows         // Zigzag row traversal
    };
    Q_ENUM(PathType)

    /** @brief Cipher configuration */
    struct CipherConfig {
        int rows = 0;              // Matrix rows (0 = auto-calculate)
        int cols = 0;              // Matrix columns (0 = auto-calculate)
        PathType path = PathType::InwardSpiral;
        bool padChar = false;      // Whether to pad with specific char
        QChar padWith = QChar('X');
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastRows = 0;
        int lastCols = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RouteCipher7(QObject *parent = nullptr);
    ~RouteCipher7() override;

    void setConfig(const CipherConfig& cfg);

    /** @brief Encrypt plaintext using route cipher */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using route cipher */
    QString decrypt(const QString& ciphertext);

    /** @brief Auto-calculate matrix dimensions from text length */
    QPair<int, int> suggestDimensions(int length) const;

    /** @brief Generate traversal order indices for given matrix and path */
    QVector<int> generatePathIndices(int rows, int cols, PathType path) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int len, int rows, int cols, double timeMs);
    void decryptDone(int len, int rows, int cols, double timeMs);

private:
    CipherConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Inward spiral traversal order */
    QVector<int> inwardSpiral(int rows, int cols) const;

    /** @brief Outward spiral traversal order */
    QVector<int> outwardSpiral(int rows, int cols) const;

    /** @brief Split columns (alternating up/down) traversal */
    QVector<int> splitColumns(int rows, int cols) const;

    /** @brief Zigzag row traversal */
    QVector<int> zigzagRows(int rows, int cols) const;

    /** @brief Build 2D matrix from flat text */
    QVector<QVector<QChar>> buildMatrix(const QString& text, int rows, int cols) const;

    /** @brief Flatten matrix row-major */
    QString flattenMatrix(const QVector<QVector<QChar>>& mat) const;
};
